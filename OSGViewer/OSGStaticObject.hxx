/* ------------------------------------------------------------------   */
/*      item            : OSGStaticObject.hxx
        made by         : Rene van Paassen
        date            : 100127
	category        : header file 
        description     : 
	changes         : 100127 first version
        language        : C++
*/

#ifndef OSGStaticObject_hxx
#define OSGStaticObject_hxx

#include "OSGObject.hxx"
#include "comm-objects.h"

/** Class for objects that stay stationary in the world */
class OSGStaticObject: public OSGObject
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
  OSGStaticObject(const WorldDataSpec &specification);
  
  /** Destructor */
  ~OSGStaticObject();

  /** Initialise position */
  virtual void init(const osg::ref_ptr<osg::Group>& root, OSGViewer* master);
  
  /** Connect to a channel entry 

      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  void connect(const GlobalId& master_id, const NameSet& cname,
                       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect);

  /** Play, update, recalculate, etc. */
  void iterate(TimeTickType ts, const BaseObjectMotion& base, double late); 
};

#endif
