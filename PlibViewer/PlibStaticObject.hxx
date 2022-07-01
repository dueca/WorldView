/* ------------------------------------------------------------------   */
/*      item            : PlibStaticObject.hxx
        made by         : Rene van Paassen
        date            : 100127
	category        : header file 
        description     : 
	changes         : 100127 first version
        language        : C++
*/

#ifndef PlibStaticObject_hxx
#define PlibStaticObject_hxx

#include "PlibObject.hxx"
#include "comm-objects.h"

/** Class for objects that stay stationary in the world */
class PlibStaticObject: public PlibObject
{
protected:
  /** position of the object */
  double position[3];

  /** orientation of the object */
  double orientation[4];

  /** scale of the object */
  double scale[3];

public:
  /** Constructor */
  PlibStaticObject(const WorldDataSpec &specification);
  
  /** Destructor */
  ~PlibStaticObject();

  /** Initialise position */
  virtual void init(ssgRoot* scene);
};

/** Class of objects that (in one or more coordinates) may stay
    centered on the observer */
class PlibCenteredObject: public PlibStaticObject
{
public:
  /** Constructor */
  PlibCenteredObject(const WorldDataSpec &specification);
  
  /** Destructor */
  ~PlibCenteredObject();

  /** Play, update, recalculate, etc. */
  virtual void iterate(TimeTickType ts,
                       const BaseObjectMotion& base, double late);

  /** Returns true if the object needs information on the observer
      position */
  bool requireInfoObserver() { return true; }
};

/** Objects that in one or more coordinates stay centered wrt the
    observer, with steps of a certain tile size. */
class PlibTiledObject: public PlibStaticObject
{
public:
  /** Constructor */
  PlibTiledObject(const WorldDataSpec &specification);
  
  /** Destructor */
  ~PlibTiledObject();
  
  /** Play, update, recalculate, etc. */
  virtual void iterate(TimeTickType ts,
                       const BaseObjectMotion& base, double late);

  /** Returns true if the object needs information on the observer
      position */
  bool requireInfoObserver() { return true; }
};

/** Ground object. Drawing does not use Z buffer, to prevent Z
    fighting with objects drawn over it */
class PlibGround: public PlibStaticObject
{
public:
  /** Constructor */
  PlibGround(const WorldDataSpec &specification);
  
  /** Destructor */
  ~PlibGround();
  
  /** Initialise position */
  virtual void init(ssgRoot* scene);
};

#endif
