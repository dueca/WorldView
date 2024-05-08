/* ------------------------------------------------------------------   */
/*      item            : OSGObjectCompatible.cxx
        made by         : Rene' van Paassen
        date            : 180903
	category        : body file
        description     :
	changes         : 180903 first version
        language        : C++
        copyright       : (c) 18 TUDelft-AE-C&S
*/

#define OSGObjectCompatible_cxx
#include "OSGObjectCompatible.hxx"
#include "AxisTransform.hxx"
#include <osg/PositionAttitudeTransform>
#include <exception>
#include <dueca/debug.h>
#include <dueca/Ticker.hxx>
#include "OSGObjectFactory.hxx"

#define DO_INSTANTIATE
#include <dueca/dueca.h>


// static map object
std::map<std::string,std::string>
OSGObjectCompatible::typemap;

OSGObjectCompatible::OSGObjectCompatible(const WorldDataSpec& spec) :
  OSGObjectMoving(spec),
  root(),
  master(NULL)
{
  // further initialisation when first data comes in
}


OSGObjectCompatible::~OSGObjectCompatible()
{
  //
}

void OSGObjectCompatible::connect(const GlobalId& master_id,
                                  const NameSet& cname,
                                  entryid_type entry_id,
                                  Channel::EntryTimeAspect time_aspect)
{
  r_motion.reset(new ChannelReadToken
                 (master_id, cname, ObjectMotion::classname, entry_id,
                  time_aspect, Channel::OneOrMoreEntries,
                  Channel::JumpToMatchTime));
}

void OSGObjectCompatible::init(const osg::ref_ptr<osg::Group>& root,
                               OSGViewer* master)
{
  this->root = root;
  this->master = master;
}

void OSGObjectCompatible::iterate(TimeTickType ts,
                                  const BaseObjectMotion& base,
                                  double late, bool freeze)
{
  if (r_motion->isValid()) {
    try {
      DataReader<ObjectMotion,MatchIntervalStartOrEarlier>
        r(*r_motion, ts);

      if (entity == NULL) {
        try {

          this->name = r.data().name.c_str();
          this->modelfile = typemap[std::string(r.data().klass.c_str())];

          // call init?
          this->OSGObject::init(root, master);
        }
        catch (const std::exception& e) {
          W_MOD("Cannot initialize OSGObjectCompatible");
        }
      }
      if (entity != NULL) {
        if (r.data().dt != 0.0) {
          ObjectMotion o2(r.data());
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
  ("compatible" OPT("Old-style moving object, for compatibility"));
