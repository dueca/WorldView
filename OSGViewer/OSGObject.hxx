/* ------------------------------------------------------------------   */
/*      item            : OsgObject.hxx
        made by         : Rene van Paassen
        date            : 090617
	category        : header file 
        description     : 
	changes         : 090617 first version
        language        : C++
*/

#ifndef OsgObject_hxx
#define OsgObject_hxx

#include <osg/Geometry>
#include <WorldObjectBase.hxx>
#include <string>

class OSGViewer;

/** Base class for OpenSceneGraph objects that are controlled from the
    simulation. 

    Typically, a derived class has 

    - a constructor with (const WorldDataSpec& x) as argument

    - a connect() function that creates a read token for the data from
      the world channel data

    - an iterate() function that reads the channel data and updates 
      the object
*/
class OSGObject: public WorldObjectBase
{
protected:
  /** The osg thing I am */
  osg::ref_ptr<osg::Node> entity;

  /** Transformation */
  osg::ref_ptr<osg::PositionAttitudeTransform> transform;

  /** File name of the model */
  std::string  modelfile;

  /** Temporary node mask?? */
  unsigned int nodemask;

public:
  /** Constructor */
  OSGObject();

  /** Destructor */
  virtual ~OSGObject();

  /** Initialise the avatar with the OSG scene */
  virtual void init(const osg::ref_ptr<osg::Group>& root, OSGViewer* master);

  /** De-initialise the avatar with the OSG scene */
  virtual void unInit(const osg::ref_ptr<osg::Group>& root);
  
  /** Control visibility */
  virtual void visible(bool vis);

public:

  /** Returns true if the object needs drawing post-access */
  virtual bool requirePostDrawAccess() { return false; }
};

#endif
