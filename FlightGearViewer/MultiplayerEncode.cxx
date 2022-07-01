/* ------------------------------------------------------------------   */
/*      item            : MultiplayerEncode.cxx
        made by         : Rene' van Paassen
        date            : 100623
	category        : body file 
        description     : 
	changes         : 100623 first version
        language        : C++
*/

#define MultiplayerEncode_cxx
#include "MultiplayerEncode.hxx"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static const unsigned headersize = 32;
static const unsigned max_text_size = 128;

MultiplayerEncode::MultiplayerEncode(const std::string& receiver, 
				     unsigned short replyport,
				     const FGAxis &ax) :
  receiver(0),
  replyport(replyport),
  axis(ax)
{
  // zero the buffer to get any junk out
  memset(buffer, 0xff, sizeof(buffer));

  // to be on the safe size, initialize buffers
  xdrmem_create(&xdr_data, &buffer[headersize], 
		sizeof(buffer)-headersize, XDR_ENCODE);
  xdrmem_create(&xdr_header, buffer, headersize, XDR_ENCODE);

  // get reply address
  struct in_addr inaddr;
  int res = inet_aton(receiver.c_str(), &inaddr);
  if (!res) {
    cerr << "Cannot decode IP address \"" << receiver << '"' << endl;
  }
  else {
    this->receiver = inaddr.s_addr;
  }
}


MultiplayerEncode::~MultiplayerEncode()
{

}

void MultiplayerEncode::encode(const BaseObjectMotion& motion, 
			       const std::string& fgclass,
			       const std::string& livery,
                               const std::string& name,
			       double time, double lag)
{

  // re-initialize XDR buffers
  xdrmem_create(&xdr_data, &buffer[headersize], 
		sizeof(buffer)-headersize, XDR_ENCODE);
  xdrmem_create(&xdr_header, buffer, headersize, XDR_ENCODE);

  // ----------------------encode data-----------------------------------
  // aircraft model, fixed length 96; available models under 
  // /usr/share/FlightGear/Aircraft
  {
    unsigned size = fgclass.size() + 1;
    char *model_ptr = const_cast<char*>(fgclass.c_str());
    if (size > 96) size = 96;
    xdr_opaque(&xdr_data, model_ptr, size);
    xdr_setpos(&xdr_data, 96);
  } // 96

  // time, float
  xdr_double(&xdr_data, &time); // 104

  // lag, float
  xdr_double(&xdr_data, &lag); // 112

  // position, ECEF, 3 doubles
  double pos[3]; float velocity[3], attitude[4], omega[3];
  axis.toECEF(pos, velocity, attitude, omega, 
	      motion.xyz, motion.attitude_q, motion.uvw, motion.omega);
  xdr_double(&xdr_data, &pos[0]);
  xdr_double(&xdr_data, &pos[1]);
  xdr_double(&xdr_data, &pos[2]);  // 136
  
  // orientation, ECEF, 3 float, normalized quaternion rot vector
  xdr_float(&xdr_data, &attitude[1]);
  xdr_float(&xdr_data, &attitude[2]);
  xdr_float(&xdr_data, &attitude[3]); // 148

  // linear velocity, ECEF
  xdr_float(&xdr_data, &velocity[0]);
  xdr_float(&xdr_data, &velocity[1]);
  xdr_float(&xdr_data, &velocity[2]); // 160

  // angular velocify, ECEF
  xdr_float(&xdr_data, &omega[0]);
  xdr_float(&xdr_data, &omega[1]);
  xdr_float(&xdr_data, &omega[2]); // 172
  
  // linear acceleration, ECEF, zero for now
  float zero = 0.0f;
  xdr_float(&xdr_data, &zero);
  xdr_float(&xdr_data, &zero);
  xdr_float(&xdr_data, &zero); // 184

  // angular acceleration, ECEF, zero for now
  xdr_float(&xdr_data, &zero);
  xdr_float(&xdr_data, &zero);
  xdr_float(&xdr_data, &zero); // 196

  // padding, 4 bytes.
  xdr_float(&xdr_data, &zero); // 200
  assert(xdr_getpos(&xdr_data) == 200);

  if (livery.size()) {
    uint32_t livery_id = 10001;
    xdr_u_int(&xdr_data, &livery_id);
    //   unsigned size = livery.size();
    char *livery_ptr = const_cast<char*>(livery.c_str());
    bool_t res = xdr_string(&xdr_data, &livery_ptr, max_text_size);
    assert(res == 1);
  }


  // iterate over all properties
  // has to be expanded. See list in multiplaymgr.cxx for propery
  // names and ID's
  {
    // property id, uint32_t 

    // property value, int, float or string
    
  }    

  // ----------------------encode header-------------------------------- 

  // magic
  static uint32_t magic = 0x46474653;  // "FGFS"
  // use relay magic, so messages are not being sent back
  //static uint32_t magic = 0x53464746;    // GSGF
  xdr_u_int(&xdr_header, &magic); // 4
  
  // protocol version
  static uint32_t version = 0x00010001;  // 1.1
  xdr_u_int(&xdr_header, &version); // 8

  // message ID
  static uint32_t pos_data_id = 7;
  xdr_u_int(&xdr_header, &pos_data_id); // 12

  // message length
  unsigned totallength = this->getBufferSize();
  xdr_u_int(&xdr_header, &totallength); // 16

  // receiver address
  xdr_u_int(&xdr_header, &receiver);    // 20

  // receiver port
  xdr_u_int(&xdr_header, &replyport);   // 24
  
  // callsign (8 bytes)
  assert(xdr_getpos(&xdr_header) == 24);
  {
    unsigned size = name.size() + 1;
    char *name_ptr = const_cast<char*>(name.c_str());
    if (size > 8) {
      cerr << "Truncating " << name 
	   << " to 8 characters for callsign" << endl;
      size = 8;
    }
    xdr_opaque(&xdr_header, name_ptr, size);
    // xdr_bytes(&xdr_header, &name_ptr, &size, 8);
    xdr_setpos(&xdr_header, 32);
  }
}

unsigned MultiplayerEncode::getBufferSize()
{
  return headersize + xdr_getpos(&xdr_data);
}
