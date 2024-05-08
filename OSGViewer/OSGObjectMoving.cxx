/* ------------------------------------------------------------------   */
/*      item            : OSGObjectMoving.cxx
        made by         : Rene' van Paassen
        date            : 180903
	category        : body file
        description     :
	changes         : 180903 first version
        language        : C++
        copyright       : (c) 18 TUDelft-AE-C&S
*/

#define OSGObjectMoving_cxx
#include "OSGObjectMoving.hxx"
#include "AxisTransform.hxx"
#include <exception>
#include <dueca/debug.h>
#include <dueca/Ticker.hxx>
#include "OSGObjectFactory.hxx"
#include <osg/PositionAttitudeTransform>

struct OSGObjectConfigError: public std::exception
{
  /** Say what is the problem */
  const char* what() { return "Incorrect configuration object"; }
};


OSGObjectMoving::OSGObjectMoving(const WorldDataSpec& spec) :
  OSGObject(),
  r_motion()
{
  this->modelfile = spec.filename[0];
  this->name = spec.name;
}


OSGObjectMoving::~OSGObjectMoving()
{

}

void OSGObjectMoving::connect(const GlobalId& master_id, const NameSet& cname,
                              entryid_type entry_id,
                              Channel::EntryTimeAspect time_aspect)
{
  r_motion.reset(new ChannelReadToken
                 (master_id, cname, BaseObjectMotion::classname, entry_id,
                  time_aspect, Channel::OneOrMoreEntries,
                  Channel::JumpToMatchTime));
}

void OSGObjectMoving::iterate(TimeTickType ts,
                              const BaseObjectMotion& base,
                              double late, bool freeze)
{
  if (r_motion->isValid() && entity != NULL) {
    try {
      DataReader<BaseObjectMotion,MatchIntervalStartOrEarlier>
        r(*r_motion, ts);
      if (r.data().dt != 0.0) {
        BaseObjectMotion o2(r.data());
        double textra = DataTimeSpec
          (r.timeSpec().getValidityStart(), ts).getDtInSeconds() + late;
        if (textra > 0.0) {
          o2.extrapolate(textra);
        }
        transform->setPosition(AxisTransform::osgPos(o2.xyz));
        transform->setAttitude(AxisTransform::osgQuat(o2.attitude_q));
      }
      else {
        transform->setPosition(AxisTransform::osgPos(r.data().xyz));
        transform->setAttitude(AxisTransform::osgQuat(r.data().attitude_q));
      }
    }
    catch (const std::exception& e) {
      W_MOD("Cannot read OSGObjectCompatible data for object " <<
            name << " error " << e.what());
    }
  }
}

#if DUECA_VERSION_NUM >= DUECA_VERSION(3,2,0)
#define OPT(A) , A
#else
#define OPT(A)
#endif

static auto OSGObjectMoving_maker = new
  SubContractor<OSGObjectTypeKey,OSGObjectMoving>
  ("moving" OPT("Moving object, defined by single model, controlled"));
