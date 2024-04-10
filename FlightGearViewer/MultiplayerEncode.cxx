/* ------------------------------------------------------------------   */
/*      item            : MultiplayerEncode.cxx
        made by         : Rene' van Paassen
        date            : 100623
        category        : body file
        description     :
        changes         : 100623 first version
        language        : C++
*/

#include "rpc/xdr.h"
#include <cstdint>
#define MultiplayerEncode_cxx
#include "MultiplayerEncode.hxx"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

static const unsigned headersize = 32;
static const unsigned max_text_size = 128;

MultiplayerEncode::MultiplayerEncode(const std::string &receiver,
                                     unsigned short replyport, const FGAxis &ax,
                                     float radarrange) :
  receiver(0),
  replyport(replyport),
  axis(ax),
  radarrange(radarrange)
{
  // zero the buffer to get any junk out
  memset(buffer, 0xff, sizeof(buffer));
  // to be on the safe size, initialize buffers
  xdrmem_create(&xdr_data, &buffer[headersize], sizeof(buffer) - headersize,
                XDR_ENCODE);
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

MultiplayerEncode::~MultiplayerEncode() {}

void MultiplayerEncode::encode(const BaseObjectMotion &motion,
                               const std::string &fgclass,
                               const std::string &livery,
                               const std::string &name, double time, double lag)
{

  // re-initialize XDR buffers
  xdrmem_create(&xdr_data, &buffer[headersize], sizeof(buffer) - headersize,
                XDR_ENCODE);
  xdrmem_create(&xdr_header, buffer, headersize, XDR_ENCODE);

  // ----------------------encode data-----------------------------------
  // aircraft model, fixed length 96; available models under
   // /usr/share/FlightGear/Aircraft
  {
    char model_ptr[96] = {};
    strncpy(model_ptr, fgclass.c_str(), min(fgclass.size(), 96UL));
    xdr_opaque(&xdr_data, model_ptr, 96);
  } // 96

  // time, double
  xdr_double(&xdr_data, &time); // 104

  // lag, double
  xdr_double(&xdr_data, &lag); // 112

  // position, ECEF, 3 doubles
  double pos[3];
  float velocity[3], attitude[4], omega[3];
  axis.toECEF(pos, velocity, attitude, omega, motion.xyz, motion.attitude_q,
              motion.uvw, motion.omega);
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
    uint32_t livery_id = 1101;
    xdr_u_int(&xdr_data, &livery_id);
    //   unsigned size = livery.size();
    char *livery_ptr = const_cast<char *>(livery.c_str());
    bool_t res = xdr_string(&xdr_data, &livery_ptr, max_text_size);
    assert(res == 1);
  }

  uint32_t chat_id = 10002;
  xdr_u_int(&xdr_data, &chat_id);
  const char *msg = "Hello";
  bool_t res = xdr_string(&xdr_data, const_cast<char **>(&msg), max_text_size);
  // iterate over all properties
  // has to be expanded. See list in multiplaymgr.cxx for propery
  // names and ID's
  {
    // property id, uint32_t

     // property value, int, float or string
  }
  // update current size counter
  bufsize = headersize + xdr_getpos(&xdr_data);
      // ----------------------encode header--------------------------------

   // magic
  static uint32_t magic = 0x46474653;  // "FGFS"
  // use relay magic, so messages are not being sent back
  // static uint32_t magic = 0x53464746;    // GSGF
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

  // radar range
  xdr_float(&xdr_header, &radarrange);       // 20

  // receiver port, unused
  xdr_u_int(&xdr_header, &replyport);   // 24

  // callsign (8 bytes)
  assert(xdr_getpos(&xdr_header) == 24);
  {
    char name_ptr[8] = {};
    strncpy(name_ptr, name.c_str(), min(name.size(), 8UL));
    xdr_opaque(&xdr_header, name_ptr, 8);
    // xdr_bytes(&xdr_header, &name_ptr, &size, 8);
    xdr_setpos(&xdr_header, 32);
  }
}

#define PHEX(name) \
uint32_t name; \
xdr_u_int(&xdr_data, &name); \
std::cout << #name << "=" << std::hex << name << std::dec << std::endl;

#define PINT(name) \
uint32_t name; \
xdr_u_int(&xdr_data, &name); \
std::cout << #name << "=" << name << std::endl;

#define PFLOAT(name) \
float name; \
xdr_float(&xdr_data, &name); \
std::cout << #name << "=" << name << std::endl;

#define PDOUBLE(name) \
double name; \
xdr_double(&xdr_data, &name); \
std::cout << #name << "=" << name << std::endl;

#define PTEXT(name, N) \
char name[N]; \
xdr_opaque(&xdr_data, name, N); \
std::cout << #name << "=" << name << std::endl;

struct PropBase
{
  const std::string name;
  PropBase(const std::string& name) : name(name) {}

