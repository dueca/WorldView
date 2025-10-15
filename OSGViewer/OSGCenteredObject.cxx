/* ------------------------------------------------------------------   */
/*      item            : OSGStaticObject.cxx
        made by         : Rene' van Paassen
        date            : 100127
        category        : body file
        description     :
        changes         : 100127 first version
        language        : C++
*/

#define OSGCenteredObject_cxx
#include "OSGCenteredObject.hxx"
#include "RvPQuat.hxx"
#include "OSGObjectFactory.hxx"
#include "AxisTransform.hxx"
#include <cstring>
#include <algorithm>
#include <osg/PositionAttitudeTransform>

OSGCenteredObject::OSGCenteredObject(const WorldDataSpec &specification) :
  OSGStaticObject(specification)
{
  //
}

OSGCenteredObject::~OSGCenteredObject()
{
  //
}

void OSGCenteredObject::connect(const GlobalId &master_id, const NameSet &cname,
                                entryid_type entry_id,
                                Channel::EntryTimeAspect time_aspect)
{
  // no action
}

void OSGCenteredObject::iterate(TimeTickType ts, const BaseObjectMotion &base,
                                double late)
{
  if (base.dt != 0.0 && late != 0.0) {
    BaseObjectMotion o2(base);
    o2.extrapolate(late);
    transform->setPosition(AxisTransform::osgPos(o2.xyz));
    // transform->setAttitude(AxisTransform::osgQuat(o2.attitude_q));
  }
  else {
    transform->setPosition(AxisTransform::osgPos(base.xyz));
    // transform->setAttitude(AxisTransform::osgQuat(base.attitude_q));
  }
}

#if DUECA_VERSION_NUM >= DUECA_VERSION(3, 2, 0)
#define OPT(A) , A
#else
#define OPT(A)
#endif

static SubContractor<OSGObjectTypeKey, OSGCenteredObject>
  *OSGCenteredObject_maker =
    new SubContractor<OSGObjectTypeKey, OSGCenteredObject>(
      "centered" OPT("An object on position of observer (skydome, interior)"));
