/* ------------------------------------------------------------------   */
/*      item            : OgreViewer.hxx
        made by         : Rene van Paassen
        date            : 090616
        category        : header file
        description     :
        changes         : 090616 first version
        language        : C++
*/

#ifndef OgreViewer_hxx
#define OgreViewer_hxx

#include <Ogre.h>
#if OGRE_VERSION >= 0x010900
#include <Overlay/OgreOverlay.h>
#include <Overlay/OgreOverlayManager.h>
#include <Overlay/OgreOverlaySystem.h>
#endif

#include "DotSceneLoader.h"
#include "OgreObject.hxx"
#include <WorldViewerBase.hxx>
#include <boost/shared_ptr.hpp>
#include <comm-objects.h>
#include <map>
#include <string>
#include <vector>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <X11/X.h>
#include <X11/Xlib.h>

class MaskSwitcher;
class WorldViewConfig;

class MyOgreRoot : public Ogre::Root
{
public:
  MyOgreRoot() :
    Ogre::Root()
  {}
  bool fireFrameStarted() { return _fireFrameStarted(); }
  bool fireFrameEnded() { return _fireFrameEnded(); }
  bool fireFrameRenderingQueued() { return _fireFrameRenderingQueued(); }
};

/** This is a wrapper that can load and display Ogre scenes,
    optionally in multiple windows and viewports. It is inteded for
    encapsulation in a DUECA module, but can also be used stand-alone
    (see the test souce main.cpp in this directory).

    This class has been derived from the ScriptCreatable base class,
    and has a (scheme) script command to create it and optionally add
    parameters.

    The instructions to create an object of this class from the Scheme
    script are:

    \verbinclude ogre-viewer.scm

    The current capabilities of this viewer are:
    <ol>
    <li> Load scenes or objects from .scene files. This uses the
    DotSceneLoader class. Use this to populate your world with static
    objects.
    <li> Add simple (single mesh) classes easily with the
    'add-object-class keyword. These classes will be available for
    animating objects found in the multistreamchannel
    <li> The available classes are stored in a factory. It is
    possible to add user-defined classes to this factory.
    </ol>
*/
class OgreViewer : public WorldViewerBase
{
  // Advance definition, collection of data for a window.
  struct WindowSet;

  // Advance definition, collection of data for a viewport
  struct Private;

protected:
  /** Keep the pointer in Ogre */
  bool keep_pointer;

  /** Accept unknown/unconfigured objects */
  bool allow_unknown;

private:
  /** Basic root object */
  //  Ogre::Root     *root;
  MyOgreRoot *root;

  /** Transform defining the location of the vehicle, 'I', cameras are
      attached here. */
  Ogre::SceneNode *ego_transform;

  /** Specification for the render windows. */
  std::list<WinSpec> winspec;

  /** scene manager */
  Ogre::SceneManager *scene;

  /** Graphics context */
  GLXContext *gl_context;

protected:
  /** Wait divisor */
  int glx_sync_divisor;

  /** Wait offset */
  int glx_sync_offset;

private:
  /** Have to do the init */
  bool init_needed;

public:
  /** GL context cache structure */
  struct GLContextSet
  {
    ::Display *dpy;
    ::GLXContext context;
    ::Window xwin;
    GLContextSet(::Display *dpy = NULL, ::GLXContext context = NULL,
                 ::Window xwin = 0) :
      dpy(dpy),
      context(context),
      xwin(xwin)
    {}
  };

private:
  /** GL context cache */
  GLContextSet temp_glc;

  /** This class can generate multiple views on the same world. A
      ViewSet encapsulates the stuff needed for a single view. */
  struct ViewSet
  {

    /** The camera */
    Ogre::Camera *camera;

    /** The render window */
    Ogre::Viewport *viewport;

    /** Rotation offset */
    Ogre::Quaternion angle_offset;

    /** Position offset */
    Ogre::Vector3 rel_pos;

    /** Constructor */
    ViewSet();

    /** Initialise */
    void init(const ViewSpec &vs, WindowSet &window, Ogre::SceneManager *scene,
              int zorder);

    /** create the camera and window. */
    void complete();
  };

