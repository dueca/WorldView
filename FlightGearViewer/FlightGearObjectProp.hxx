/* ------------------------------------------------------------------   */
/*      item            : FlightGearObjectProp.hxx
        made by         : Rene van Paassen
        date            : 240513
        category        : header file
        description     : Flightgear object with encoded properties
        changes         : 240513 first version
        language        : C++
*/

#ifndef FlightGearObjectProp_hxx
#define FlightGearObjectProp_hxx

#include "FlightGearObject.hxx"
#include <boost/scoped_ptr.hpp>
#include <dueca.h>
#include <dueca_ns.h>

class FlightGearViewer;

/** FlighGear transmittable object with a means to add additional properties in
    the data message.

    There are two options to use this:

    - if fitting, create a DCO object that derives from FGObjectMotion, and
      override the virtual call to
      ~~~~{.cxx}
        virtual size_t encodeProperties(XDR &xdr_data) const;
      ~~~~

    - otherwise, create a DCO object that directly derives from
      BaseObjectMotion, add the `encodeProperties` call, and create a new
      "subsidiary" for the factory:

      ~~~~{.cxx}
      #include "FlightGearObjectProp.ixx"
      #include "comm-objects.h"

      static auto FlightGearObjectMine_maker =
        new SubContractor<FGObjectTypeKey, FlightGearObjectProp<MyDCOObject>>(
        "myfgprops", "Specialty FlighGear object, position controlled,
          properties added");
      ~~~~
*/
template <class FGDCO> class FlightGearObjectProp : public FlightGearObject
{

public:
  /** Constructor */
  FlightGearObjectProp(const WorldDataSpec &spec);

  /** Destructor */
  ~FlightGearObjectProp();

  /** Connect to a channel entry

      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  void connect(const GlobalId &master_id, const NameSet &cname,
               entryid_type entry_id,
               Channel::EntryTimeAspect time_aspect) override;

  /** Play, update, recalculate, etc. */
  void iterate(TimeTickType ts, const BaseObjectMotion &base, double late,
               bool freeze) override;
};

#endif
