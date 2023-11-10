/* ------------------------------------------------------------------   */
/*      item            : OgreViewer.cxx
        made by         : Rene' van Paassen
        date            : 090616
        category        : body file
        description     :
        changes         : 090616 first version
        language        : C++
*/

#define OgreViewer_cxx
#include "OgreViewer.hxx"
#include "WorldObjectBase.hxx"
#include "OgreObject.hxx"
#include "DotSceneLoader.h"
#include "AxisTransform.hxx"
#include "OgreObjectFactory.hxx"
#if OGRE_VERSION >= ((1<<16 | (10<<8) | 0))
#include <OgreBitesConfigDialog.h>
#endif
#include "../comm-objects/WorldViewConfig.hxx"
#include <unistd.h>
#include <map>

#define D_MOD
#include <debug.h>

#define GLX_GLXEXT_PROTOTYPES 1
#include <GL/glxext.h>

#define DEB(A) std::cerr << A << std::endl;
#ifndef DEB
#define DEB(A)
#endif

/** Possibility to wait for video sync */
static PFNGLXWAITVIDEOSYNCSGIPROC glXWaitVideoSyncSGI = NULL;

/** Joining a swap group, when multiple windows. */
static PFNGLXJOINSWAPGROUPSGIXPROC glXJoinSwapGroupSGIX = NULL;

using namespace Ogre;
using namespace std;

// I tried creating X11 screen/display specific windows, and letting
// OGRE draw in these. It fails, for now. The fallback is letting OGRE
// create windows on the default display. In the HMI lab we will run
// two dueca processes on the outside visual pc (adapt multicast
// accessor and GenericStart script). Each process will have one
// panorama display with two projectors attached. Per panorama display,
// we run two viewports that are both running on one Ogre scene
// So there will be a duplication of the Ogre scene on that
// computer. However, it is a quad core, and this set-up enables
// compensation of the delay time in the view
//
//#define OWN_WIN_AND_CONTEXT 1

/** This is a listener-derived class. It will enable and disable the
    proper masks for the hmi lab masking */
class MaskSwitcher: public RenderTargetListener
{
  /** Set of overlays for a window */
  typedef std::map<Ogre::String, Overlay*> maskcollection_t;

  /** Collection of windows with overlays */
  typedef std::map<Viewport*, maskcollection_t> maskset_t;

  /** map with viewports and their associated masks */
  maskset_t  masks;

  /** callback, will enable the associated mask */
  virtual void preViewportUpdate ( const RenderTargetViewportEvent &ev)
  {
    maskset_t::const_iterator ii = masks.find ( ev.source);
    if ( ii != masks.end()) {
      for (maskcollection_t::const_iterator jj = ii->second.begin();
           jj != ii->second.end(); jj++) {
        jj->second->show();
      }
    }
  }

  /** callback, after drawind the mask is hidden again */
  void postViewportUpdate ( const RenderTargetViewportEvent &ev)
  {
    maskset_t::const_iterator ii = masks.find ( ev.source);
    if ( ii != masks.end()) {
      for (maskcollection_t::const_iterator jj = ii->second.begin();
           jj != ii->second.end(); jj++) {
        jj->second->hide();
      }
    }
  }
public:

  /** Add an entry to the map */
  void addEntry ( Viewport* vp, Overlay* ov)
  {
    // first check for already existing overlay for the selected Viewport *vp
    maskset_t::iterator vpset = masks.find(vp);

    if (vpset == masks.end()) {
      // first time, insert an empty set of overlays
      masks[vp] = maskcollection_t();
    }

    // insert the overlay
    masks[vp][ov->getName()] =  ov;
  }

  void removeEntry ( Viewport* vp, const Ogre::String& ovname)
  {
    // first check for already existing overlay for the selected Viewport *vp
    maskset_t::iterator vpset = masks.find(vp);

    if (vpset != masks.end()) {
      maskcollection_t::iterator ovl = vpset->second.find(ovname);
      if (ovl != vpset->second.end()) {
        vpset->second.erase(ovl);
      }
    }
  }
};

