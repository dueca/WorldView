/* ------------------------------------------------------------------   */
/*      item            : FlightGearViewer.hxx
        made by         : Rene van Paassen
        date            : 090616
        category        : header file
        description     :
        changes         : 090616 first version
        language        : C++
        status          : incomplete
*/

#ifndef FlightGearViewer_hxx
#define FlightGearViewer_hxx

#include "Dstring.hxx"
#include "SimTime.hxx"
#include <netinet/in.h>
#undef OTHER_TRAFFIC
#define MPJUSTSEND

#include "AxisTransform.hxx"
#include "FlightGearObject.hxx"
#include "MultiplayerEncode.hxx"
#include "comm-objects.h"
#include <WorldViewerBase.hxx>
#include <boost/shared_ptr.hpp>
#include <map>
#include <string>
#include <vector>

/** This is a class that can communicate with a flightgear
    visualisation. It derives from the WorldViewerBase, and can be used
    by the WorldView module
*/
class FlightGearViewer : public WorldViewerBase
{
  /** Socket for communication to flightgear. */
  int sockfd;

protected:
  /** Ip or host address for the data. */
  std::string receiver;

  /** Ip number of my own interface, used to select sending over only
      one interface. */
  std::string own_interface;

  /** Port number for the data packets. */
  int port;

  /** Destination for the packets. */
  struct sockaddr_in dest_address;

  /** Flag to do binary packets. Otherwise formatted ASCII */
  bool binary_packets;

  /** Visibility in boundary layer */
  double vis_boundary;

  /** Visibility aloft */
  double vis_aloft;

private:
  /** Current position */
  FlightGearCommand fg_command;

  /** Axis definition of the local axis (location and orientation),
      or, as a default, the */
  boost::shared_ptr<FGAxis> axis;

  /** Encoder object */
  boost::shared_ptr<MultiplayerEncode> encoder;

private:
  /** Latest tick according to update call */
  TimeTickType current_tick;

protected:
  /** Age to keep multiplayer clients active */
  TimeTickType retain_age;

private:
  /** Structure needed for one client, socket to send to, and
      age to remember activity  */
  struct MultiplayerClient
  {

    /** Sending socket */
    int sockfd;

    /** Reception address */
    struct sockaddr dest;

    /** Create new client */
    MultiplayerClient(int sockfd, const sockaddr &in);

    /** Take current encoded message, and copy to the client.

      */
    void update(const MultiplayerEncode &encoder) const;
  };

  /** Map to keep info on clients. */
  typedef std::list<MultiplayerClient> client_map_t;

  /** Map with clients */
  client_map_t mp_clients;

protected:

  /** Port number for multiplay messages */
  int mp_port;

  /** Radar range for multiplayer objects */
  float mp_radarrange;

  /** interface for UDP receive. */
  std::string mp_interface;

  /** multiplayer socket, output. */
  int mp_sockfd;

  /** multiplayer socket in */
  int mp_sockin;

private:

  /** Time offset */
  double mp_time0;

public:
  /** Constructor */
  FlightGearViewer();

  /** Destructor */
  ~FlightGearViewer();

    /** Initialise the communication */
  virtual bool complete();

  /** set the base camera position
      @param tick  DUECA current time tick
      @param base  Object motion, position, etc.
      @param late  Time after DUECA tick */
  void setBase(TimeTickType tick, const BaseObjectMotion &base, double late);

  /** Initialise the windows etc. */
  void init(bool waitswap){};

  /** Do a re-draw

      For FlightGear, this is simply sending the latest position to the
      fgfs process.
      \param wait   Not used here. */
  void redraw(bool wait = false, bool save_context = false);

  /** Wait for the swap. */
  void waitSwap();

    /** Set the origin point for a local coordinate system */
  bool setLatLonPsi0(const vector<double> &vec);

  /** Send other object data */
  inline MultiplayerEncode &getEncoder() { return *encoder; }

  /** Send multiplayer encoded report */
  void sendPositionReport();

  /** Flight time calculation */
  double getFlightTime(double time);

private:
    /** Map with created (due to presence in the world channel)
  objects. They are indexed with channel entry index, and removed
  from the map when the entry is removed from the channel. */
  typedef std::map<creation_key_t, boost::intrusive_ptr<FlightGearObject>>
    created_objects_t;

  /** Objects creates automatically */
  created_objects_t active_objects;

  /** Livery/flightgear class combination */
  struct FGSpecs
  {
    /** Class of vehicle as known in FlightGear */
    std::string fgclass;

    /** Livery / colours */
    std::string livery;

    /** Constructor, with strings */
    FGSpecs(const std::string &fgclass, const std::string &livery);

    /** Empty constructor, for the list */
    FGSpecs();
  };

  /** mapping between local class and FlighGear stuff */
  typedef std::map<std::string, FGSpecs> FGClassMap;

  /** mapping between local class and FlightGear stuff */
  FGClassMap flightgear_classes;

  /** Create a controllable object. Object creation depends on class of
      data supplied, further init may rely on fist data entering. */
  bool createControllable(const GlobalId &master_id, const NameSet &cname,
                          entryid_type entry_id, uint32_t creation_id,
                          const std::string &data_class,
                          const std::string &entry_label,
                          Channel::EntryTimeAspect time_aspect);

  /** Remove a controllable */
  void removeControllable(const dueca::NameSet &cname, uint32_t creation_id);

protected:
  /** Specify an additional mapping for the sim->fg classes */
  bool modelTableEntry(const std::vector<std::string> &s);

  /** Add a multiplayer client */
  bool addMultiplayClient(const std::string& s);
};

#endif
