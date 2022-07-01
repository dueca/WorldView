/* ------------------------------------------------------------------   */
/*      item            : OSGStaticObject.cxx
        made by         : Rene' van Paassen
        date            : 100127
	category        : body file 
        description     : 
	changes         : 100127 first version
        language        : C++
*/

#define OSGStaticObject_cxx
#include "OSGStaticObject.hxx"
#include "RvPQuat.hxx"
#include "OSGObjectFactory.hxx"
#include "AxisTransform.hxx"
#include <osg/PositionAttitudeTransform>
#include <cstring>
#include <algorithm>
#include <cmath>

static const double deg2rad = M_PI / 180.0;

OSGStaticObject::OSGStaticObject(const WorldDataSpec &specification) :
  OSGObject()
{
  memset(position, 0, sizeof(position));
  memset(orientation, 0, sizeof(orientation));
  for (int ii = 3; ii--; ) scale[ii] = 1.0;
  orientation[0] = 1.0;
  if (specification.coordinates.size() >= 3) {
    for (int ii = 3; ii--; ) {
      position[ii] = specification.coordinates[ii];
    }
  }
  if (specification.coordinates.size() >= 6) {
    phithtpsi2Q(orientation, 
		specification.coordinates[3]*deg2rad, 
		specification.coordinates[4]*deg2rad, 
		specification.coordinates[5]*deg2rad);
  }
  if (specification.coordinates.size() >= 9) {
    std::copy(&specification.coordinates[6], &specification.coordinates[9], 
	      scale);
  }
  if (specification.filename.size() > 0) modelfile = specification.filename[0];
  this->name = specification.name;
}

OSGStaticObject::~OSGStaticObject()
{

}

void OSGStaticObject::init(const osg::ref_ptr<osg::Group>& root,
                           OSGViewer* master)
{
  // do default init (model loading etc.)
  OSGObject::init(root, master);
  entity->setDataVariance(osg::Object::STATIC);

  // set with the default position
  transform->setPosition(AxisTransform::osgPos(position));
  transform->setAttitude(AxisTransform::osgQuat(orientation));
}

void OSGStaticObject::connect(const GlobalId& master_id, const NameSet& cname,
                       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect)
{
  // todo
}

void OSGStaticObject::iterate(TimeTickType ts,
                       const BaseObjectMotion& base, double late)
{
  // todo
}

#if DUECA_VERSION_NUM >= DUECA_VERSION(3,2,0)
#define OPT(A) , A
#else
#define OPT(A)
#endif

static SubContractor<OSGObjectTypeKey, OSGStaticObject> 
*OSGStaticObject_maker = 
  new SubContractor<OSGObjectTypeKey, OSGStaticObject>
  ("static" OPT("Static object, defined by a single model"));
