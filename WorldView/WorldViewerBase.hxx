/* ------------------------------------------------------------------   */
/*      item            : WorldViewerBase.hxx
        made by         : Rene van Paassen
        date            : 100122
	category        : header file
        description     :
	changes         : 100122 first version
        language        : C++
*/

#ifndef WorldViewerBase_hxx
#define WorldViewerBase_hxx

#include <map>
#include <set>
#include <string>
#include <dueca/dueca.h>
#include "WorldObjectBase.hxx"
#include "../../WorldView/SpecificationBase/SpecificationBase.hxx"
#include "comm-objects.h"

/** Map type for keeping track of objects in this world */
typedef pair<WorldObjectBase*, unsigned int> ControlledWithCount;
typedef std::map<std::string,ControlledWithCount> ControllablesMap;

/** Set for keeping track of object classes I cannot make */
typedef std::set<std::string> NotCreatable;
class WorldView;

/** This is a base class for viewer classes based on different scene
    graphing or graphics toolkits.

    The base class is derived from ScriptCreatable, so it is connected
    to dueca scripting, but is not creatable, since it contains pure
    virtual functions.
*/
class WorldViewerBase: public SpecificationBase
{

protected:
  /** Controlled objects, can be found by their name and called with
      arguments */
  ControllablesMap controllables;

  /** Have tried to, but could not create these */
  NotCreatable     uncreatables;

  /** Key type for created objects; channel name and creation id */
  typedef std::pair<std::string, unsigned> creation_key_t;

public: /* Initialisation */

  /** Constructor */
  WorldViewerBase();

  /** Destructor */
  ~WorldViewerBase();

  /** Do work after construction, normally opening the window(s) */
  virtual bool complete() = 0;

public: /* per-cycle interaction, updating viewpoint and drawing */

  /** If needed, provide a pointer to the WorldView object. */
  virtual void setMaster(WorldView* m);

  /** Initialise the windows etc. */
  virtual void init(bool waitswap) = 0;

  /** Do a re-draw
      \param wait   If true, do now swap the buffers. The application
                    must later wait and swap with the waitSwap function. */
  virtual void redraw(bool wait = false, bool reset_context = false) = 0;

  /** Wait for the swap. */
  virtual void waitSwap() = 0;

  /** set the base camera position
      @param tick  DUECA current time tick
      @param base  Object motion, position, etc.
      @param late  Time after DUECA tick */
  virtual void setBase(TimeTickType tick, const BaseObjectMotion& base,
                       double late, bool freeze) = 0;

  /** Set drawing context current; possibly needed when multiple GL windows
      use same thread */
  virtual void makeContextCurrent();

public: /* Feedback of key and cursor events */

  /** Indicate which event types are interesting to you */
  virtual bool setEventMask(const std::string& window, unsigned long mask);

  /** The current event */
  WorldViewerEvent current_event;

  /** Get next event. In case there is no event, the type is None */
  virtual const WorldViewerEvent& getNextEvent(const std::string& window);

public: /* management of objects in the world */

  /** Insert another controlled object */
  void addControllable
  (const dueca::GlobalId& master_id,
   const dueca::NameSet& cname, dueca::entryid_type entry_id,
   uint32_t creation_id, const std::string& data_class,
   const std::string& entry_label, dueca::Channel::EntryTimeAspect time_aspect);

  virtual bool createControllable
  (const dueca::GlobalId& master_id,
   const dueca::NameSet& cname, dueca::entryid_type entry_id,
   uint32_t creation_id, const std::string& data_class,
   const std::string& entry_label,
   dueca::Channel::EntryTimeAspect time_aspect) = 0;

  /** Remove a controllable */
  virtual void removeControllable(const dueca::NameSet& cname,
                                  uint32_t creation_id) = 0;

  /** Change the configuration of the scene graph, returns true if
      successful */
  virtual bool adaptSceneGraph(const WorldViewConfig& adapt);
};

#endif
