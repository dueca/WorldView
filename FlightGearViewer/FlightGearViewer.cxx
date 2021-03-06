/* ------------------------------------------------------------------   */
/*      item            : FlightGearViewer.cxx
        made by         : Rene' van Paassen
        date            : 090616
	category        : body file 
        description     : 
	changes         : 090616 first version
        language        : C++
*/

#define FlightGearViewer_cxx

#include "FlightGearViewer.hxx"
#include <unistd.h>
#include <cstdio>
#include <debug.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

#ifdef DUECA
#include <debug.h>
#else
#include <iostream>
#define E_MOD(A) cerr << "ERR:" << A << endl;
#define W_MOD(A) cerr << "WRN:" << A << endl;
#define I_MOD(A) cerr << "INF:" << A << endl;
#define D_MOD(A) cerr << "DEB:" << A << endl;
#endif

using namespace std;

FlightGearViewer::FlightGearViewer() :
  sockfd(-1),
  receiver("127.0.0.1"),    
  own_interface("0.0.0.0"),
  port(7100),
  dest_address(),
  binary_packets(false),
  vis_boundary(5000.0),
  vis_aloft(10000.0),
  axis(new FGECEFAxis()),
  mp_socket(-1),
  mp_hostip(""),
  mp_port(5000),
  mp_address()
{
  //
}


FlightGearViewer::~FlightGearViewer()
{
  if (sockfd != -1) close(sockfd);
}

bool FlightGearViewer::complete()
{
  const char* classname = "FligthGearViewer";

  sockfd = socket(PF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    perror("Creating socket");
    return false;
  }

  // source address
  struct sockaddr_in src_address;
  memset(&src_address, 0, sizeof(src_address));
  src_address.sin_family = AF_INET;

  // if an interface was selected, try to set this one
  if (own_interface.size() && 
      inet_aton(own_interface.c_str(), &src_address.sin_addr) == 0) {
    E_MOD(classname << " source address invalid");
    return false;

    // and bind to the source address
    if (bind(sockfd, reinterpret_cast<sockaddr*>(&src_address), 
	     sizeof(src_address) != 0)) {
      perror("Cannot bind to source");
      return false;
    }
  }

  // destination address
  memset(&dest_address, 0, sizeof(dest_address));
  dest_address.sin_family = AF_INET;

  // destination must have been filled in
  if (!receiver.size() || 
      inet_aton(receiver.c_str(), &dest_address.sin_addr) == 0) {
    E_MOD(classname << " invalid receiver address!");
    return false;
  }
  
  // and port too
  if (port <= 0 || port > 0xffff) {
    E_MOD(classname << " invalid port");
    return false;
  }
  dest_address.sin_port = htons(port);

  // create the encoder
  encoder = boost::shared_ptr<MultiplayerEncode>
    (new MultiplayerEncode(own_interface, port, *axis));

  // prepare destination address
  memset(&mp_address, 0, sizeof(mp_address));
  mp_address.sin_family = AF_INET;

  // IP address of the server, if applicable
  if (inet_aton(mp_hostip.c_str(), &mp_address.sin_addr) == 0) {
    I_MOD(classname << " no valid multiplayer address, no multiplay");
  }
  else {

    // check multiplayer port
    if (mp_port <= 0 || mp_port > 0xffff) {
      E_MOD(classname << " invalid multiplayer port");
      return false;
    }
    mp_address.sin_port = htons(mp_port);

    // create socket
    mp_socket = socket(PF_INET, SOCK_DGRAM, 0);
    if (mp_socket == -1) {
      perror("Creating multiplayer socket");
      return false;
    }

    // source address should be OK, can now safely bind
    if (own_interface.size()) {
      assert(bind(mp_socket, reinterpret_cast<sockaddr*>(&src_address), 
		  sizeof(src_address)) == 0);
    }
  }
 
  return true;
}

bool FlightGearViewer::setLatLonPsi0(const vector<double>& vec)
{
  if (vec.size() != 2 && vec.size() != 3 && vec.size() != 4) {
    E_MOD("FlightGearViewer::setLatLonPsi0 Need 2, 3 or 4 parameters");
    return false;
  }
  const double deg2rad = M_PI / 180.0;
  double psi_zero = vec.size() >= 3 ? vec[2] : 0.0;
  double h_zero = vec.size() == 4 ? vec[3] : 0.0;
  axis = boost::shared_ptr<FGAxis>(new FGLocalAxis(vec[0]*deg2rad, 
						   vec[1]*deg2rad, 
						   h_zero,
						   psi_zero*deg2rad));
  return true;
}
  
