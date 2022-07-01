/* ------------------------------------------------------------------   */
/*      item            : OgreObjectCarried.hxx
        made by         : Rene van Paassen
        date            : 180903
	category        : header file 
        description     : 
	changes         : 180903 first version
        language        : C++
        copyright       : (c) 2018 TUDelft-AE-C&S
*/

#ifndef OgreObjectCarried_hxx
#define OgreObjectCarried_hxx

#include "OgreObjectMoving.hxx"
#include <dueca/fixvector.hxx>

/** Ogre objects controlled with the "BaseObjectMotion" dco object. 

    The motion is relative to the observer's viewpoint, not relative
    to the world. Think car body & interior, etc. 
 */   
class OgreObjectCarried: public OgreObjectMoving
{
  // initial position
  dueca::fixvector<3, double> position;

  // initial attitude
  dueca::fixvector<3, double> attitude;
  
public:

  /** Constructor 

      @param spec   Object data, defines the name of the object
  */  
  OgreObjectCarried(const WorldDataSpec& spec);

  /** Destructor */
  ~OgreObjectCarried();

  /** Initialise the avatar with the OGRE scene, init attaches to eye
      rather than world root */
  virtual void init(Ogre::SceneManager* manager);

  /** Play, update, recalculate, etc.
      A carried object can do without access to channel data, in that
      case, this iterate will govern, and its position/orientation are
      not updated.
   */
  virtual void iterate(TimeTickType ts,
                       const BaseObjectMotion& base, double late);
};

#endif
