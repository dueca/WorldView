/* ------------------------------------------------------------------   */
/*      item            : OgreObjectMoving.cxx
        made by         : Rene' van Paassen
        date            : 180903
	category        : body file
        description     :
	changes         : 180903 first version
        language        : C++
        copyright       : (c) 18 TUDelft-AE-C&S
*/

#define OgreObjectMoving_cxx
#include "OgreObjectMoving.hxx"
#include "AxisTransform.hxx"
#include <exception>
#include <dueca/debug.h>
#include <dueca/Ticker.hxx>
#include "OgreObjectFactory.hxx"

struct OgreObjectConfigError: public std::exception
{
  /** Say what is the problem */
  const char* what() { return "Incorrect configuration object"; }
};


OgreObjectMoving::OgreObjectMoving(const WorldDataSpec& spec) :
  OgreObject(),
  r_motion()
{
  if (spec.filename.size()) {
    size_t sidx = spec.filename[0].find('/');
    if (sidx != std::string::npos) {
      this->groupname = spec.filename[0].substr(0, sidx);
      this->mesh_name = spec.filename[0].substr(sidx+1);
    }
    else {
      this->groupname = "General";
      this->mesh_name = spec.filename[0];
    }
  }
  this->name = spec.name;
}


OgreObjectMoving::~OgreObjectMoving()
{

}

void OgreObjectMoving::connect(const GlobalId& master_id, const NameSet& cname,
                               entryid_type entry_id,
                               Channel::EntryTimeAspect time_aspect)
{
  r_motion.reset(new ChannelReadToken
                 (master_id, cname, BaseObjectMotion::classname, entry_id,
                  time_aspect, Channel::OneOrMoreEntries,
                  Channel::JumpToMatchTime));
}

void OgreObjectMoving::iterate(TimeTickType ts,
                               const BaseObjectMotion& base,
                               double late, bool freeze)
{
  if (r_motion->isValid() && node != NULL) {
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
        node->setPosition(AxisTransform::ogrePosition(o2.xyz));
        node->setOrientation(AxisTransform::ogreQuaternion(o2.attitude_q));
      }
      else {
        node->setPosition
          (AxisTransform::ogrePosition(r.data().xyz));
        node->setOrientation
          (AxisTransform::ogreQuaternion(r.data().attitude_q));
      }
    }
    catch (const std::exception& e) {
      W_MOD("Cannot read BaseObjectMotion data for object " <<
            name << " error " << e.what());
    }
  }
}

static SubContractor<OgreObjectTypeKey, OgreObjectMoving>
*OgreObjectMoving_maker = new
    SubContractor<OgreObjectTypeKey, OgreObjectMoving>
    ("OgreObjectMoving");
