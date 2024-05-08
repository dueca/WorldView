/* ------------------------------------------------------------------   */
/*      item            : FlightGearObject.cxx
        made by         : Rene' van Paassen
        date            : 100707
        category        : body file
        description     :
        changes         : 100707 first version
        language        : C++
*/

#include "MultiplayerEncode.hxx"
#define FlightGearObject_cxx
#include "FlightGearObject.hxx"
#include "FlightGearViewer.hxx"
#include <dueca/Ticker.hxx>
#include <dueca/debug.h>

FlightGearObject::FlightGearObject(const std::string &name,
                                   const std::string &fgclass,
                                   const std::string &jsonfile,
                                   FlightGearViewer *master) :
  itime(0.0),
  name(name),
  fgclass(fgclass),
  master(master)
{
  if (jsonfile.size()) {
    coder.reset(new PropertyEncoderJSON(jsonfile));
  }
}

FlightGearObject::~FlightGearObject() {}

void FlightGearObject::visible(bool vis)
{
  // not yet implemented
}

void FlightGearObject::iterate(TimeTickType ts, const BaseObjectMotion &base,
                               double late, bool freeze)
{
  if (r_motion->isValid()) {
    try {
      DataReader<BaseObjectMotion, MatchIntervalStartOrEarlier> r(*r_motion,
                                                                  ts);
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
        master->getEncoder().encode(o2, fgclass, name, ftime, 0.1, coder.get());
        master->sendPositionReport();
      }
    }
    catch (const std::exception &e) {
      W_MOD("Cannot read FlightGearObject data for object " << name << " error "
                                                            << e.what());
    }
  }
}

void FlightGearObject::connect(const GlobalId &master_id, const NameSet &cname,
                               entryid_type entry_id,
                               Channel::EntryTimeAspect time_aspect)
{
  r_motion.reset(new ChannelReadToken(
    master_id, cname, BaseObjectMotion::classname, entry_id, time_aspect,
    Channel::OneOrMoreEntries, Channel::JumpToMatchTime));
}
