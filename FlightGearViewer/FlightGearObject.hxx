/* ------------------------------------------------------------------   */
/*      item            : FlightGearObject.hxx
        made by         : Rene van Paassen
        date            : 100707
        category        : header file
        description     :
        changes         : 100707 first version
        language        : C++
*/

#ifndef FlightGearObject_hxx
#define FlightGearObject_hxx

#include "MultiplayerEncode.hxx"
#include "WorldDataSpec.hxx"
#include "comm-objects.h"
#include <WorldObjectBase.hxx>
#include <boost/scoped_ptr.hpp>
#include <dueca.h>
#include <dueca_ns.h>
#include <string>

class FlightGearViewer;

/** Memory for "other" players to be transmitted to a FlightGear
    server */
class FlightGearObject : public WorldObjectBase
{
protected:
  /** Channel read token for motion input */
  boost::scoped_ptr<ChannelReadToken> r_motion;

  /** time of the previous update */
  double itime;

  /** Name of this thing. */
  std::string name;

  /** Aircraft class as known to FlightGear. */
  std::string fgclass;

  /** Pointer to the boss, for conversion and sending of the data */
  const FlightGearViewer *master;

  /** Encoder for a JSON fixed data file */
  boost::scoped_ptr<MultiplayerEncode::PropertyEncoderBase> coder;

public:
  /** Constructor */
  FlightGearObject(const WorldDataSpec& spec);

  /** Set pointer to the current viewer */
  void setViewer(const FlightGearViewer* v);

  /** Destructor */
  ~FlightGearObject();

  /** Set pointer to the FlightGearViewer */
  void setViewer(FlightGearViewer *viewer);

  /** Connect to a channel entry

      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  virtual void connect(const GlobalId &master_id, const NameSet &cname,
                       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect) override;

  /** Play, update, recalculate, etc. */
  void iterate(TimeTickType ts, const BaseObjectMotion &base, double late,
               bool freeze) override;

  /** Information about the name of this object, might be useful. */
  inline const std::string &getName() const { return name; }

  /** Control visibility (is that possible?) */
  void visible(bool vis);
};

#endif