  /** Each render window needs a specification and possibly a set of
      views */
  struct WindowSet
  {

#ifdef OWN_WIN_AND_CONTEXT
    /** The X display device */
    ::Display *dpy;

    /** GL context */
    ::GLXContext glc;

    /** X window id */
    ::Window xwin;
#endif

    /** Listener for switching on appropriate mask */
    MaskSwitcher *switcher;

    /** The OGRE window pointer */
    Ogre::RenderWindow *window;

    /** A list of view sets; these represent the different render
        areas */
    std::list<ViewSet> viewset;

    /** Constructor */
    WindowSet() :
#ifdef OWN_WIN_AND_CONTEXT
      dpy(NULL),
      glc(NULL),
      xwin(0),
#endif
      switcher(NULL),
      window(NULL)
    {}

    /** Request that for the specified viewport, the specified overlay
        is turned on, and off again when viewport drawing is over. */
    void addViewportNotification(Ogre::Viewport *vp, Ogre::Overlay *ov);
  };

  /** Map of windows */
  typedef std::map<Ogre::String, WindowSet> WindowsMap;

  /** map with windows. */
  WindowsMap windows;

protected:
  /** Ogre additional resources */
  struct ResourceSpec
  {
    /** file location */
    std::string location;
    /** resource type */
    std::string type;
    /** resource group */
    std::string group;
    /** recursive reading? */
    bool recursive;

    /** Constructor */
    ResourceSpec(const std::string &l, const std::string &t,
                 const std::string &g, bool r) :
      location(l),
      type(t),
      group(g),
      recursive(r)
    {}
  };

  /** List of additional resources. */
  std::list<ResourceSpec> resourcespec;

private:
  /** Map with created (due to presence in the world channel)
      objects. They are indexed with channel entry index, and removed
      from the map when the entry is removed from the channel. */
  typedef std::map<creation_key_t, boost::intrusive_ptr<OgreObject>>
    created_objects_t;

  /** Objects creates automatically */
  created_objects_t active_objects;

  /** List with static  objects. */
  typedef std::list<boost::intrusive_ptr<OgreObject>> static_objects_t;

  /** Objects creates automatically */
  static_objects_t static_objects;

#ifdef OWN_WIN_AND_CONTEXT
  /** Keep a count of opened displays; re-use the pointer if a new
      window gets on an already opened display */
  std::map<Ogre::String, Display *> opened_displays;
#endif

  /** Helper */
  WindowSet myCreateWindow(const WinSpec &ws, Ogre::Root *root, bool waitswap);

  /** List of specifications for the wiews, will be applied later */
  std::list<ViewSpec> viewspec;

  /** A pre cooked scene in a .scene file */
  std::list<std::pair<std::string, std::string>> scene_files;

private:
  /** Helper function, loads resources from resources.cfg */
  void setupResources();

public:
  /** Constructor */
  OgreViewer(); // const char* name, bool interact = false);

  /** Destructor */
  ~OgreViewer();

  /** Check for preparedness / completenes of specification */
  virtual bool complete();

  /** Add a viewport */
  void addViewport(const ViewSpec &vp);

  /** Add a window */
  void addWindow(const WinSpec &window) { winspec.push_back(window); }

  /** set the base camera position
      @param tick  DUECA current time tick
      @param base  Object motion, position, etc.
      @param late  Time after DUECA tick */
  void setBase(TimeTickType tick, const BaseObjectMotion &base, double late,
               bool freeze) final;

  /** Set drawing context current; possibly needed when multiple GL windows
      use same thread */
  void makeContextCurrent();

  /** Add a scene to be loaded */
  bool addScene(const std::string &scene, const std::string &rgroup);

  /** Open the windows etc. In DUECA this is called from the
      first run in the drawing thread. */
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

  /** Create a static (not controlled) controllable object. */
  bool createStatic(const std::vector<std::string> &classname);

  /** Do a re-draw
      \param wait   If true, do now swap the buffers. The application
                    must later wait and swap with the waitSwap function. */
  void redraw(bool wait = false, bool reset_context = false);

  /** Wait for the swap. */
  void waitSwap();

  /** Change the configuration of the scene graph, returns true if
      successful */
  bool adaptSceneGraph(const WorldViewConfig &adapt);
};

#endif