void FlightGearViewer::redraw(bool wait, bool save_context)
{
  if (binary_packets) {
    char buffer[6*8+4];
    AmorphStore st(buffer, sizeof(buffer));
    int magic = 0x44544543;
    packData(st, fg_command);
    packData(st, magic);
    if (sendto(sockfd, buffer, sizeof(buffer), 0, 
	       reinterpret_cast<sockaddr*>(&dest_address), 
	       sizeof(dest_address)) == -1) {
      perror("Sending to flightgear");
    }
  }
  else {
    char buffer[1000];
    int nchar = snprintf(buffer, sizeof(buffer), 
			 "%.9f,%.9f,%.9f,%.7f,%.7f,%.7f,%.1f,%.1f\n", 
			 fg_command.latlonalt_phithtpsi[0],
			 fg_command.latlonalt_phithtpsi[1],
			 fg_command.latlonalt_phithtpsi[2],
			 fg_command.latlonalt_phithtpsi[3],
			 fg_command.latlonalt_phithtpsi[4],
			 fg_command.latlonalt_phithtpsi[5],
			 vis_boundary, vis_aloft);
    if (sendto(sockfd, buffer, nchar, 0, 
	       reinterpret_cast<sockaddr*>(&dest_address), 
	       sizeof(dest_address)) == -1) {
      perror("Sending to flightgear");
    }
  }
}

void FlightGearViewer::sendPositionReport()
{
  // nothing if multiplayer not configured
  if (mp_socket == -1) return;
  
  if (sendto(mp_socket, encoder->getBuffer(), encoder->getBufferSize(), 0, 
	     reinterpret_cast<sockaddr*>(&mp_address), 
	     sizeof(mp_address)) == -1) {
    perror("Sending to multiplayer server");
  }
}

void FlightGearViewer::waitSwap()
{
  
}

void FlightGearViewer::setBase(TimeTickType tick, const BaseObjectMotion& base,
                               double late)
{
  axis->transform(fg_command.latlonalt_phithtpsi, base.xyz, base.attitude_q);

  for (auto &obj: active_objects) {
    obj.second->iterate(tick, base, late);
  }
}

bool FlightGearViewer::createControllable
(const GlobalId& master_id, const NameSet& cname, entryid_type entry_id,
 uint32_t creation_id, const std::string& data_class,
 const std::string& entry_label, Channel::EntryTimeAspect time_aspect)
{
  creation_key_t keypair(cname.name, creation_id);
  
  // check
  assert(active_objects.count(keypair) == 0);

  // find from created classes, first on name+label
  std::string key = data_class + std::string(":") + entry_label;
  auto ii = flightgear_classes.find(key);

  FlightGearObject *op = NULL;
  
  // if not found, now on data_class alone
  if (ii == flightgear_classes.end()) {
    ii = flightgear_classes.find(data_class);
  }

  if (ii != flightgear_classes.end()) {
    
    op = new FlightGearObject(entry_label, ii->second.fgclass,
                              ii->second.livery, this);
    op->connect(master_id, cname, entry_id, time_aspect);
    boost::intrusive_ptr<FlightGearObject> bop(op);
    active_objects[keypair] = bop;
    return true;
  }
  
  return false;
}

void FlightGearViewer::removeControllable(const NameSet& cname,
                                          uint32_t creation_id)
{
  active_objects[std::make_pair(cname.name, creation_id)].reset();
}

FlightGearViewer::FGSpecs::FGSpecs(const std::string& fgclass, 
				   const std::string& livery) :
  fgclass(fgclass),
  livery(livery)
{
  //
}

FlightGearViewer::FGSpecs::FGSpecs()
{
  //
}

bool FlightGearViewer::modelTableEntry(const std::vector<std::string>& s)
{
  if (s.size() != 3 || !s[0].size() || !s[1].size() ) {
    cerr << "Need to specify 3 strings, first two non-empty" << endl;
    return false;
  }

  // There should be no mapping yet
  FGClassMap::const_iterator ii = flightgear_classes.find(s[0]);
  if (ii != flightgear_classes.end()) {
    cerr << "Already have a translation for " << s[0] << endl;
    return false;
  }
  
  // insert new mapping
  flightgear_classes[s[0]] = FGSpecs(s[1], s[2]);

  return true;
}
