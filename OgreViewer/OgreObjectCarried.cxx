/* ------------------------------------------------------------------   */
/*      item            : OgreObjectCarried.cxx
        made by         : Rene' van Paassen
        date            : 180903
	category        : body file 
        description     : 
	changes         : 180903 first version
        language        : C++
        copyright       : (c) 18 TUDelft-AE-C&S
*/

#define OgreObjectCarried_cxx
#include "OgreObjectCarried.hxx"
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


OgreObjectCarried::OgreObjectCarried(const WorldDataSpec& spec) :
  OgreObjectMoving(spec),
  position(0.0),
  attitude(0.0)
{
  if (spec.coordinates.size() >= 3) {
    std::copy(spec.coordinates.begin(), spec.coordinates.begin()+3,
              position.begin());
  }
  if (spec.coordinates.size() >= 6) {
    std::copy(spec.coordinates.begin()+3, spec.coordinates.begin()+6,
              attitude.begin());
  }
}


OgreObjectCarried::~OgreObjectCarried()
{
  
}

void OgreObjectCarried::init(Ogre::SceneManager* manager)
{
  try {
    Ogre::MeshPtr m = Ogre::MeshManager::getSingleton().load
      (mesh_name, groupname);
    entity = manager->createEntity(name.c_str(), mesh_name.c_str());
    node = manager->getSceneNode("__ego_node__")->
      createChildSceneNode(name.c_str());
    node->attachObject(entity);

    // initial, and in case no connection, final position
    node->setPosition(AxisTransform::ogrePosition(position));
    node->setOrientation
      (AxisTransform::ogreQuaternion(attitude[0], attitude[1], attitude[2]));
  }
  catch (const Ogre::Exception& e) {
    std::cerr << "OgreObjectCarried caught " << e.what() << std::endl;
    return;
  }
}

void OgreObjectCarried::iterate(TimeTickType ts,
                                const BaseObjectMotion& base,
                                double late)
{
  // only if there is a token (after a connect) update the relative position
  if (r_motion) {
    this->OgreObjectMoving::iterate(ts, base, late);
  }
}
  
static SubContractor<OgreObjectTypeKey, OgreObjectCarried>
*OgreObjectCarried_maker = new
    SubContractor<OgreObjectTypeKey, OgreObjectCarried>
    ("OgreObjectCarried");