  virtual void dump(XDR& xdr_data) = 0;
  virtual void dump(short int value, XDR& xdr_data) {
    std::cerr << "Wrong dump " << name << std::endl;
  }
};

struct PropInt: public PropBase {
  PropInt(const std::string& name) : PropBase(name) {}

  void dump(XDR& xdr_data) {
    int val; xdr_int(&xdr_data, &val);
    std::cout << name << "=" << val << std::endl;
  }
};

struct PropShort: public PropBase {
  PropShort(const std::string& name) : PropBase(name) {}

  void dump(XDR& xdr_data) {
    short int val; xdr_short(&xdr_data, &val);
    dump(val, xdr_data);
  }

  void dump(short int val, XDR& xdr_data) final {
    std::cout << name << "=" << val << std::endl;
  }

};

struct PropBool: public PropBase {
  PropBool(const std::string& name) : PropBase(name) {}

  void dump(XDR& xdr_data) final {
    int val; xdr_bool(&xdr_data, &val);
    dump(val, xdr_data);
  }

  void dump(short int val, XDR& xdr_data) final {
    std::cout << name << "=" << val << std::endl;
  }
};

struct PropBoolArray: public PropBase {
  PropBoolArray(const std::string& name) : PropBase(name) {}

  void dump(XDR& xdr_data) final {
    uint32_t val; xdr_u_int(&xdr_data, &val);
    std::cout << name << "=";
    for (unsigned bidx = 0; bidx < 30; bidx++) {
      std::cout << ((val & (1 << bidx)) != 0);
    }
    std::cout << std::endl;
  }
};

struct PropFloat: public PropBase {
  PropFloat(const std::string& name) : PropBase(name) {}

  void dump(XDR& xdr_data) {
    float val; xdr_float(&xdr_data, &val);
    std::cout << name << "=" << val << std::endl;
  }
};

template<int scale>
struct PropShortFloat: public PropBase {
  PropShortFloat(const std::string& name) : PropBase(name) {}

  void dump(XDR& xdr_data) final {
    int16_t val; xdr_short(&xdr_data, &val);
    dump(val, xdr_data);
  }

  void dump(short int val, XDR& xdr_data) final {
    std::cout << name << "=" << val/float(scale) << std::endl;
  }
};

struct PropString: public PropBase {

  PropString(const std::string& name) : PropBase(name) {}

  void dump(XDR& xdr_data) {
    unsigned int len; xdr_u_int32_t(&xdr_data, &len);
    char val[64] = {};
    xdr_opaque(&xdr_data, val, len);

    //char val[64]; xdr_string(&xdr_data, reinterpret_cast<char**>(&val), 64);
    std::cout << name << "=" << val << std::endl;
  }
  void dump(short int len, XDR& xdr_data) final {
    char val[64] = {};
    xdr_opaque(&xdr_data, reinterpret_cast<char*>(val), unsigned(len));
    std::cout << name << "=s=" << val << std::endl;
  }


};

typedef std::map<uint32_t,PropBase*> propmap_t;
const int BOOLARRAY_BLOCKSIZE = 40;

const int BOOLARRAY_BASE_1 = 11000;
const int BOOLARRAY_BASE_2 = BOOLARRAY_BASE_1 + BOOLARRAY_BLOCKSIZE;
const int BOOLARRAY_BASE_3 = BOOLARRAY_BASE_2 + BOOLARRAY_BLOCKSIZE;
const int V2018_1_BASE = 11990;
const int V2018_3_BASE = 13000;

