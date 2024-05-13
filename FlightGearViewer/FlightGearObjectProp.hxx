/* ------------------------------------------------------------------   */
/*      item            : FlightGearObjectProp.hxx
        made by         : Rene van Paassen
        date            : 240513
        category        : header file
        description     :
        changes         : 240513 first version
        language        : C++
*/

#ifndef FlightGearObjectProp_hxx
#define FlightGearObjectProp_hxx

#include "comm-objects.h"
#include "FlightGearObject.hxx"
#include <boost/scoped_ptr.hpp>
#include <dueca.h>
#include <dueca_ns.h>


class FlightGearViewer;

/** Memory for "other" players to be transmitted to a FlightGear
    server */
class FlightGearObjectProp : public FlightGearObject
{

public:
  /** Constructor */
  FlightGearObjectProp(const WorldDataSpec& spec);

  /** Destructor */
  ~FlightGearObjectProp();

  /** Connect to a channel entry

      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  virtual void connect(const GlobalId &master_id, const NameSet &cname,
                       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect) override;


  /** Play, update, recalculate, etc. */
  virtual void iterate(TimeTickType ts, const BaseObjectMotion &base,
		       double late, bool freeze) override;


};

#endif
