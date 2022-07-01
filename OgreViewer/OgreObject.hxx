/* ------------------------------------------------------------------   */
/*      item            : OgreObject.hxx
        made by         : Rene van Paassen
        date            : 090617
	category        : header file 
        description     : 
	changes         : 090617 first version
        language        : C++
*/

#ifndef OgreObject_hxx
#define OgreObject_hxx

#include <Ogre.h>
#include <WorldObjectBase.hxx>
#include <string>

/** Base class for ogre objects that are controlled from the
    simulation. */
class OgreObject: public WorldObjectBase
{
protected:
  /** The ogre thing I am */
  Ogre::Entity *entity;

  /** The node pointing here */
  Ogre::SceneNode  *node;

  /** Pointer to the scenemanager */
  Ogre::SceneManager* manager;
  
  /** Name in the ogre world */
  std::string   mesh_name;

  /** Resource group name for the mesh */
  std::string   groupname;

public:
  /** Constructor */
  OgreObject();

  /** Destructor */
  ~OgreObject();

  /** Initialise the avatar with the OGRE scene */
  virtual void init(Ogre::SceneManager* manager);

  /** Uninit when the ogre model has been thrown out */
  void unInit(Ogre::SceneManager* manager);

  /** Name */
  inline const std::string& getName() { return name; }
  
  /** Control visibility */
  virtual void visible(bool vis);
};

#endif
