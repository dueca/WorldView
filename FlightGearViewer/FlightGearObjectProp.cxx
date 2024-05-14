/* ------------------------------------------------------------------   */
/*      item            : FlightGearObjectProp.cxx
        made by         : Rene' van Paassen
        date            : 100707
        category        : body file
        description     :
        changes         : 100707 first version
        language        : C++
*/

#include "MultiplayerEncode.hxx"
#define FlightGearObjectProp_cxx
#include "FGObjectFactory.hxx"
#include "FlightGearObjectProp.hxx"
#include "FlightGearViewer.hxx"
#include <dueca/Ticker.hxx>
#include <dueca/debug.h>

class PropertyEncoderDCO : public MultiplayerEncode::PropertyEncoderBase
{
  const FGObjectMotion &obj;
  const PropertyEncoderBase &json;

public:
  PropertyEncoderDCO(const FGObjectMotion &obj,
                     const PropertyEncoderBase &json) :
    obj(obj),
    json(json)
  {}
  virtual ~PropertyEncoderDCO() {}

  virtual size_t operator()(XDR &xdr_data) const final
  {
    return obj.encodeProperties(xdr_data) + json(xdr_data);
  }
};

FlightGearObjectProp::FlightGearObjectProp(const WorldDataSpec &spec) :
  FlightGearObject(spec)
{
  //
}

FlightGearObjectProp::~FlightGearObjectProp() {}

void FlightGearObjectProp::iterate(TimeTickType ts,
                                   const BaseObjectMotion &base, double late,
                                   bool freeze)
{
  if (r_motion->isValid()) {
    try {
      DataReader<FGObjectMotion, MatchIntervalStartOrEarlier> r(*r_motion, ts);
      TimeTickType time = r.timeSpec().getValidityStart();

      // only send if new or extrapolated data
      if (time != itime || r.data().dt != 0.0) {
        itime = time;

        BaseObjectMotion o2(r.data());
        if (freeze) {
          o2.omega = 0.0;
          o2.uvw = 0.0;
        }
        else if (r.data().dt != 0.0) {
          double textra =
            DataTimeSpec(r.timeSpec().getValidityStart(), ts).getDtInSeconds() +
            late;
          if (textra > 0.0) {
            o2.extrapolate(textra);
          }
        }

        // get the flight time for FlightGear, seconds since UTC0
        double ftime =
          master->getFlightTime(DataTimeSpec(0, time).getDtInSeconds());

        // encodes fixed properties and the standard set in the DCO object
        PropertyEncoderDCO mycoder(r.data(), *coder);

        // convert to FG protocol
        master->getEncoder().encode(o2, fgclass, name, ftime, 0.1, &mycoder);

        // send the result
        master->sendPositionReport();
      }
    }
    catch (const std::exception &e) {
      W_MOD("Cannot read FlightGearObjectProp data for object "
            << name << " error " << e.what());
    }
  }
}

void FlightGearObjectProp::connect(const GlobalId &master_id,
                                   const NameSet &cname, entryid_type entry_id,
                                   Channel::EntryTimeAspect time_aspect)
{
  r_motion.reset(new ChannelReadToken(
    master_id, cname, getclassname<FGObjectMotion>(), entry_id, time_aspect,
    Channel::OneOrMoreEntries, Channel::JumpToMatchTime));
}

static auto FlightGearObjectProp_maker =
  new SubContractor<FGObjectTypeKey, FlightGearObjectProp>(
    "fgprops", "Moving FlighGear object, position controlled, property added");