void OgreViewer::WindowSet::addViewportNotification
( Ogre::Viewport* vp, Ogre::Overlay *ov)
{
  if ( !switcher) {
    cout << "OgreViewer: Creating new listener" << endl;
    assert ( window != NULL);
    switcher = new MaskSwitcher();
    window->addListener ( switcher);
  }
  cout << "OgreViewer: Adding overlay to listener" << endl;
  switcher->addEntry ( vp, ov);
}


OgreViewer::ViewSet::ViewSet() :
  camera ( NULL),
  viewport ( NULL),
  angle_offset ( Quaternion::IDENTITY),
  rel_pos ( Vector3::ZERO)
{

}


void OgreViewer::ViewSet::init ( const ViewSpec& spec,
                                 WindowSet& ws,
                                 Ogre::SceneManager *scene,
                                 int zorder)
{
  cout << "OgreViewer: Creating camera \"" << spec.name
       << "\" in window \"" << ws.window->getName() << '"' << endl;
  camera = scene->createCamera ( spec.name.c_str());

  // attach it to the ego motion node
  scene->getSceneNode("__ego_node__")->attachObject(camera);

  if ( spec.portcoords.size() == 4) {
    viewport = ws.window->addViewport
      ( camera, zorder,
        float ( spec.portcoords[0]) /float ( ws.window->getWidth()),
        float ( spec.portcoords[1]) /float ( ws.window->getHeight()),
        float ( spec.portcoords[2]) /float ( ws.window->getWidth()),
        float ( spec.portcoords[3]) /float ( ws.window->getHeight()));
  }
  if ( spec.frustum_data.size() >= 2) {
    camera->setNearClipDistance ( spec.frustum_data[0]);
    camera->setFarClipDistance ( spec.frustum_data[1]);
  }
  if ( spec.frustum_data.size() == 3) {
    camera->setFOVy ( Radian ( Degree ( spec.frustum_data[2])));
  }
  else if ( spec.frustum_data.size() == 6) {
    camera->setFrustumExtents ( spec.frustum_data[2], spec.frustum_data[3],
                                spec.frustum_data[5], spec.frustum_data[4]);
  }
  if ( spec.eye_pos.size() >= 3) {
    rel_pos = AxisTransform::ogrePosition
      ( spec.eye_pos[0], spec.eye_pos[1], spec.eye_pos[2]);
    camera->setPosition(rel_pos);
  }
  if ( spec.eye_pos.size() == 6) {
    angle_offset = AxisTransform::ogreQuaternion
      ( spec.eye_pos[3], spec.eye_pos[4], spec.eye_pos[5]);
    camera->setOrientation(angle_offset);
  }
  if ( spec.overlay.size()) {
    cout << "OgreViewer: Looking for overlay " << spec.overlay << endl;
    Overlay *ov = OverlayManager::getSingleton().getByName ( spec.overlay);
    if ( !ov) {
      cerr << "OgreViewer: Cannot find overlay " << spec.overlay << endl;
    }
    else {
      ws.addViewportNotification ( viewport, ov);
    }
  }
}

OgreViewer::OgreViewer() :
  keep_pointer(false),
  allow_unknown(false),
  root(NULL),
  scene(NULL),
  glx_sync_divisor(1),
  glx_sync_offset(0),
  init_needed(true)
{
  // create the root and try to read configuration from ogre.cfg
  root = new MyOgreRoot();

  // read the resources from resources.cfg
  setupResources();

  // try to read configuration from ogre.cfg
  if ( access("ogre.cfg", R_OK))
    {
#if OGRE_VERSION >= ((1<<16 | (10<<8) | 0))
      Ogre::ConfigDialog* dialog = OgreBites::getNativeConfigDialog();
      if (!root->showConfigDialog(dialog))
#else
      if (!root->showConfigDialog())
#endif
        {
          cerr << "OgreViewer: no valid config" << endl;
          return;
        }
    }
  else
    {
      root->restoreConfig();
    }

}


OgreViewer::~OgreViewer()
{

}

void OgreViewer::setupResources()
{
  ConfigFile cf;
  cf.load("resources.cfg");

  // Go through all sections & settings in the file
  ConfigFile::SectionIterator seci = cf.getSectionIterator();

  String secName, typeName, archName;
  while ( seci.hasMoreElements())
    {
      secName = seci.peekNextKey();
      ConfigFile::SettingsMultiMap *settings = seci.getNext();
      ConfigFile::SettingsMultiMap::iterator i;
      for ( i = settings->begin(); i != settings->end(); ++i)
        {
          typeName = i->first;
          archName = i->second;
          ResourceGroupManager::getSingleton().addResourceLocation
            ( archName, typeName, secName);
        }
    }
}

