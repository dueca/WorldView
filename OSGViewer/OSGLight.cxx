/* ------------------------------------------------------------------   */
/*      item            : OSGLight.cxx
        made by         : Rene' van Paassen
        date            : 150318
        category        : body file
        description     :
        changes         : 150318 first version
        language        : C++
*/

#define OSGLight_cxx
#include "OSGLight.hxx"
#include "OSGObjectFactory.hxx"
#include "AxisTransform.hxx"

#include <cmath>
#include <osg/LightModel>
#include <osg/LightSource>
#include <osg/PositionAttitudeTransform>
#include <osgDB/ReadFile>
#include <iostream>

#define DEB(A) std::cerr << A << std::endl;

// GL light counter
static unsigned lightno = 0;

template<class IT>
inline void vset(osg::Vec3d& t, IT& i, const IT& end)
{
  IT i2 = i;
  for (int ix = 3; ix--; ) {
    if (++i == end) return;
  }

  t.set(*i2, *(i2+1), *(i2+2));
}

template<class IT>
inline void vset(osg::Vec4d& t, IT& i, const IT& end)
{
  IT i2 = i;
  for (int ix = 4; ix--; ) {
    if (++i == end) return;
  }

  { t.set(*i2, *(i2+1), *(i2+2), *(i2+3)); }
}


template<class IT>
OSGLightBase::OSGLightBase(IT begin, const IT& end) :
  ambient(0, 0, 0, 1),
  diffuse(0, 0, 0, 1),
  specular(0, 0, 0, 1),
  position(0, 0, 0, 1),
  direction(0, 0, 0),
  spot_exponent(2),
  spot_cutoff(10),
  constant_att(0),
  linear_att(0),
  quadratic_att(0)
{
  vset(ambient, begin, end);  // 0, 1, 2, 3
  vset(diffuse, begin, end);  // 4
  vset(specular, begin, end);
  vset(position, begin, end);
  vset(direction, begin, end);
  if (begin != end)
    constant_att = *(begin++);
  if (begin != end)
    linear_att = *(begin++);
  if (begin != end)
    quadratic_att = *(begin++);
  if (begin != end)
    spot_exponent = *(begin++);
  if (begin != end)
    spot_cutoff = *(begin++);
}



void OSGLightBase::init(const osg::ref_ptr<osg::Group>& root,
                        osg::ref_ptr<osg::Group>& parent)
{
  const double EPS = 1e-10;

  osg::ref_ptr<osg::Light> l = new osg::Light;
  osg::StateSet *rset = root->getOrCreateStateSet();
  rset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
  rset->setMode(GL_LIGHT0, osg::StateAttribute::ON);
  root->setStateSet(rset);

  // light number
  DEB("init light " << lightno);
  l->setLightNum(lightno++);

  // set the light values
  l->setAmbient(ambient);
  l->setDiffuse(diffuse);
  l->setSpecular(specular);

  // position, or directional light, depending on 4th parameter;
  l->setPosition(position);

  // if it not a directional light, it might be a spot
  if (fabs(position[3]) > EPS && direction.length() > EPS) {

    // spot light
    l->setDirection(direction);
    l->setSpotExponent(spot_exponent);
    l->setSpotCutoff(spot_cutoff);
  }

  // attenuation parameters
  l->setConstantAttenuation(constant_att);
  l->setLinearAttenuation(linear_att );
  l->setQuadraticAttenuation(quadratic_att);

  osg::ref_ptr<osg::LightSource> source = new osg::LightSource;
  source->setLight(l.get()); // .get() ??
  source->setLocalStateSetModes(osg::StateAttribute::ON);
  source->setStateSetModes(*(root->getOrCreateStateSet()),
                           osg::StateAttribute::ON);
  parent->addChild(source);
}

OSGLight::OSGLight(const WorldDataSpec& sp) :
  base(&(sp.coordinates[0]), &(sp.coordinates.back())+1)
{
  //
}

OSGLight::~OSGLight()
{
  //
}

void OSGLight::init(const osg::ref_ptr<osg::Group>& root, OSGViewer* master)
{
  // base location of the object
  transform = new osg::PositionAttitudeTransform();
  osg::ref_ptr<osg::Group> group = new osg::Group;

  // maybe add the lamp?
  if (modelfile.size()) {
    entity = osgDB::readNodeFile(modelfile, NULL);
    if (!entity.valid()) {
      cerr << "Failed to read " << modelfile << endl;
      return;
    }
    entity->setDataVariance(osg::Object::DYNAMIC);
    group->addChild(entity);
  }

  // now add the light itself
  base.init(root, group);
  transform->addChild(group);
  transform->setName(this->getName());
  root->addChild(transform);
  transform->setNodeMask(nodemask);
  //  visible(true);
}

void OSGLight::connect(const GlobalId& master_id, const NameSet& cname,
                       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect)
{
  // todo
}

void OSGLight::iterate(TimeTickType ts,
                       const BaseObjectMotion& base, double late)
{
  // todo
}

OSGCenteredLight::OSGCenteredLight(const WorldDataSpec& sp) :
  OSGLight(sp)
{
  //
}

OSGCenteredLight::~OSGCenteredLight()
{
  //
}
void OSGCenteredLight::connect(const GlobalId& master_id, const NameSet& cname,
                               entryid_type entry_id,
                               Channel::EntryTimeAspect time_aspect)
{
  // no connection needed
}

void OSGCenteredLight::iterate(TimeTickType ts,
                                const BaseObjectMotion& base,
                                double late)
{
  if (base.dt != 0.0 && late != 0.0) {
    BaseObjectMotion o2(base); o2.extrapolate(late);
    transform->setPosition(AxisTransform::osgPos(o2.xyz));
    transform->setAttitude(AxisTransform::osgQuat(o2.attitude_q));
  }
  else {
    transform->setPosition(AxisTransform::osgPos(base.xyz));
    transform->setAttitude(AxisTransform::osgQuat(base.attitude_q));
  }
}

void OSGCenteredLight::init(osg::ref_ptr<osg::Group>& root)
{
  //
}

OSGStaticLight::OSGStaticLight(const WorldDataSpec& sp) :
  OSGLight(sp)
{
  //
}

OSGStaticLight::~OSGStaticLight()
{
  //
}

void OSGStaticLight::connect(const GlobalId& master_id, const NameSet& cname,
                             entryid_type entry_id,
                             Channel::EntryTimeAspect time_aspect)
{
  // no action
}

void OSGStaticLight::iterate(TimeTickType ts, const BaseObjectMotion& base,
                             double late)
{
  // no action
}

#if DUECA_VERSION_NUM >= DUECA_VERSION(3,2,0)
#define OPT(A) , A
#else
#define OPT(A)
#endif

static SubContractor<OSGObjectTypeKey, OSGLight>
*OSGLight_maker = new SubContractor<OSGObjectTypeKey, OSGLight>
  ("light" OPT("A light that can be moved"));

static SubContractor<OSGObjectTypeKey, OSGStaticLight>
*OSGStaticLight_maker = new
  SubContractor<OSGObjectTypeKey, OSGStaticLight>
  ("static-light" OPT("Stationary light, fixed location"));

static SubContractor<OSGObjectTypeKey, OSGCenteredLight>
*OSGCenteredLight_maker = new
  SubContractor<OSGObjectTypeKey, OSGCenteredLight>
  ("centered-light" OPT("Light carried with the observer"));
