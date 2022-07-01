/* ------------------------------------------------------------------   */
/*      item            : OgreObject.cxx
        made by         : Rene' van Paassen
        date            : 090617
	category        : body file 
        description     : 
	changes         : 090617 first version
        language        : C++
*/

#define OgreObject_cxx
#include "OgreObject.hxx"
#include "AxisTransform.hxx"
#include <iostream>

#define DEB(A) std::cerr << A << std::endl;
#ifndef DEB
#define DEB(A)
#endif

using namespace Ogre;

OgreObject::OgreObject() :
  entity(NULL),
  node(NULL),
  manager(NULL),
  mesh_name(),
  groupname("General")
{
  // no further initialisation
}


OgreObject::~OgreObject()
{
  
}

void OgreObject::init(Ogre::SceneManager* manager)
{
  try {
    MeshPtr m = Ogre::MeshManager::getSingleton().load(mesh_name, groupname);
    DEB("getting mesh " << groupname << "/" << mesh_name << " ptr=" <<
        reinterpret_cast<void*>(m.get()));
    entity = manager->createEntity(name.c_str(), mesh_name.c_str());
    
    node = manager->getRootSceneNode()->createChildSceneNode(name.c_str());
    node->attachObject(entity);
  }
  catch (const Ogre::Exception& e) {
    std::cerr << "OgreObject caught " << e.what() << std::endl;
    return;
  }
}

void OgreObject::unInit(Ogre::SceneManager* manager)
{
  if (node) {
    node->removeAndDestroyAllChildren();
    manager->destroySceneNode(node);
  }
  if (entity) { manager->destroyEntity(entity); }
  node = NULL; entity = NULL;
}

void OgreObject::visible(bool vis)
{
  if (entity) entity->setVisible(vis);
}
