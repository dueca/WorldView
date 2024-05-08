/* ------------------------------------------------------------------   */
/*      item            : FlightGearViewer.cxx
        made by         : Rene' van Paassen
        date            : 090616
        category        : body file
        description     :
        changes         : 090616 first version
        language        : C++
*/

#include "Dstring.hxx"
#include "FlightGearCommand.hxx"
#include "FlightGearObject.hxx"
#include "WorldDataSpec.hxx"
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <chrono>
#include <ctime>
#include <limits>
#define FlightGearViewer_cxx

#include "FlightGearViewer.hxx"
#include "rpc/xdr.h"
#include <arpa/inet.h>
#include <boost/lexical_cast.hpp>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define DUECA
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
  fg_command(),
  axis(new FGECEFAxis()),
  encoder(),
  current_tick(0),
  retain_age(100000), // one second
  debugdump(false),
  mp_clients(),
  mp_port(0),
  mp_radarrange(50.0), // nm
  mp_interface(),
  mp_sockfd(-1),
  mp_time0(std::numeric_limits<double>::quiet_NaN())
{
  //
}

FlightGearViewer::~FlightGearViewer()
{
  if (sockfd != -1)
    close(sockfd);
  if (mp_sockfd != -1)
    close(mp_sockfd);
}

bool FlightGearViewer::complete()
{
  const char *classname = "FlightGearViewer";

  // only send data if receiver configured
  if (receiver.size()) {
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
      if (bind(sockfd, reinterpret_cast<sockaddr *>(&src_address),
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
  }

  // create the encoder
  encoder = boost::shared_ptr<MultiplayerEncode>(
    new MultiplayerEncode(own_interface, port, *axis));

  return true;
}

bool FlightGearViewer::setLatLonAltPsi0(const vector<double> &vec)
{
  if (vec.size() != 2 && vec.size() != 3 && vec.size() != 4) {
    E_MOD("FlightGearViewer::setLatLonAltPsi0 Need 2, 3 or 4 parameters");
    return false;
  }
  const double deg2rad = M_PI / 180.0;
  double h_zero = vec.size() >= 3 ? vec[2] : 0.0;
  double psi_zero = vec.size() == 4 ? vec[3] : 0.0;
  axis = boost::shared_ptr<FGAxis>(new FGLocalAxis(
    vec[0] * deg2rad, vec[1] * deg2rad, h_zero, psi_zero * deg2rad));
  return true;
}

struct MessageHead
{
  XDR xdr_data;
  uint32_t magic;
  uint32_t version;
  uint32_t msgid;
  uint32_t msglen;
  float radarrange;
  uint32_t unused;
  dueca::Dstring<8> name;

  MessageHead(const char *b)
  {
    xdrmem_create(&xdr_data, const_cast<char *>(b), 24, XDR_DECODE);
    xdr_u_int(&xdr_data, &magic);
    xdr_u_int(&xdr_data, &version);
    xdr_u_int(&xdr_data, &msgid);
    xdr_u_int(&xdr_data, &msglen);
    xdr_float(&xdr_data, &radarrange);
    xdr_u_int(&xdr_data, &unused);
    xdr_opaque(&xdr_data, name.data(), 8UL);
  }

  bool valid() const { return magic == 0x46474653 && msgid == 7; }

  const dueca::Dstring<8> &getName() const { return name; }
};

struct MessageBody
{
  XDR xdr_data;
  double time, lag, x, y, z;
  MessageBody(const char *buffer)
  {
    xdrmem_create(&xdr_data, const_cast<char *>(&buffer[32 + 96]), 40,
                  XDR_DECODE);

    xdr_double(&xdr_data, &time);
    xdr_double(&xdr_data, &lag);
    xdr_double(&xdr_data, &x);
    xdr_double(&xdr_data, &y);
    xdr_double(&xdr_data, &z);
  }
};

void FlightGearViewer::redraw(bool wait, bool save_context)
{
  if (binary_packets) {

    // send buffer
    char buffer[6 * 8 + 4];
    AmorphStore st(buffer, sizeof(buffer));

    uint32_t magic = 0x44544543;
    packData(st, fg_command);
    packData(st, magic);

    if (sockfd != -1) {
      if (sendto(sockfd, buffer, sizeof(buffer), 0,
                 reinterpret_cast<sockaddr *>(&dest_address),
                 sizeof(dest_address)) == -1) {
        perror("Sending to flightgear");
      }
    }
  }
  else {
    char buffer[1000];
    int nchar = snprintf(
      buffer, sizeof(buffer), "%.9f,%.9f,%.9f,%.7f,%.7f,%.7f,%.1f,%.1f\n",
      fg_command.latlonalt_phithtpsi[0], fg_command.latlonalt_phithtpsi[1],
      fg_command.latlonalt_phithtpsi[2], fg_command.latlonalt_phithtpsi[3],
      fg_command.latlonalt_phithtpsi[4], fg_command.latlonalt_phithtpsi[5],
      vis_boundary, vis_aloft);
    if (sockfd != -1) {
      if (sendto(sockfd, buffer, nchar, 0,
                 reinterpret_cast<sockaddr *>(&dest_address),
                 sizeof(dest_address)) == -1) {
        perror("Sending to flightgear");
      }
    }
  }

  if (mp_sockfd != -1) {

    // check whether messages came in on the multiplayer port
    // set-up for select
    fd_set socks;
    FD_ZERO(&socks);
    FD_SET(mp_sockfd, &socks);
    struct timeval timeout = { 0 };
    char buffer[2000];

    // need the peer address, prepare a union for this
    union {
      struct sockaddr_in in;
      struct sockaddr gen;
    } peer_ip;
    socklen_t peer_ip_len = sizeof(peer_ip.in);

    // check messages
    while (select(mp_sockfd + 1, &socks, NULL, NULL, &timeout) > 0) {
      ssize_t nbytes = recvfrom(mp_sockfd, buffer, sizeof(buffer), 0,
                                &peer_ip.gen, &peer_ip_len);

      if (nbytes > 32) {
        MessageHead msgh(buffer);
        if (msgh.valid()) {

          if (debugdump) {
            std::cout << std::endl << "Multiplay from peer "
                      << inet_ntoa(peer_ip.in.sin_addr) << ":"
                      << ntohs(peer_ip.in.sin_port)
                      << " size=" << nbytes << std::endl;
            encoder->dump(buffer, nbytes);
          }
        }
        else {
          W_MOD("Invalid message from " <<
                inet_ntoa(peer_ip.in.sin_addr) << ":" <<
                ntohs(peer_ip.in.sin_port) << " size=" << nbytes);
        }
      }
    }
  }
}

FlightGearViewer::MultiplayerClient::MultiplayerClient(int sockfd,
                                                       const sockaddr &in) :
  sockfd(sockfd),
  dest(in)
{
  //
}

void FlightGearViewer::MultiplayerClient::update(
  const MultiplayerEncode &encoder) const
{
  sendto(sockfd, encoder.getBuffer(), encoder.getBufferSize(), 0, &dest,
         sizeof(dest));
}

void FlightGearViewer::sendPositionReport()
{
  // print message for debug purposes
  if (debugdump) {
    std::cout << std::endl << "Multiplay send to "
              << mp_clients.size() << " clients, size="
              << encoder->getBufferSize() << std::endl;
    encoder->dump(encoder->getBuffer(), encoder->getBufferSize());
  }

  // send to all the clients
  for (auto const &client : mp_clients) {
    MessageBody msgb(encoder->getBuffer());
    MessageHead msgh(encoder->getBuffer());
    //std::cerr << msgh.getName() << "time=" << msgb.time << " x=" << msgb.x
    //          << " y=" << msgb.y << " z=" << msgb.z << std::endl;

    client.update(*encoder);
  }
}

void FlightGearViewer::waitSwap() {}

void FlightGearViewer::setBase(TimeTickType tick, const BaseObjectMotion &base,
                               double late, bool freeze)
{
  current_tick = tick;
  axis->transform(fg_command.latlonalt_phithtpsi, base.xyz, base.attitude_q);

  for (auto &obj : active_objects) {
    obj.second->iterate(tick, base, late, freeze);
  }
}

bool FlightGearViewer::createControllable(
  const GlobalId &master_id, const NameSet &cname, entryid_type entry_id,
  uint32_t creation_id, const std::string &data_class,
  const std::string &entry_label, Channel::EntryTimeAspect time_aspect)
{
  creation_key_t keypair(cname.name, creation_id);

  // check this is unique
  assert(active_objects.count(keypair) == 0);

  try {
    WorldDataSpec obj =
      retrieveFactorySpec(data_class, entry_label, creation_id);

    FlightGearObject *op =
      new FlightGearObject(obj.name, obj.type, obj.filename[0], this);
    op->connect(master_id, cname, entry_id, time_aspect);
    boost::intrusive_ptr<FlightGearObject> bop(op);
    active_objects[keypair] = bop;
    return true;
  }
  catch (const MapSpecificationError &problem) {
    W_MOD("FlightGearViewer: not configured for "
          << data_class << ':' << entry_label << ", ignoring channel " << cname
          << " entry " << entry_id);
  }
  catch (const std::exception &problem) {
    cerr << "FlightGearViewer: When trying to create for " << data_class << '/'
         << entry_label << " encountered: " << problem.what() << endl;
  }
  return false;
}

double FlightGearViewer::getFlightTime(double time)
{
  if (isnan(mp_time0)) {
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t tt = system_clock::to_time_t(now);

    // gm time
    std::tm gt = *std::gmtime(&tt);
    time_t gmnow = mktime(&gt);
    time_t diff = gmnow - tt;
    if (gt.tm_isdst > 0) {
      diff = diff - 3600;
    }

    // set to start of GMT
    gt.tm_hour = 0;
    gt.tm_min = 0;
    gt.tm_sec = 0;

    // convert back to time
    tt = std::mktime(&gt);

    auto start_of_day = system_clock::from_time_t(tt - diff);
    auto elapsed = now - start_of_day;

    double timeinday = 1e-3 * duration_cast<milliseconds>(elapsed).count();

    mp_time0 = timeinday - time;
  }

  double res = time + mp_time0;
  if (res > 24 * 3600.0) {
    W_MOD("New UTC day! " << res << " time0 " << mp_time0 << " time " << time);
    mp_time0 -= 24 * 3600.0;
    res = time + mp_time0;
  }
  return res;
}

void FlightGearViewer::removeControllable(const NameSet &cname,
                                          uint32_t creation_id)
{
  active_objects.erase(std::make_pair(cname.name, creation_id));
}

FlightGearViewer::FGSpecs::FGSpecs(const std::string &fgclass,
                                   const std::string &livery) :
  fgclass(fgclass),
  livery(livery)
{
  //
}

FlightGearViewer::FGSpecs::FGSpecs()
{
  //
}

bool FlightGearViewer::modelTableEntry(const std::vector<std::string> &s)
{
  if (s.size() != 3 || !s[0].size() || !s[1].size()) {
    cerr << "Need to specify 3 strings, first two non-empty" << endl;
    return false;
  }

  WorldDataSpec obj;
  obj.name = s[0];
  obj.type = s[1]; // flightgear aircraft model
  obj.filename.push_back(s[2]); // livery
  addFactorySpec(s[0], obj);
  return true;
}

bool FlightGearViewer::addMultiplayClient(const std::string &addr)
{
  uint16_t mcport = 5010U;
  std::string mcaddress;
  size_t pport = addr.find(':');

  try {
    if (pport != string::npos) {
      mcport = boost::lexical_cast<uint16_t>(addr.substr(pport + 1));
      mcaddress = addr.substr(0, pport);
    }
    else {
      mcaddress = addr;
    }
  }
  catch (const std::exception &e) {
    E_MOD("Cannot decode peer destination from " << addr
                                                 << " error:" << e.what());
    return false;
  }

  // configure the UDP address for sending
  struct addrinfo *ta;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_CANONNAME;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;
  int aires =
    getaddrinfo(mcaddress.c_str(),
                boost::lexical_cast<std::string>(mcport).c_str(), &hints, &ta);

  if (aires || ta->ai_next != NULL) {
    E_MOD("Cannot get address info on multicast client "
          << mcaddress << ":" << mcport << ", error " << gai_strerror(aires));
    return false;
  }

  if (mp_sockfd == -1) {
    if (!mp_port) {
      E_MOD("Need multiplay port configured first");
      return false;
    }

    // multiplayer settings, if applicable, receive socket
    mp_sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (mp_sockfd == -1) {
      E_MOD("Error creating multiplayer socket " << strerror(errno));
      return false;
    }

    // accept from any host, and bind to given port
    union {
      struct sockaddr_in recept_address;
      struct sockaddr use_address;
    } addru;
    memset(&addru, 0, sizeof(addru));

    // if interface specified, listen only there
    if (mp_interface.size()) {
      in_addr host_address;
      if (inet_aton(mp_interface.c_str(), &host_address)) {
        addru.recept_address.sin_addr = host_address;
      }
      else {
        E_MOD("Cannot convert own interface " << mp_interface);
        return false;
      }
    }
    addru.recept_address.sin_port = htons(mp_port);
    addru.recept_address.sin_family = AF_INET;
    if (::bind(mp_sockfd, &addru.use_address, sizeof(addru))) {
      E_MOD("Unable to bind to multiplay server port, error "
            << strerror(errno));
      return false;
    }
  }

  mp_clients.emplace_back(mp_sockfd, *(ta->ai_addr));

  // return address info
  freeaddrinfo(ta);

  return true;
}