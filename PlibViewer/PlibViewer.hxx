/* ------------------------------------------------------------------   */
/*      item            : PlibViewer.hxx
        made by         : Rene van Paassen
        date            : 100126
        category        : header file
        description     :
        changes         : 100126 first version
        language        : C++
*/

#ifndef PlibViewer_hxx
#define PlibViewer_hxx

#include "PlibStaticObject.hxx"
#include "comm-objects.h"
#include <WorldViewerBase.hxx>
#include <boost/shared_ptr.hpp>
#include <list>
#include <plib/ssg.h>
#include <string>

/** Hidden object for X implementation, to prevent all kinds of
    #define trouble */
struct PlibViewerImp;
struct PlibViewer_WindowSet;
struct _XDisplay;
typedef struct _XDisplay Display;

/** This is a class for creating an visualisation with Plib. The
    visualisation optionally uses multiple X gl windows, and within
    each window multiple viewports are possible.

    The class is started by defining windows, viewports and objects in
    the world (addWindow, addViewport, addObject). Then
    complete must be called to complete the definition.

    After that, init is called. This creates windows and initialises
    ssg/plib

    For drawing, there are two options;
    <ol>
    <li> Call redraw with wait = false, does a redraw and a swap,
    un-synchronized.
    <li> Call redraw with wait = true, and after this call waitSwap,
    which waits for the swap and then returns.
    </ol>

    The child PlibViewer_Dueca turns this into a DUECA helper
    class. You might look there for how to call this from a dueca.mod
    script.
 */
class PlibViewer : public WorldViewerBase
{

  /** Counter for the post call drawing, doubled with screen */
  unsigned long post_counter;

  /** current camera position */
  double position[3];

  /** current camera orientation */
  double orientation[4];

  /** Root of the scene */
  ssgRoot *scene;

  /** X related stuff */
  boost::scoped_ptr<PlibViewerImp> imp;

protected:
  /** Path to the models */
  std::string modelpath;

  /** Path to the textures */
  std::string texturepath;

  /** Window specification, modified by derived class */
  std::list<WinSpec> winspec;

  /** Wait divisor */
  int glx_sync_divisor;

  /** Wait offset */
  int glx_sync_offset;

  /** keep the normal cursor */
  bool keep_pointer;

  /** background/clear color */
  std::vector<double> bg_color;

  friend struct PlibViewer_WindowSet;

private:
  /** Organisation of a single camera/view/window */
  struct ViewSet
  {
    /** Camera context */
    ssgContext *camera;

    /** Viewport definition */
    std::vector<int> viewport;

    /** Rotation offset */
    double angle_offset[4];

    /** Position offset */
    double rel_pos[3];

    /** Constructor */
    ViewSet();

    /** Initialise */
    void init(const ViewSpec &vs, const PlibViewer_WindowSet &ws,
              ssgRoot *scene, int zorder);
  };

  /** Viewport specification list, keeps definitions of views */
  std::list<ViewSpec> viewspec;

  /** Definition of a map with all windows */
  typedef std::map<std::string, PlibViewer_WindowSet> WindowsMap;

  /** All opened windows */
  WindowsMap windows;

  /** Helper */
  PlibViewer_WindowSet myCreateWindow(const WinSpec &ws);

  /** Keep a count of opened displays; re-use the pointer if a new
      window gets on an already opened display */
  std::map<std::string, Display *> opened_displays;

public:
  /** Constructor */
  PlibViewer();

  /** Destructor */
  ~PlibViewer();

  /** Check for preparedness / completenes of specification */
  virtual bool complete();

  /** Initialise the windows etc.  */
  void init(bool waitswap);

  /** Create a controllable object. Object creation depends on class of
      data supplied, further init may rely on first data entering. */
  bool createControllable(const GlobalId &master_id, const NameSet &cname,
                          entryid_type entry_id, uint32_t creation_id,
                          const std::string &data_class,
                          const std::string &entry_label,
                          Channel::EntryTimeAspect time_aspect);

  /** Remove a controllable */
  void removeControllable(const dueca::NameSet &cname, uint32_t creation_id);

  /** set the base camera position
      @param tick  DUECA current time tick
      @param base  Object motion, position, etc.
      @param late  Time after DUECA tick
      @param freeze Unused here */
  void setBase(TimeTickType tick, const BaseObjectMotion &base, double late,
               bool freeze) final;

  /** Add a viewport */
  void addViewport(const ViewSpec &vp) { viewspec.push_back(vp); }

  /** Add a window */
  void addWindow(const WinSpec &window) { winspec.push_back(window); }

  /** Add an object */
  void addObject(const WorldDataSpec &obj);

  /** Add an object, but now a ready-made one */
  void addObject(PlibObject *obj);

  /** Do a re-draw
      \param wait   If true, do now swap the buffers. The application
                    must later wait and swap with the waitSwap function. */
  void redraw(bool wait = false, bool reset_context = false);

  /** Wait for the swap. */
  void waitSwap();

  /** set the event mask, as per Xlib programming rules
      \param window   Name of the window
      \param mask     maks of events, see Xlib */
  bool setEventMask(const std::string &window, unsigned long mask);

  /** Get next event. In case there is no event, the type is None */
  const WorldViewerEvent &getNextEvent(const std::string &window);

  /** Create a static (not controlled) controllable object. */
  bool createStatic(const std::vector<std::string> &classname);

protected:
  /** Map with created (due to presence in the world channel)
      objects. They are indexed with channel entry index, and removed
      from the map when the entry is removed from the channel. */
  typedef std::map<creation_key_t, boost::intrusive_ptr<PlibObject>>
    created_objects_t;

  /** Objects creates automatically */
  created_objects_t active_objects;

  /** List with static  objects. */
  typedef std::list<boost::intrusive_ptr<PlibObject>> static_objects_t;

  /** Objects creates automatically */
  static_objects_t static_objects;

  /** Objects that need post-draw access */
  static_objects_t post_draw;

private:
  /** Called whenever the window needs redrawing. This has the GL code
      to redraw the display. */
  void display();

  /** Try to create an object based on class name alone. */
  WorldObjectBase *createOnClass(const std::string &name,
                                 const std::string &klass);

  /** for calling raw GL code etc. */
  void drawPostCall();
};

#endif
