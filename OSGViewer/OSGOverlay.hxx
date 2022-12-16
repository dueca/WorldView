/* ------------------------------------------------------------------   */
/*      item            : OSGOverlay.hxx
        made by         : Rene van Paassen
        date            : 100127
	category        : header file 
        description     : 
	changes         : 100127 first version
        language        : C++
*/

#ifndef OSGOverlay_hxx
#define OSGOverlay_hxx

#include "OSGObject.hxx"
#include "comm-objects.h"

// https://github.com/openscenegraph/OpenSceneGraph/blob/master/examples/osghud/osghud.cpp

class OSGViewer;


/** Fixed, single window overlay mask */
class OSGOverlay: public OSGObject
{
protected:

  /** Post-render camera */
  osg::ref_ptr<osg::Camera>    orthocam;

  /** Matching window */
  std::string                  window_name;

  /** Matching view */
  std::string                  view_name;
  
public:
  /** Constructor 
      
      @param spec Generic data specification for graphic
                  objects
                  - filename[0]: name of the mask bitmap
                  - 
  */
  OSGOverlay(const WorldDataSpec &spec);
  
  /** Destructor */
  ~OSGOverlay();

  /** Connect to a channel entry 

      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  void connect(const GlobalId& master_id, const NameSet& cname,
                       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect);

  /** Play, update, recalculate, etc. */
  void iterate(TimeTickType ts, const BaseObjectMotion& base, double late); 

  /** Initialise position */
  virtual void init(const osg::ref_ptr<osg::Group>& root,
		    OSGViewer* master) override;
};

#endif