/** Quick exception struct. */
struct DuecaOgreConfigError: public std::exception
{
  /** Say what is the problem */
  const char* what() { return "Configuration error ogre"; }
};


inline void saveGLContext(OgreViewer::GLContextSet& temp_glc,
                            Ogre::RenderWindow *window)
{
  ::GLXContext glc = NULL;
  ::Window xwin = 0;
  ::Display* dpy = NULL;
  window->getCustomAttribute("WINDOW", &xwin);
  window->getCustomAttribute("DISPLAY", &dpy);
  window->getCustomAttribute("CLCONTEXT", &glc);
  temp_glc = OgreViewer::GLContextSet(dpy, glc, xwin);
}

inline void restoreGLContext(OgreViewer::GLContextSet& temp_glc)
{
  if ( temp_glc.context) {
    glXMakeCurrent(temp_glc.dpy, temp_glc.xwin, temp_glc.context);
  }
}


// to simplify programming
inline OgreViewer::WindowSet
OgreViewer::myCreateWindow(const WinSpec &ws, Root* root, bool waitswap)
{
  NameValuePairList args;
  WindowSet res;

  // check if it is on a deflt display, if not, add the parent window
  // handle
  String display = ws.display.size() ? ws.display : std::string(":0.0");

  cout << "OgreViewer: Creating window \"" << ws.name << '"' << endl;

#ifdef OWN_WIN_AND_CONTEXT
  // remove the screen number
#if OGRE_VERSION <= (( 1 << 16) | ( 6 << 8) | ( 4))
  std::vector<String> tok = StringUtil::split ( display, ".");
#else
  Ogre::vector<String>::type tok = StringUtil::split ( display, ".");
#endif

  // get the display part, open if needed.
  int screen = 0;
  if ( opened_displays.find ( tok[0]) == opened_displays.end())
    {
      res.dpy = XOpenDisplay ( display.c_str());
      screen = XDefaultScreen ( res.dpy);
      opened_displays[ tok[0] ] = res.dpy;
    }
  else
    {
      res.dpy = opened_displays[ tok[0] ];
    }

  Window xroot = RootWindow ( res.dpy, screen);

  GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
  XVisualInfo *vi = glXChooseVisual ( res.dpy, screen, att);
  if ( vi == NULL)
    {
      cerr << "OgreViewer: Cannot find appropriate visual" << endl;
    }

  XSetWindowAttributes    swa = { };
  Colormap cmap = XCreateColormap ( res.dpy, xroot, vi->visual, AllocNone);
  swa.colormap = cmap;
  swa.event_mask = ExposureMask | KeyPressMask;
  res.xwin = XCreateWindow
    ( res.dpy, xroot, 0, 0,
      ws.size_and_position[0], ws.size_and_position[1], 0, vi->depth,
      InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
  XMapWindow ( res.dpy, res.xwin);
  XStoreName ( res.dpy, res.xwin, ws.name.c_str());

  res.glc = glXCreateContext ( res.dpy, vi, NULL, GL_TRUE);
  glXMakeCurrent ( res.dpy, res.xwin, res.glc);

  args["externalGLControl"] = "True";

  res.window = root->createRenderWindow ( ws.name, 0, 0, false, &args);
#else

  if ( waitswap)
    {
      args["vsync"] = "True";
      args["vsyncInterval"] = Ogre::StringConverter::toString ( glx_sync_divisor);
    }

  // Full screen?
  if ( ws.size_and_position.size() == 0) {
    res.window = root->createRenderWindow ( String ( ws.name), 0, 0, true, &args);
  }
  else {
    // Check for position information
    if ( ws.size_and_position.size() == 4) {
      args["left"] = StringConverter::toString ( ws.size_and_position[2]);
      args["top"] = StringConverter::toString ( ws.size_and_position[3]);
    }
    res.window = root->createRenderWindow
      ( ws.name, ws.size_and_position[0], ws.size_and_position[1],
        false, &args);
  }

  // reset the cursor manually (
  if ( !keep_pointer) {
    ::Window xwin = 0;
    ::Display* dpy = NULL;
    res.window->getCustomAttribute ( "WINDOW", &xwin);
    res.window->getCustomAttribute ( "DISPLAY", &dpy);
    assert ( xwin != 0); assert ( dpy != NULL);

    static const char emptycursor_bits[6] = {0x00};
    Pixmap cursor_pixmap =
      XCreateBitmapFromData ( dpy, xwin, emptycursor_bits, 1, 1);
    XColor black;
    black.pixel = BlackPixel ( dpy, 0);
    Cursor cursor = XCreatePixmapCursor ( dpy, cursor_pixmap, cursor_pixmap,
                                          &black, &black, 0, 0);
    XDefineCursor ( dpy, xwin, cursor);
    XFreePixmap ( dpy, cursor_pixmap);
  }

#if 1
  // get the gl context
  if (!temp_glc.context) {
    saveGLContext(temp_glc, res.window);
  }
  else {
    ::GLXContext          gl2_context;
    res.window->getCustomAttribute ( "GLCONTEXT", &gl2_context);
    if (temp_glc.context != gl2_context) {
      cerr << "Ogre created 2 different GL contexts" << endl;
    }
  }
#endif

#endif

  if ( !res.window) {
    cerr << "OgreViewer: cannot create window \"" << ws.name.c_str() << '"' << endl;
    throw ( DuecaOgreConfigError());
  }

  return res;
}

bool OgreViewer::complete()
{

  // Create a default window if nothing was specified
  if ( winspec.empty()) {
    WinSpec window;
    window.name = "DUECA/OGRE default window";
    float size_and_position[] = { 400, 300};
    window.size_and_position.resize ( 2);
    copy ( &size_and_position[0], &size_and_position[2],
           window.size_and_position.begin());
    window.display = ":0.0";
    addWindow ( window);
  }
  return true;
}

void OgreViewer::makeContextCurrent()
{
  // grep context of first window??? Is that shared in OGRE??
  restoreGLContext(temp_glc);
}

void OgreViewer::init(bool waitswap)
{
  // initialise without window, then create the window(s)
  root->initialise(false);

  // last window name
  string last_window;
  while (!winspec.empty()) {
    if (windows.find(winspec.front().name) != windows.end()) {
      cerr << "OgreViewer: Already specified a window " << winspec.front().name
           << " ignoring second one" << endl;
    }
    windows[winspec.front().name] =
      myCreateWindow(winspec.front(), root, waitswap && windows.size() == 0);
    last_window = winspec.front().name;
    winspec.pop_front();
  }

  // scene manager
  scene = root->createSceneManager( ST_GENERIC, "OgreViewerInst");
  //ego_transform = scene->createSceneNode("__ego_node__");
  ego_transform = scene->getRootSceneNode()->
    createChildSceneNode("__ego_node__");

#if OGRE_VERSION >= 0x010900
  Ogre::OverlaySystem *mOverlaySystem = new Ogre::OverlaySystem();
  scene->addRenderQueueListener(mOverlaySystem);
#endif

  // Set default mipmap level (NB some APIs ignore this)
  TextureManager::getSingleton().setDefaultNumMipmaps ( 5);

  // optionally create resource listener??
  while (!resourcespec.empty()) {
    ResourceGroupManager::getSingleton().addResourceLocation
      (resourcespec.front().location, resourcespec.front().type,
       resourcespec.front().group, resourcespec.front().recursive);
    resourcespec.pop_front();
  }

  // initialise all resource groups
  ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

  // create cameras and viewports
  int zorder = 0; // should do zorder per window??
  while ( !viewspec.empty()) {
    // find the appropriate window
    WindowsMap::iterator ii = windows.find ( viewspec.front().winname);
    if ( ii == windows.end()) {
      cerr << "OgreViewer: Could not find window \"" << viewspec.front().winname
           << "\" for view \"" << viewspec.front().name << '"' << endl;
      viewspec.pop_front();
    }
    else {
      if ( viewspec.front().render_shadows) {
        // select the right shadow technique
        if ( viewspec.front().shadow_technique == "stencil")
          scene->setShadowTechnique (Ogre::SHADOWTYPE_STENCIL_MODULATIVE);
        else
          scene->setShadowTechnique (Ogre::SHADOWTYPE_TEXTURE_MODULATIVE);

        // limit the range for which we draw shadows, so we can limit
        // the impact on the renderer
        scene->setShadowFarDistance (viewspec.front().shadow_farDistance);

        // set the shadow colour
        scene->setShadowColour (Ogre::ColourValue (viewspec.front().shadow_colour[0],
                                                     viewspec.front().shadow_colour[1],
                                                     viewspec.front().shadow_colour[2]));

      }
      ii->second.viewset.push_back (ViewSet());

      ii->second.viewset.back().init
        (viewspec.front(), ii->second, scene, zorder++);
      viewspec.pop_front();
    }
  }

  // if applicable, load a default scene
  while (scene_files.size())
    {
      Ogre::DotSceneLoader scene_loader;
      scene_loader.parseDotScene
        (scene_files.front().first, scene_files.front().second, scene);
      cout << "OgreViewer: Loaded file " << scene_files.front().first << " with " <<
        scene_loader.staticObjects.size() << " static and " <<
        scene_loader.dynamicObjects.size() << " dynamic objects" << endl;
      scene_files.pop_front();
    }

  // explicitly load the (manual) controllable models
  for (ControllablesMap::const_iterator ii = controllables.begin();
        ii != controllables.end(); ii++)
    {
      if (dynamic_cast<OgreObject*> (ii->second.first))
        {
          dynamic_cast<OgreObject*> (ii->second.first)->init (scene);
        }
    }
  /* Seems to be unnecessary as the active_objects are already initialised upon addition to the scene
     Mark Mulder 29/9/2010
     for (ObjectListType::iterator ii = active_objects.begin();
     ii != active_objects.end(); ii++) (*ii)->init (scene);
  */

  glXWaitVideoSyncSGI = reinterpret_cast<PFNGLXWAITVIDEOSYNCSGIPROC>
    (glXGetProcAddress
      (reinterpret_cast<const GLubyte*> ("glXWaitVideoSyncSGI")));
  glXJoinSwapGroupSGIX = reinterpret_cast<PFNGLXJOINSWAPGROUPSGIXPROC>
    (glXGetProcAddress
      (reinterpret_cast<const GLubyte*> ("glXJoinSwapGroupSGIX")));

  // if multiple windows are opened, and a swap group is possible,
  // join these windows
  WindowsMap::iterator ii = windows.begin();
  if (glXJoinSwapGroupSGIX && ii != windows.end()) {
    ::Display* dpy; ::Window xwin;
    ii->second.window->getCustomAttribute ("DISPLAY", &dpy);
    ii->second.window->getCustomAttribute ("WINDOW", &xwin);
    while (++ii != windows.end()) {
      ::Window xwin2;
      ii->second.window->getCustomAttribute ("WINDOW", &xwin2);
      (*glXJoinSwapGroupSGIX) (dpy, xwin, xwin2);
    }
  }

  // initialize any objects already created
  for (auto &obj : active_objects) {
    obj.second->init(scene);
  }
  for (auto &obj : static_objects) {
    obj->init(scene);
  }

  // left this code in for when Ogre would complain/not handle
  // resetting the current context.
  // store the context of the last created window, and set the current
  // context to none (needed for thread changes!)
  //saveGLContext(temp_glc, windows[last_window].window);
}

void OgreViewer::addViewport (const ViewSpec& vp)
{
  viewspec.push_back (vp);
}

void OgreViewer::redraw(bool wait, bool reset_context)
{
  // call this to throw some frame events that make sure
  // texture controllers are updated properly
  root->fireFrameStarted();

  for (WindowsMap::const_iterator ii = windows.begin();
        ii != windows.end(); ii++) {
      ii->second.window->update (!wait);
  }

  // these calls finish the fireFrameStarted() call
  root->fireFrameRenderingQueued();
  root->fireFrameEnded();
}

#define SWAP_FIRST_THEN_WAIT
void OgreViewer::waitSwap()
{
#ifdef SWAP_FIRST_THEN_WAIT
  // ask for a swap on all buffers, with a wait only on the
#if OGRE_VERSION < 0x010900
  unsigned nwin = windows.size(), iwin = 0;
#endif
  for (WindowsMap::const_iterator ii = windows.begin();
        ii != windows.end(); ii++)
    {
#if OGRE_VERSION >= 0x010900
      ii->second.window->swapBuffers ();
#else
      ii->second.window->swapBuffers (++iwin == nwin);
#endif
    }
#endif
#ifdef SELF_WAIT
  unsigned int count;
  if (glXWaitVideoSyncSGI && windows.size())
    {
      int res = (*glXWaitVideoSyncSGI) (glx_sync_divisor, glx_sync_offset, &count);
      switch (res)
        {
        case 0:
          return;
        case GLX_BAD_VALUE:
          cerr << "OgreViewer: error glXWaitVideoSyncSGI, bad params" << endl;
          break;
        case GLX_BAD_CONTEXT:
          cerr << "OgreViewer: error glXWaitVideoSyncSGI, bad context" << endl;
        default:
          cerr << "OgreViewer: error glXWaitVideoSyncSGI, " << res << endl;
        }
      glXWaitVideoSyncSGI = NULL;
    }
#endif
#ifndef SWAP_FIRST_THEN_WAIT
  // ask for a swap on all buffers
  for (WindowsMap::const_iterator ii = windows.begin();
        ii != windows.end(); ii++)
    {
#if OGRE_VERSION >= 0x010900
      ii->second.window->swapBuffers ();
#else
      ii->second.window->swapBuffers (true);
#endif
    }
#endif
}

void OgreViewer::setBase(TimeTickType tick, const BaseObjectMotion& base,
                         double late)
{
  ego_transform->setPosition(AxisTransform::ogrePosition(base.xyz));
  ego_transform->setOrientation(AxisTransform::ogreQuaternion(base.attitude_q));

  // this either uses the base, a channel reader, or both, to update
  for (auto &obj: active_objects) {
    obj.second->iterate(tick, base, late);
  }
  for (auto &obj: static_objects) {
    obj->iterate(tick, base, late);
  }

#if 0
  Quaternion camrot = AxisTransform::ogreQuaternion (quat);
  Vector3 base = AxisTransform::ogrePosition (xyz[0], xyz[1], xyz[2]);
  Matrix4 rot (camrot);

  for (WindowsMap::const_iterator jj = windows.begin();
        jj != windows.end(); jj++)
    {
      for (std::list<ViewSet>::const_iterator ii = jj->second.viewset.begin();
            ii != jj->second.viewset.end(); ii++)
        {
          ii->camera->setPosition (base + rot * ii->rel_pos);
          ii->camera->setOrientation (camrot * ii->angle_offset);
        }
    }
#endif
}

bool OgreViewer::addScene (const std::string& scene, const std::string& group)
{
  scene_files.push_back (std::pair<std::string,std::string> (scene, group));
  return true;
}


bool OgreViewer::createControllable
(const GlobalId& master_id, const NameSet& cname, entryid_type entry_id,
 uint32_t creation_id, const std::string& data_class,
 const std::string& entry_label, Channel::EntryTimeAspect time_aspect)
{
  creation_key_t keypair(cname.name, creation_id);

  // check
  assert(active_objects.count(keypair) == 0);

  // create entry on the basis of data class and entry label
  OgreObject *op = NULL;
  WorldDataSpec obj;
  try {
    obj = retrieveFactorySpec(data_class, entry_label, creation_id);

    // find the factory and create an object
    op = OgreObjectFactory::instance().create(obj.type, obj);
    op->connect(master_id, cname, entry_id, time_aspect);
    if (scene) {
      op->init(scene);
    }
    boost::intrusive_ptr<OgreObject> bop(op);
    active_objects[keypair] = bop;
    return true;
  }
  catch (const CFCannotMake& problem) {
    if (!allow_unknown) {
      W_MOD("OgreViewer: factory cannot create for " << data_class <<
            " encountered: " <<  problem.what());
      throw(problem);
    }
    W_MOD("OgreViewer: factory cannot create for " << data_class <<
          ", ignoring channel " << cname << " entry " << entry_id);
  }
  catch (const MapSpecificationError& problem) {
    if (!allow_unknown) {
      W_MOD("OgreViewer: not configured for " << data_class <<
            " encountered: " <<  problem.what());
      throw(problem);
    }
    W_MOD("OgreViewer: not configured for " << data_class <<
          ", ignoring channel " << cname << " entry " << entry_id);
  }
  catch (const std::exception& problem) {
    cerr << "OgreViewer: When trying to create for " << data_class
         << " encountered: " <<  problem.what() << endl;
    throw(problem);
  }
  return false;
}

bool OgreViewer::createStatic(const std::vector<std::string>& name)
{
  auto obj = retrieveFactorySpec(name[0], "", static_objects.size(), true);
  if (obj.type.size() == 0) {
    E_MOD("cannot find object type \"" << name[0] << "\" in the factory");
    return false;
  }

  if (name.size() >= 2) {
    obj.name = name[1];
  }
  try {
    OgreObject *op = OgreObjectFactory::instance().create(obj.type, obj);
    if (scene) {
      op->init(scene);
    }
    boost::intrusive_ptr<OgreObject> bop(op);
    static_objects.push_back(bop);
    return true;
  }
  catch (const std::exception& problem) {
    cerr << "OgreViewer: When trying to create " << obj.type
         << " encountered: " <<  problem.what() << endl;
  }
  return false;
}

void OgreViewer::removeControllable(const NameSet& cname, uint32_t creation_id)
{
  active_objects.erase(std::make_pair(cname.name, creation_id));
}

bool OgreViewer::adaptSceneGraph (const WorldViewConfig& adapt)
{
  try {

    Ogre::SceneNode *node = NULL;

    switch (adapt.command) {

    case WorldViewConfig::ReadScene:
    case WorldViewConfig::ClearModels: {

      // un-init all controlled objects
      for (auto &obj : active_objects) { obj.second->unInit(scene); }
      for (auto &obj : static_objects) { obj->unInit(scene); }

      scene->clearScene();

      // remove everything else except transform
      //size_t idx = 0;
      //Ogre::SceneNode *rt = scene->getRootSceneNode();
      //while (idx < rt->numChildren()) {
      //  if (rt->getChild(idx) == ego_transform) {
      //    idx++;
      //  }
      //  else {
      //    rt->removeAndDestroyChild(idx);
      //  }
      //}

      ego_transform = scene->getRootSceneNode()->
        createChildSceneNode("__ego_node__");

      for (auto &win: windows) {
        for (auto &vs: win.second.viewset) {
          ego_transform->attachObject(vs.camera);
        }
      }

      // detach the ego node with the cameras
      //scene->getRootSceneNode()->removeChild(ego_transform);

      // clear the scene
      //scene->clearScene();

      // attach the ego again
      //scene->getRootSceneNode()->addChild(ego_transform);

      // reload controllables
      for (auto &obj : active_objects) { obj.second->init(scene); }
      for (auto &obj : static_objects) { obj->init(scene); }
    }
      if (adapt.command == WorldViewConfig::ClearModels) break;

      {
        // load the new scene files
        for (auto &ff :adapt.config.filename) {
          Ogre::DotSceneLoader scene_loader;
          scene_loader.parseDotScene (ff, adapt.config.type, scene);
          cout << "OgreViewer: Loaded file " << ff << " with " <<
            scene_loader.staticObjects.size() << " static and " <<
            scene_loader.dynamicObjects.size() << " dynamic objects" << endl;
        }
      }
      break;

    case WorldViewConfig::RemoveNode:
      scene->destroySceneNode(adapt.config.name);
      scene->destroyEntity(adapt.config.name);
      break;

    case WorldViewConfig::LoadOverlay: {
      ////////////////////// NEW CODE - Mark Mulder 29/9/2010
#ifdef D_MOD
      cout  << "OgreViewer: number of defined windows in WindowsMap = "
            << windows.size() << endl;
      int count = 1;
      for (WindowsMap::const_iterator ii = windows.begin();
           ii != windows.end(); ii++) {
        cout << "OgreViewer: window(" << count << ") name = "
             << ii->first << "; nr of viewsets = "
             << ii->second.viewset.size() << endl;
        int set_count = 1;
        for (std::list<ViewSet>::const_iterator i =
               ii->second.viewset.begin();
             i != ii->second.viewset.end(); ++i) {
          cout << "  >>>> viewset(" << set_count
               << ") camera name = " << i->camera->getName() << endl;
        }
        count++;
      }
#endif
      // try to set a different mask for the left front viewport
      // first try to find the front window
      if (windows.find(adapt.viewspecs.winname) != windows.end()) {
        // then try to find the left viewport of the front window
        bool found_viewport = false;
        std::list<ViewSet>::const_iterator i =
          windows.find(adapt.viewspecs.winname)->second.viewset.begin();

        while (i != windows.find(adapt.viewspecs.winname)->
               second.viewset.end()) {
          // have we found the right camera?
          if (i->camera->getName() == adapt.viewspecs.name) {
            Overlay *ov = OverlayManager::getSingleton().getByName
              (adapt.viewspecs.overlay);
            if (!ov) {
              cerr << "OgreViewer: Cannot find overlay <"
                   << adapt.viewspecs.overlay
                   << "> - overlay not adjusted" << endl;
            }
            else {
              windows.find(adapt.viewspecs.winname)->
                second.addViewportNotification (i->viewport, ov);
            }
            found_viewport = true;
            // leave the loop, because we found the right window
            break;
          }
          // keep looking for the right camera
          else {
            i++;
          }
        }
        if (!found_viewport) {
          cerr << "OgreViewer: Could not find the specified viewport <"
               << adapt.viewspecs.name << "> - overlay not adjusted" << endl;
        }
      }
      else {
        cerr << "OgreViewer: Could not find the specified window <"
             << adapt.viewspecs.winname << "> - overlay not adjusted" << endl;
      }
      ////////////////////// END NEW CODE
      break;
    }

    case WorldViewConfig::RemoveOverlay: {
      ////////////////////// NEW CODE - RvP 15/9/2016
      // Update may 2017

      // find the window mentioned in the event
      WindowsMap::iterator win = windows.find(adapt.viewspecs.winname);

      if (win != windows.end()) {

        // and the corresponding viewport
        for (std::list<ViewSet>::iterator vs = win->second.viewset.begin();
             vs != win->second.viewset.end(); vs++) {

          if (vs->camera->getName() == adapt.viewspecs.name) {
            win->second.switcher->removeEntry
              (vs->viewport, adapt.viewspecs.overlay);

            // Success, exit here
            return true;
          }
        }
      }

      // flag failure ...
      W_MOD("Could not find window \"" << adapt.viewspecs.winname <<
            "\" view \"" << adapt.viewspecs.name <<
            "\" for removing overlay \"" << adapt.viewspecs.overlay << "\"");
      return false;
    }

    case WorldViewConfig::LoadObject: {
      if (adapt.config.filename.size() != 1) {
        cerr << "OgreViewer: Must specify the mesh name" << endl;
        return false;
      }
      node = scene->getRootSceneNode()->
        createChildSceneNode (adapt.config.name);
      {
        MeshPtr m = Ogre::MeshManager::getSingleton().load
          (adapt.config.filename[0], adapt.config.type);
        Ogre::Entity* entity = scene->createEntity
          (adapt.config.name.c_str(), adapt.config.filename[0]);
        node->attachObject (entity);
      }
      // intentional fall-through
    }

    case WorldViewConfig::MoveObject: {
      if (node == NULL) {
        node = scene->getSceneNode (adapt.config.name);
      }
      if (!node) return false;

      {
        int ncoord = adapt.config.coordinates.size();
        if (ncoord != 3 && ncoord != 6 && ncoord != 9) {
          cerr << "OgreViewer: Specify position <orientation> <scale>"
               << endl;
          return false;
        }
        node->setPosition (adapt.config.coordinates[0],
                            adapt.config.coordinates[1],
                            adapt.config.coordinates[2]);
        if (ncoord == 3) break;
        node->setOrientation
          (AxisTransform::ogreQuaternion (adapt.config.coordinates[3],
                                            adapt.config.coordinates[4],
                                            adapt.config.coordinates[5]));
        if (ncoord == 6) break;
        node->setScale (adapt.config.coordinates[6],
                         adapt.config.coordinates[7],
                         adapt.config.coordinates[8]);
      }
      break;
    }
    case WorldViewConfig::ListNodes: {
      cerr << "OgreViewer: ListNodes not implemented" << endl;
      break;
    }
    }
  }
  catch (const Ogre::Exception& e) {
    std::cerr << "OgreViewer: OgreObject caught" << e.what() << std::endl;
    return false;
  }

  return true;
}