void MultiplayerEncode::dump(const char* buffer, size_t bufsize)
{
  XDR xdr_data;
  xdrmem_create(&xdr_data, const_cast<char *>(buffer), bufsize, XDR_DECODE);

  static propmap_t propmap = {
    std::make_pair(10, new PropShort("sim/multiplay/protocol-version")),
    std::make_pair(100, new PropShortFloat<32767>("surface-positions/left-aileron-pos-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(101, new PropShortFloat<32767>("surface-positions/right-aileron-pos-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(102, new PropShortFloat<32767>("surface-positions/elevator-pos-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(103, new PropShortFloat<32767>("surface-positions/rudder-pos-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(104, new PropShortFloat<32767>("surface-positions/flap-pos-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(105, new PropShortFloat<32767>("surface-positions/speedbrake-pos-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(106, new PropShortFloat<32767>("gear/tailhook/position-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(107, new PropShortFloat<32767>("gear/launchbar/position-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    //
    std::make_pair(108, new PropString("gear/launchbar/state")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, encode_launchbar_state_for_transmission, NULL },
    std::make_pair(109, new PropShortFloat<32767>("gear/launchbar/holdback-position-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(110, new PropShortFloat<32767>("canopy/position-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(111, new PropShortFloat<32767>("surface-positions/wing-pos-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(112, new PropShortFloat<32767>("surface-positions/wing-fold-pos-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },

    // to enable decoding this is the transient ID record that is in the packet. This is not sent directly - instead it is the result
    // of the conversion of property 108.
    std::make_pair(120, new PropInt("gear/launchbar/state-value")),  //::INT, TT_NOSEND,  V1_1_2_PROP_ID, NULL, decode_received_launchbar_state },

    std::make_pair(200, new PropShortFloat<32767>("gear/gear[0]/compression-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(201, new PropShortFloat<32767>("gear/gear[0]/position-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(210, new PropShortFloat<32767>("gear/gear[1]/compression-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(211, new PropShortFloat<32767>("gear/gear[1]/position-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(220, new PropShortFloat<32767>("gear/gear[2]/compression-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(221, new PropShortFloat<32767>("gear/gear[2]/position-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(230, new PropShortFloat<32767>("gear/gear[3]/compression-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(231, new PropShortFloat<32767>("gear/gear[3]/position-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(240, new PropShortFloat<32767>("gear/gear[4]/compression-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(241, new PropShortFloat<32767>("gear/gear[4]/position-norm")),  //::FLOAT, TT_SHORT_FLOAT_NORM,  V1_1_PROP_ID, NULL, NULL },

    std::make_pair(300, new PropShortFloat<10>("engines/engine[0]/n1")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(301, new PropShortFloat<10>("engines/engine[0]/n2")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(302, new PropShortFloat<10>("engines/engine[0]/rpm")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(310, new PropShortFloat<10>("engines/engine[1]/n1")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(311, new PropShortFloat<10>("engines/engine[1]/n2")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(312, new PropShortFloat<10>("engines/engine[1]/rpm")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(320, new PropShortFloat<10>("engines/engine[2]/n1")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(321, new PropShortFloat<10>("engines/engine[2]/n2")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(322, new PropShortFloat<10>("engines/engine[2]/rpm")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(330, new PropShortFloat<10>("engines/engine[3]/n1")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(331, new PropShortFloat<10>("engines/engine[3]/n2")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(332, new PropShortFloat<10>("engines/engine[3]/rpm")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(340, new PropShortFloat<10>("engines/engine[4]/n1")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(341, new PropShortFloat<10>("engines/engine[4]/n2")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(342, new PropShortFloat<10>("engines/engine[4]/rpm")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(350, new PropShortFloat<10>("engines/engine[5]/n1")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(351, new PropShortFloat<10>("engines/engine[5]/n2")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(352, new PropShortFloat<10>("engines/engine[5]/rpm")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(360, new PropShortFloat<10>("engines/engine[6]/n1")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(361, new PropShortFloat<10>("engines/engine[6]/n2")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(362, new PropShortFloat<10>("engines/engine[6]/rpm")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(370, new PropShortFloat<10>("engines/engine[7]/n1")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(371, new PropShortFloat<10>("engines/engine[7]/n2")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(372, new PropShortFloat<10>("engines/engine[7]/rpm")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(380, new PropShortFloat<10>("engines/engine[8]/n1")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(381, new PropShortFloat<10>("engines/engine[8]/n2")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(382, new PropShortFloat<10>("engines/engine[8]/rpm")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(390, new PropShortFloat<10>("engines/engine[9]/n1")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(391, new PropShortFloat<10>("engines/engine[9]/n2")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(392, new PropShortFloat<10>("engines/engine[9]/rpm")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },

    std::make_pair(800, new PropShortFloat<10>("rotors/main/rpm")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(801, new PropShortFloat<10>("rotors/tail/rpm")),  //::FLOAT, TT_SHORT_FLOAT_1,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(810, new PropShortFloat<1000>("rotors/main/blade[0]/position-deg")),  //::FLOAT, TT_SHORT_FLOAT_3,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(811, new PropShortFloat<1000>("rotors/main/blade[1]/position-deg")),  //::FLOAT, TT_SHORT_FLOAT_3,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(812, new PropShortFloat<1000>("rotors/main/blade[2]/position-deg")),  //::FLOAT, TT_SHORT_FLOAT_3,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(813, new PropShortFloat<1000>("rotors/main/blade[3]/position-deg")),  //::FLOAT, TT_SHORT_FLOAT_3,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(820, new PropShortFloat<1000>("rotors/main/blade[0]/flap-deg")),  //::FLOAT, TT_SHORT_FLOAT_3,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(821, new PropShortFloat<1000>("rotors/main/blade[1]/flap-deg")),  //::FLOAT, TT_SHORT_FLOAT_3,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(822, new PropShortFloat<1000>("rotors/main/blade[2]/flap-deg")),  //::FLOAT, TT_SHORT_FLOAT_3,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(823, new PropShortFloat<1000>("rotors/main/blade[3]/flap-deg")),  //::FLOAT, TT_SHORT_FLOAT_3,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(830, new PropShortFloat<1000>("rotors/tail/blade[0]/position-deg")),  //::FLOAT, TT_SHORT_FLOAT_3,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(831, new PropShortFloat<1000>("rotors/tail/blade[1]/position-deg")),  //::FLOAT, TT_SHORT_FLOAT_3,  V1_1_PROP_ID, NULL, NULL },

    std::make_pair(900, new PropFloat("sim/hitches/aerotow/tow/length")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(901, new PropFloat("sim/hitches/aerotow/tow/elastic-constant")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(902, new PropFloat("sim/hitches/aerotow/tow/weight-per-m-kg-m")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(903, new PropFloat("sim/hitches/aerotow/tow/dist")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(904, new PropBool("sim/hitches/aerotow/tow/connected-to-property-node")),  //::BOOL, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(905, new PropString("sim/hitches/aerotow/tow/connected-to-ai-or-mp-callsign")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(906, new PropFloat("sim/hitches/aerotow/tow/break-force")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(907, new PropFloat("sim/hitches/aerotow/tow/end-force-x")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(908, new PropFloat("sim/hitches/aerotow/tow/end-force-y")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(909, new PropFloat("sim/hitches/aerotow/tow/end-force-z")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(930, new PropBool("sim/hitches/aerotow/is-slave")),  //::BOOL, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(931, new PropFloat("sim/hitches/aerotow/speed-in-tow-direction")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(932, new PropBool("sim/hitches/aerotow/open")),  //::BOOL, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(933, new PropFloat("sim/hitches/aerotow/local-pos-x")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(934, new PropFloat("sim/hitches/aerotow/local-pos-y")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(935, new PropFloat("sim/hitches/aerotow/local-pos-z")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },

    std::make_pair(1001, new PropShortFloat<10000>("controls/flight/slats")),  //::FLOAT, TT_SHORT_FLOAT_4,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(1002, new PropShortFloat<10000>("controls/flight/speedbrake")),  //::FLOAT, TT_SHORT_FLOAT_4,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(1003, new PropShortFloat<10000>("controls/flight/spoilers")),  //::FLOAT, TT_SHORT_FLOAT_4,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(1004, new PropShortFloat<10000>("controls/gear/gear-down")),  //::FLOAT, TT_SHORT_FLOAT_4,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(1005, new PropShortFloat<1000>("controls/lighting/nav-lights")),  //::FLOAT, TT_SHORT_FLOAT_3,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(1006, new PropBool("controls/armament/station[0]/jettison-all")),  //::BOOL, TT_SHORTINT,  V1_1_PROP_ID, NULL, NULL },

    std::make_pair(1100, new PropInt("sim/model/variant")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(1101, new PropString("sim/model/livery/file")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },

    std::make_pair(1200, new PropString("environment/wildfire/data")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(1201, new PropShort("environment/contrail")),  //::INT, TT_SHORTINT,  V1_1_PROP_ID, NULL, NULL },

    std::make_pair(1300, new PropShort("tanker")),  //::INT, TT_SHORTINT,  V1_1_PROP_ID, NULL, NULL },

    std::make_pair(1400, new PropString("scenery/events")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },

    std::make_pair(1500, new PropShort("instrumentation/transponder/transmitted-id")),  //::INT, TT_SHORTINT,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(1501, new PropShort("instrumentation/transponder/altitude")),  //::INT, TT_ASIS, V1_1_PROP_ID, NULL, NULL },
    std::make_pair(1502, new PropShort("instrumentation/transponder/ident")),  //::BOOL, TT_SHORTINT, V1_1_PROP_ID, NULL, NULL },
    std::make_pair(1503, new PropShort("instrumentation/transponder/inputs/mode")),  //::INT, TT_SHORTINT, V1_1_PROP_ID, NULL, NULL },
    std::make_pair(1504, new PropShort("instrumentation/transponder/ground-bit")),  //::BOOL, TT_SHORTINT, V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(1505, new PropShort("instrumentation/transponder/airspeed-kt")),  //::INT, TT_SHORTINT, V1_1_2_PROP_ID, NULL, NULL },

    std::make_pair(10001, new PropString("sim/multiplay/transmission-freq-hz")),  //::STRING, TT_NOSEND,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10002, new PropString("sim/multiplay/chat")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },

    std::make_pair(10100, new PropString("sim/multiplay/generic/string[0]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10101, new PropString("sim/multiplay/generic/string[1]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10102, new PropString("sim/multiplay/generic/string[2]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10103, new PropString("sim/multiplay/generic/string[3]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10104, new PropString("sim/multiplay/generic/string[4]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10105, new PropString("sim/multiplay/generic/string[5]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10106, new PropString("sim/multiplay/generic/string[6]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10107, new PropString("sim/multiplay/generic/string[7]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10108, new PropString("sim/multiplay/generic/string[8]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10109, new PropString("sim/multiplay/generic/string[9]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10110, new PropString("sim/multiplay/generic/string[10]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10111, new PropString("sim/multiplay/generic/string[11]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10112, new PropString("sim/multiplay/generic/string[12]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10113, new PropString("sim/multiplay/generic/string[13]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10114, new PropString("sim/multiplay/generic/string[14]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10115, new PropString("sim/multiplay/generic/string[15]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10116, new PropString("sim/multiplay/generic/string[16]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10117, new PropString("sim/multiplay/generic/string[17]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10118, new PropString("sim/multiplay/generic/string[18]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10119, new PropString("sim/multiplay/generic/string[19]")),  //::STRING, TT_ASIS,  V1_1_2_PROP_ID, NULL, NULL },

    std::make_pair(10200, new PropFloat("sim/multiplay/generic/float[0]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10201, new PropFloat("sim/multiplay/generic/float[1]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10202, new PropFloat("sim/multiplay/generic/float[2]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10203, new PropFloat("sim/multiplay/generic/float[3]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10204, new PropFloat("sim/multiplay/generic/float[4]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10205, new PropFloat("sim/multiplay/generic/float[5]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10206, new PropFloat("sim/multiplay/generic/float[6]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10207, new PropFloat("sim/multiplay/generic/float[7]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10208, new PropFloat("sim/multiplay/generic/float[8]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10209, new PropFloat("sim/multiplay/generic/float[9]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10210, new PropFloat("sim/multiplay/generic/float[10]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10211, new PropFloat("sim/multiplay/generic/float[11]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10212, new PropFloat("sim/multiplay/generic/float[12]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10213, new PropFloat("sim/multiplay/generic/float[13]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10214, new PropFloat("sim/multiplay/generic/float[14]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10215, new PropFloat("sim/multiplay/generic/float[15]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10216, new PropFloat("sim/multiplay/generic/float[16]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10217, new PropFloat("sim/multiplay/generic/float[17]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10218, new PropFloat("sim/multiplay/generic/float[18]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10219, new PropFloat("sim/multiplay/generic/float[19]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },

    std::make_pair(10220, new PropFloat("sim/multiplay/generic/float[20]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10221, new PropFloat("sim/multiplay/generic/float[21]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10222, new PropFloat("sim/multiplay/generic/float[22]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10223, new PropFloat("sim/multiplay/generic/float[23]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10224, new PropFloat("sim/multiplay/generic/float[24]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10225, new PropFloat("sim/multiplay/generic/float[25]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10226, new PropFloat("sim/multiplay/generic/float[26]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10227, new PropFloat("sim/multiplay/generic/float[27]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10228, new PropFloat("sim/multiplay/generic/float[28]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10229, new PropFloat("sim/multiplay/generic/float[29]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10230, new PropFloat("sim/multiplay/generic/float[30]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10231, new PropFloat("sim/multiplay/generic/float[31]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10232, new PropFloat("sim/multiplay/generic/float[32]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10233, new PropFloat("sim/multiplay/generic/float[33]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10234, new PropFloat("sim/multiplay/generic/float[34]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10235, new PropFloat("sim/multiplay/generic/float[35]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10236, new PropFloat("sim/multiplay/generic/float[36]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10237, new PropFloat("sim/multiplay/generic/float[37]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10238, new PropFloat("sim/multiplay/generic/float[38]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10239, new PropFloat("sim/multiplay/generic/float[39]")),  //::FLOAT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },

    std::make_pair(10300, new PropInt("sim/multiplay/generic/int[0]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10301, new PropInt("sim/multiplay/generic/int[1]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10302, new PropInt("sim/multiplay/generic/int[2]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10303, new PropInt("sim/multiplay/generic/int[3]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10304, new PropInt("sim/multiplay/generic/int[4]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10305, new PropInt("sim/multiplay/generic/int[5]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10306, new PropInt("sim/multiplay/generic/int[6]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10307, new PropInt("sim/multiplay/generic/int[7]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10308, new PropInt("sim/multiplay/generic/int[8]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10309, new PropInt("sim/multiplay/generic/int[9]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10310, new PropInt("sim/multiplay/generic/int[10]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10311, new PropInt("sim/multiplay/generic/int[11]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10312, new PropInt("sim/multiplay/generic/int[12]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10313, new PropInt("sim/multiplay/generic/int[13]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10314, new PropInt("sim/multiplay/generic/int[14]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10315, new PropInt("sim/multiplay/generic/int[15]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10316, new PropInt("sim/multiplay/generic/int[16]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10317, new PropInt("sim/multiplay/generic/int[17]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10318, new PropInt("sim/multiplay/generic/int[18]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },
    std::make_pair(10319, new PropInt("sim/multiplay/generic/int[19]")),  //::INT, TT_ASIS,  V1_1_PROP_ID, NULL, NULL },

    std::make_pair(10500, new PropShort("sim/multiplay/generic/short[0]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10501, new PropShort("sim/multiplay/generic/short[1]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10502, new PropShort("sim/multiplay/generic/short[2]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10503, new PropShort("sim/multiplay/generic/short[3]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10504, new PropShort("sim/multiplay/generic/short[4]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10505, new PropShort("sim/multiplay/generic/short[5]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10506, new PropShort("sim/multiplay/generic/short[6]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10507, new PropShort("sim/multiplay/generic/short[7]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10508, new PropShort("sim/multiplay/generic/short[8]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10509, new PropShort("sim/multiplay/generic/short[9]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10510, new PropShort("sim/multiplay/generic/short[10]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10511, new PropShort("sim/multiplay/generic/short[11]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10512, new PropShort("sim/multiplay/generic/short[12]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10513, new PropShort("sim/multiplay/generic/short[13]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10514, new PropShort("sim/multiplay/generic/short[14]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10515, new PropShort("sim/multiplay/generic/short[15]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10516, new PropShort("sim/multiplay/generic/short[16]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10517, new PropShort("sim/multiplay/generic/short[17]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10518, new PropShort("sim/multiplay/generic/short[18]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10519, new PropShort("sim/multiplay/generic/short[19]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10520, new PropShort("sim/multiplay/generic/short[20]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10521, new PropShort("sim/multiplay/generic/short[21]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10522, new PropShort("sim/multiplay/generic/short[22]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10523, new PropShort("sim/multiplay/generic/short[23]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10524, new PropShort("sim/multiplay/generic/short[24]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10525, new PropShort("sim/multiplay/generic/short[25]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10526, new PropShort("sim/multiplay/generic/short[26]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10527, new PropShort("sim/multiplay/generic/short[27]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10528, new PropShort("sim/multiplay/generic/short[28]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10529, new PropShort("sim/multiplay/generic/short[29]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10530, new PropShort("sim/multiplay/generic/short[30]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10531, new PropShort("sim/multiplay/generic/short[31]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10532, new PropShort("sim/multiplay/generic/short[32]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10533, new PropShort("sim/multiplay/generic/short[33]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10534, new PropShort("sim/multiplay/generic/short[34]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10535, new PropShort("sim/multiplay/generic/short[35]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10536, new PropShort("sim/multiplay/generic/short[36]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10537, new PropShort("sim/multiplay/generic/short[37]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10538, new PropShort("sim/multiplay/generic/short[38]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10539, new PropShort("sim/multiplay/generic/short[39]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10540, new PropShort("sim/multiplay/generic/short[40]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10541, new PropShort("sim/multiplay/generic/short[41]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10542, new PropShort("sim/multiplay/generic/short[42]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10543, new PropShort("sim/multiplay/generic/short[43]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10544, new PropShort("sim/multiplay/generic/short[44]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10545, new PropShort("sim/multiplay/generic/short[45]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10546, new PropShort("sim/multiplay/generic/short[46]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10547, new PropShort("sim/multiplay/generic/short[47]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10548, new PropShort("sim/multiplay/generic/short[48]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10549, new PropShort("sim/multiplay/generic/short[49]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10550, new PropShort("sim/multiplay/generic/short[50]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10551, new PropShort("sim/multiplay/generic/short[51]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10552, new PropShort("sim/multiplay/generic/short[52]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10553, new PropShort("sim/multiplay/generic/short[53]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10554, new PropShort("sim/multiplay/generic/short[54]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10555, new PropShort("sim/multiplay/generic/short[55]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10556, new PropShort("sim/multiplay/generic/short[56]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10557, new PropShort("sim/multiplay/generic/short[57]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10558, new PropShort("sim/multiplay/generic/short[58]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10559, new PropShort("sim/multiplay/generic/short[59]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10560, new PropShort("sim/multiplay/generic/short[60]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10561, new PropShort("sim/multiplay/generic/short[61]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10562, new PropShort("sim/multiplay/generic/short[62]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10563, new PropShort("sim/multiplay/generic/short[63]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10564, new PropShort("sim/multiplay/generic/short[64]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10565, new PropShort("sim/multiplay/generic/short[65]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10566, new PropShort("sim/multiplay/generic/short[66]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10567, new PropShort("sim/multiplay/generic/short[67]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10568, new PropShort("sim/multiplay/generic/short[68]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10569, new PropShort("sim/multiplay/generic/short[69]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10570, new PropShort("sim/multiplay/generic/short[70]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10571, new PropShort("sim/multiplay/generic/short[71]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10572, new PropShort("sim/multiplay/generic/short[72]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10573, new PropShort("sim/multiplay/generic/short[73]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10574, new PropShort("sim/multiplay/generic/short[74]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10575, new PropShort("sim/multiplay/generic/short[75]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10576, new PropShort("sim/multiplay/generic/short[76]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10577, new PropShort("sim/multiplay/generic/short[77]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10578, new PropShort("sim/multiplay/generic/short[78]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(10579, new PropShort("sim/multiplay/generic/short[79]")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },

   std::make_pair(BOOLARRAY_BASE_1 +  0, new PropBoolArray("sim/multiplay/generic/bool[0]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 +  1, new PropBoolArray("sim/multiplay/generic/bool[1]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 +  2, new PropBoolArray("sim/multiplay/generic/bool[2]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 +  3, new PropBoolArray("sim/multiplay/generic/bool[3]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 +  4, new PropBoolArray("sim/multiplay/generic/bool[4]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 +  5, new PropBoolArray("sim/multiplay/generic/bool[5]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 +  6, new PropBoolArray("sim/multiplay/generic/bool[6]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 +  7, new PropBoolArray("sim/multiplay/generic/bool[7]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 +  8, new PropBoolArray("sim/multiplay/generic/bool[8]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 +  9, new PropBoolArray("sim/multiplay/generic/bool[9]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 10, new PropBoolArray("sim/multiplay/generic/bool[10]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 11, new PropBoolArray("sim/multiplay/generic/bool[11]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 12, new PropBoolArray("sim/multiplay/generic/bool[12]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 13, new PropBoolArray("sim/multiplay/generic/bool[13]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 14, new PropBoolArray("sim/multiplay/generic/bool[14]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 15, new PropBoolArray("sim/multiplay/generic/bool[15]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 16, new PropBoolArray("sim/multiplay/generic/bool[16]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 17, new PropBoolArray("sim/multiplay/generic/bool[17]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 18, new PropBoolArray("sim/multiplay/generic/bool[18]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 19, new PropBoolArray("sim/multiplay/generic/bool[19]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 20, new PropBoolArray("sim/multiplay/generic/bool[20]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 21, new PropBoolArray("sim/multiplay/generic/bool[21]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 22, new PropBoolArray("sim/multiplay/generic/bool[22]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 23, new PropBoolArray("sim/multiplay/generic/bool[23]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 24, new PropBoolArray("sim/multiplay/generic/bool[24]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 25, new PropBoolArray("sim/multiplay/generic/bool[25]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 26, new PropBoolArray("sim/multiplay/generic/bool[26]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 27, new PropBoolArray("sim/multiplay/generic/bool[27]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 28, new PropBoolArray("sim/multiplay/generic/bool[28]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 29, new PropBoolArray("sim/multiplay/generic/bool[29]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_1 + 30, new PropBoolArray("sim/multiplay/generic/bool[30]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },

    std::make_pair(BOOLARRAY_BASE_2 + 0, new PropBoolArray("sim/multiplay/generic/bool[31]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 1, new PropBoolArray("sim/multiplay/generic/bool[32]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 2, new PropBoolArray("sim/multiplay/generic/bool[33]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 3, new PropBoolArray("sim/multiplay/generic/bool[34]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 4, new PropBoolArray("sim/multiplay/generic/bool[35]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 5, new PropBoolArray("sim/multiplay/generic/bool[36]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 6, new PropBoolArray("sim/multiplay/generic/bool[37]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 7, new PropBoolArray("sim/multiplay/generic/bool[38]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 8, new PropBoolArray("sim/multiplay/generic/bool[39]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 9, new PropBoolArray("sim/multiplay/generic/bool[40]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 10, new PropBoolArray("sim/multiplay/generic/bool[41]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
        // out of sequence between the block and the buffer becuase of a typo. repurpose the first as that way [72] will work
        // correctly on older versions.
    std::make_pair(BOOLARRAY_BASE_2 + 11, new PropBoolArray("sim/multiplay/generic/bool[91]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 12, new PropBoolArray("sim/multiplay/generic/bool[42]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 13, new PropBoolArray("sim/multiplay/generic/bool[43]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 14, new PropBoolArray("sim/multiplay/generic/bool[44]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 15, new PropBoolArray("sim/multiplay/generic/bool[45]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 16, new PropBoolArray("sim/multiplay/generic/bool[46]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 17, new PropBoolArray("sim/multiplay/generic/bool[47]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 18, new PropBoolArray("sim/multiplay/generic/bool[48]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 19, new PropBoolArray("sim/multiplay/generic/bool[49]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 20, new PropBoolArray("sim/multiplay/generic/bool[50]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 21, new PropBoolArray("sim/multiplay/generic/bool[51]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 22, new PropBoolArray("sim/multiplay/generic/bool[52]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 23, new PropBoolArray("sim/multiplay/generic/bool[53]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 24, new PropBoolArray("sim/multiplay/generic/bool[54]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 25, new PropBoolArray("sim/multiplay/generic/bool[55]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 26, new PropBoolArray("sim/multiplay/generic/bool[56]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 27, new PropBoolArray("sim/multiplay/generic/bool[57]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 28, new PropBoolArray("sim/multiplay/generic/bool[58]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 29, new PropBoolArray("sim/multiplay/generic/bool[59]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_2 + 30, new PropBoolArray("sim/multiplay/generic/bool[60]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },

    std::make_pair(BOOLARRAY_BASE_3 + 0, new PropBoolArray("sim/multiplay/generic/bool[61]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 1, new PropBoolArray("sim/multiplay/generic/bool[62]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 2, new PropBoolArray("sim/multiplay/generic/bool[63]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 3, new PropBoolArray("sim/multiplay/generic/bool[64]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 4, new PropBoolArray("sim/multiplay/generic/bool[65]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 5, new PropBoolArray("sim/multiplay/generic/bool[66]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 6, new PropBoolArray("sim/multiplay/generic/bool[67]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 7, new PropBoolArray("sim/multiplay/generic/bool[68]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 8, new PropBoolArray("sim/multiplay/generic/bool[69]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 9, new PropBoolArray("sim/multiplay/generic/bool[70]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 10, new PropBoolArray("sim/multiplay/generic/bool[71]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
        // out of sequence between the block and the buffer becuase of a typo. repurpose the first as that way [72] will work
        // correctly on older versions.
    std::make_pair(BOOLARRAY_BASE_3 + 11, new PropBoolArray("sim/multiplay/generic/bool[92]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 12, new PropBoolArray("sim/multiplay/generic/bool[72]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 13, new PropBoolArray("sim/multiplay/generic/bool[73]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 14, new PropBoolArray("sim/multiplay/generic/bool[74]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 15, new PropBoolArray("sim/multiplay/generic/bool[75]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 16, new PropBoolArray("sim/multiplay/generic/bool[76]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 17, new PropBoolArray("sim/multiplay/generic/bool[77]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 18, new PropBoolArray("sim/multiplay/generic/bool[78]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 19, new PropBoolArray("sim/multiplay/generic/bool[79]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 20, new PropBoolArray("sim/multiplay/generic/bool[80]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 21, new PropBoolArray("sim/multiplay/generic/bool[81]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 22, new PropBoolArray("sim/multiplay/generic/bool[82]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 23, new PropBoolArray("sim/multiplay/generic/bool[83]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 24, new PropBoolArray("sim/multiplay/generic/bool[84]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 25, new PropBoolArray("sim/multiplay/generic/bool[85]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 26, new PropBoolArray("sim/multiplay/generic/bool[86]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 27, new PropBoolArray("sim/multiplay/generic/bool[87]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 28, new PropBoolArray("sim/multiplay/generic/bool[88]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 29, new PropBoolArray("sim/multiplay/generic/bool[89]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },
    std::make_pair(BOOLARRAY_BASE_3 + 30, new PropBoolArray("sim/multiplay/generic/bool[90]")),  //::BOOL, TT_BOOLARRAY,  V1_1_2_PROP_ID, NULL, NULL },


    std::make_pair(V2018_1_BASE + 0,  new PropShort("sim/multiplay/mp-clock-mode")),  //::INT, TT_SHORTINT,  V1_1_2_PROP_ID, NULL, NULL },


  };

  PHEX(magic);
  PHEX(version);
  PINT(msgid);
  PINT(msglen);
  PINT(radarrange);
  PINT(unused);
  PTEXT(name, 8UL);
  PTEXT(model, 96UL);
  PDOUBLE(time);
  PDOUBLE(lag);
  PDOUBLE(x);
  PDOUBLE(y);
  PDOUBLE(z);
  PFLOAT(rx);
  PFLOAT(ry);
  PFLOAT(rz);
  PFLOAT(u);
  PFLOAT(v);
  PFLOAT(w);
  PFLOAT(wx);
  PFLOAT(wy);
  PFLOAT(wz);
  PFLOAT(du);
  PFLOAT(dv);
  PFLOAT(dw);
  PFLOAT(dwx);
  PFLOAT(dwy);
  PFLOAT(dwz);
  PHEX(padding);

  bool knowprop = true;
  while (knowprop && xdr_getpos(&xdr_data) < bufsize) {
    uint32_t prid;
    short int value;
    bool shortencode = false;
    xdr_u_int(&xdr_data, &prid);
    if (prid & 0xffff0000) {
      value = prid & 0xffff;
      prid = prid >> 16;
      shortencode = true;
    }
    auto prop = propmap.find(prid);
    if (prop == propmap.end()) {
      std::cout << "could not find prop " << prid << std::endl;
      knowprop = false;
    }
    else {
      std::cout << "0x" << std::setw(4) << std::hex << prop->first << std::dec << std::setw(0) << " :";
      if (shortencode) {
        prop->second->dump(value, xdr_data);
      }
      else {
        prop->second->dump(xdr_data);
      }
    }
  }
}