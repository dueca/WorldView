/* ------------------------------------------------------------------   */
/*      item            : PlibViewer.cxx
        made by         : Rene' van Paassen
        date            : 100126
	category        : body file 
        description     : 
	changes         : 100126 first version
        language        : C++
*/

#define PlibViewer_cxx
#include "PlibViewer.hxx"
#include <cmath>
#include "AxisTransform.hxx"
#include <iostream>
#include "PlibObjectFactory.hxx"

#include <X11/X.h>
#include <X11/Xlib.h>
#define GLX_GLXEXT_PROTOTYPES 1
#include <GL/glx.h>
#include <GL/glxext.h>
#include <GL/glu.h>
#include <GL/glu.h>

#include <X11/XKBlib.h>
#include <dueca/debug.h>

using namespace std;

/** Hidden object for X implementation, to prevent all kinds of 
    #define trouble */
struct PlibViewerImp
{
/** Possibility to wait for video sync */
  PFNGLXWAITVIDEOSYNCSGIPROC glXWaitVideoSyncSGI;

  /** Joining a swap group, when multiple windows. */
  PFNGLXJOINSWAPGROUPSGIXPROC glXJoinSwapGroupSGIX;

  /** single context to share display lists etc. */
  GLXContext shared_context;

  /** Cursor */
  Cursor     cursor;

/** Constructor */
PlibViewerImp() :
  glXWaitVideoSyncSGI(NULL),
  glXJoinSwapGroupSGIX(NULL),
  shared_context(NULL)
{ }

};

/** Organisation of a single X window, possibly with multiple
    cameras and views */
struct PlibViewer_WindowSet 
{
/** Name of this object */
    std::string name;

    /** Pointer to the viewer, to get to common data */
    PlibViewer* viewer;
    
    /** The X display device */
    ::Display* dpy;

    /** GL context */
    ::GLXContext glc;

    /** X window id */
    ::Window xwin;

    /** A list of view sets; these represent the different render
	areas */
    std::list<PlibViewer::ViewSet> viewsets;
    
    /** Constructor */
    PlibViewer_WindowSet(const std::string& name = "", PlibViewer* viewer = NULL) : 
      name(name), viewer(viewer), dpy(NULL), glc(NULL), xwin(0)
    { }

    /** Main function drawing */
    void redraw(bool wait);

    /** make the gl context current */
    void contextCurrent(bool own = true);

    /** Called whenever the window needs redrawing. This has the GL code
	to redraw the display. */
    void display();
    
    /** Do the swap */
    void swap();
  }; 

PlibViewer::ViewSet::ViewSet() :
  camera(NULL)
{
  for (int ii = 3; ii--; ) {
    angle_offset[ii+1] = 0.0;
    rel_pos[ii] = 0.0;
  }
  angle_offset[0] = 1.0;
}

PlibViewer::PlibViewer() :
  WorldViewerBase(), 
  scene(NULL),
  imp(new PlibViewerImp),
  glx_sync_divisor(1),
  glx_sync_offset(0),
  keep_pointer(false)
{
  bg_color.push_back(0.1);
  bg_color.push_back(0.1);
  bg_color.push_back(0.1);
}


PlibViewer::~PlibViewer()
{
  
}

bool PlibViewer::complete()
{
  // Create a default window if nothing was specified
  if (winspec.empty()) {
    WinSpec window;
    window.name = "DUECA/PLIB default window";
    float size_and_position[] = { 400, 300};
    window.size_and_position.resize(2);
    copy(&size_and_position[0], &size_and_position[2],
	 window.size_and_position.begin());
    addWindow(window);
  }
  return true;
}

inline static Bool WaitForNotify (Display *d, XEvent *e, char *arg)
{
  return (e->type == MapNotify) & (e->xmap.window == (Window)arg);
}

inline PlibViewer_WindowSet 
PlibViewer::myCreateWindow(const WinSpec &ws)
{
  PlibViewer_WindowSet res(ws.name, this);
  cout << "Creating window \"" << ws.name << '"' << endl;

  // get the display part, open if needed. Currently only using
  // full string. 
  int screen = 0;
  if (opened_displays.find(ws.display) == opened_displays.end()) {
    if (ws.display.size()) {
      res.dpy = XOpenDisplay(ws.display.c_str());
    }
    else {
      res.dpy = XOpenDisplay(NULL);
    }
    screen = XDefaultScreen(res.dpy);
    opened_displays[ws.display] = res.dpy;
  }
  else {
    res.dpy = opened_displays[ws.display];
    screen = XDefaultScreen(res.dpy);
  }
  
  if (res.dpy == NULL) {
    cerr << "Could not open display " << ws.display << endl;
  } 

  /// access the root window for further steps
  Window xroot = RootWindow(res.dpy, screen);

  // continue with GL
  GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
  XVisualInfo *vi = glXChooseVisual(res.dpy, screen, att);
  if (vi == NULL) {
    cerr << "Cannot find appropriate visual" << endl;
  }
  
  XSetWindowAttributes    swa = { };
  Colormap cmap = XCreateColormap(res.dpy, xroot, vi->visual, AllocNone);

  swa.colormap = cmap;
  // events for user: XMotionEvent XKeyEvent XButtonEvent
  swa.event_mask = ExposureMask | StructureNotifyMask 
    /* | KeyPressMask | KeyReleasMask |
    ButtonPressMask | ButtonReleaseMask |
    PointerMotionMask | ButtonMotionMask */;
  res.xwin = XCreateWindow
    (res.dpy, xroot, 0, 0, 
     ws.size_and_position[0], ws.size_and_position[1], 0, vi->depth, 
     InputOutput, vi->visual, CWColormap|CWEventMask, &swa);
  XMapWindow(res.dpy, res.xwin);
  XEvent event;
  XStoreName(res.dpy, res.xwin, ws.name.c_str());

  // create a context. The context is shared between all windows with
  // plib, first context becomes the one for sharing
  res.glc = glXCreateContext(res.dpy, vi, imp->shared_context, GL_TRUE);
  if (imp->shared_context == NULL) imp->shared_context = res.glc;


  
  // remove mouse pointer
  if (!keep_pointer) {
    static const char emptycursor_bits[6] = {0x00};
    Pixmap cursor_pixmap = XCreateBitmapFromData(res.dpy, res.xwin, 
						 emptycursor_bits, 1, 1);
    XColor black;
    black.pixel = BlackPixel(res.dpy, screen);
    imp->cursor = XCreatePixmapCursor(res.dpy, cursor_pixmap, cursor_pixmap, 
				 &black, &black, 0, 0); 
    XDefineCursor(res.dpy, res.xwin, imp->cursor);
    XFreePixmap(res.dpy, cursor_pixmap);
  }

  // this waits for the creation notification
  XIfEvent(res.dpy, &event, WaitForNotify, reinterpret_cast<char*>(res.xwin));

  // now re-set the event stuff
  XSelectInput(res.dpy, res.xwin, ws.mask);
  
  // according to valgrind (SLED11, 100129)
  // there is still an uninitialised value/cond jump for this call??
  glXMakeCurrent(res.dpy, res.xwin, res.glc);

  // prepare this context?
  glEnable ( GL_DEPTH_TEST );
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  if (scene == NULL) {

    // initialise ssg, once and only once
    ssgInit();
    scene     = new ssgRoot();
    // model and texture paths
    if (modelpath.size()) ssgModelPath(modelpath.c_str());
    if (texturepath.size()) ssgTexturePath(texturepath.c_str());
  }

  // this is a very simple start. Might need to extend this; fog etc.
  // re-set the light for this context
  sgVec3 solposn ;
  sgSetVec3 ( solposn, 0, 0, 1000 ) ;
  ssgGetLight ( 0 ) -> setPosition ( solposn ) ;
  ssgGetLight ( 0 ) -> setColour (GL_AMBIENT,0.7f,0.7f,0.7f);
  ssgGetLight ( 0 ) -> on();

  glFlush();

  return res;
}

void PlibViewer_WindowSet::redraw(bool wait)
{
  glXMakeCurrent(dpy, xwin, glc);
  display();
  glFlush();
  if (!wait)
    glXSwapBuffers(dpy, xwin);   // swap should occur at retrace
}

void PlibViewer_WindowSet::swap()
{
  glXMakeCurrent(dpy, xwin, glc);
  glXSwapBuffers(dpy, xwin);   // swap should occur at retrace
}

void PlibViewer_WindowSet::contextCurrent(bool own)
{
  if (own)
    glXMakeCurrent(dpy, xwin, glc);
  else
    glXMakeCurrent(dpy, None, NULL);
}

void PlibViewer_WindowSet::display()
{
  glClearColor(viewer->bg_color[0], viewer->bg_color[1], 
	       viewer->bg_color[2], 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (list<PlibViewer::ViewSet>::const_iterator ii = viewsets.begin();
       ii != viewsets.end(); ii++ ) {
    
    // set the camera, etc. 
    //    double camquat[4]; QxQ(camquat, ii->angle_offset,
    //    viewer->orientation);
    double camquat[4]; QxQ(camquat, viewer->orientation, ii->angle_offset);
    double xyz[3] = { ii->rel_pos[0] + viewer->position[0], 
		      ii->rel_pos[1] + viewer->position[1], 
		      ii->rel_pos[2] + viewer->position[2]};
			 
    sgCoord campos = AxisTransform::plibCoordinates(xyz, camquat);

    ii->camera->setCamera(&campos);
    glViewport(ii->viewport[0], ii->viewport[1], // x0, y0
	       ii->viewport[2], ii->viewport[3]); // width, heigth
    ii->camera->makeCurrent();
    
    ssgCullAndDraw(viewer->scene);
    // add special drawing needs
    viewer->drawPostCall();
  }
}



void PlibViewer::init(bool waitswap)
{
  while (!winspec.empty()) {
    if (windows.find(winspec.front().name) != windows.end()) {
      cerr << "Already specified a window " << winspec.front().name 
	   << " ignoring second one" << endl;
    }
    windows[winspec.front().name] = myCreateWindow(winspec.front());
    winspec.pop_front();
  }
  
  int zorder = 0; // should do zorder per window??
  while (!viewspec.empty()) { 
    // find the appropriate window
    WindowsMap::iterator ii = windows.find(viewspec.front().winname);
    if (ii == windows.end()) {
      cerr << "Could not find window \"" << viewspec.front().winname 
	   << "\" for view \"" << viewspec.front().name << '"' << endl;
      viewspec.pop_front();
    }
    else {
      ii->second.viewsets.push_back(ViewSet());

      ii->second.viewsets.back().init
	(viewspec.front(), ii->second, scene, zorder++);
      viewspec.pop_front();
    }
  }
  
  // get first window's context for plib init
  // VERY IMPORTANT. If you continue in another thread, the context
  // you use (here for creating plib models) must be closed off!
  // otherwise this context remains blocked for the current thread
  WindowsMap::iterator cc = windows.begin();
  if (cc == windows.end()) return;
  cc->second.contextCurrent();

  // if applicable, initialize static objects
  for (auto &ob: active_objects) { ob.second->init(scene); }
  for (auto &ob: static_objects) { ob->init(scene); }

  // also initialize world objects
  for (ControllablesMap::iterator ii = controllables.begin(); 
       ii != controllables.end(); ii++) {
    PlibObject* o = dynamic_cast<PlibObject*>(ii->second.first);
    if (o) o->init(scene);
  }

  imp->glXWaitVideoSyncSGI = reinterpret_cast<PFNGLXWAITVIDEOSYNCSGIPROC>
    (glXGetProcAddress
     (reinterpret_cast<const GLubyte*>("glXWaitVideoSyncSGI")));
  imp->glXJoinSwapGroupSGIX = reinterpret_cast<PFNGLXJOINSWAPGROUPSGIXPROC>
    (glXGetProcAddress
     (reinterpret_cast<const GLubyte*>("glXJoinSwapGroupSGIX")));
  PFNGLXSWAPINTERVALSGIPROC glXSwapIntervalSGI = 
    reinterpret_cast<PFNGLXSWAPINTERVALSGIPROC>
    (glXGetProcAddress
     (reinterpret_cast<const GLubyte*>("glXSwapIntervalSGI")));
  
  // set the swap interval
  if (glXSwapIntervalSGI) {
    (*glXSwapIntervalSGI) (glx_sync_divisor);
  }

  // if multiple windows are opened, and these swap group is possible,
  // join these windows
  WindowsMap::iterator ii = windows.begin();

  // combine other windows in the same swap group with this one
  ::Display* dpy = ii->second.dpy;
  ::Window xwin = ii->second.xwin;
  if (imp->glXJoinSwapGroupSGIX) {
    while (++ii != windows.end()) {
      (*imp->glXJoinSwapGroupSGIX) (dpy, xwin, ii->second.xwin);
    }
  }

  // release window's context
  cc->second.contextCurrent(false);
}


void PlibViewer::ViewSet::init(const ViewSpec& spec,
			       const PlibViewer_WindowSet& ws,
			       ssgRoot *scene, 
			       int zorder)
{
  cout << "Creating camera \"" << spec.name
       << "\" in window \"" << ws.name << '"' << endl;

  camera    = new ssgContext();
  viewport  = spec.portcoords;
  
  if (spec.frustum_data.size() == 3) {
    double h_to_w = double(viewport[3])/double(viewport[2]);
    double fovy = spec.frustum_data[2]*M_PI/180.0;
    double fovx = atan(tan(fovy)/h_to_w);

    camera->setNearFar(spec.frustum_data[0], spec.frustum_data[1]);
    camera->setFOV(fovx*180.0/M_PI, fovy*180.0/M_PI);
  }
  else if (spec.frustum_data.size() == 6) {
    camera->setFrustum(spec.frustum_data[2], spec.frustum_data[3], 
		       spec.frustum_data[4], spec.frustum_data[5],
		       spec.frustum_data[0], spec.frustum_data[1]);
  }
  
  if (spec.eye_pos.size() >= 3) {
    rel_pos[0] = spec.eye_pos[0];
    rel_pos[1] = spec.eye_pos[1];
    rel_pos[2] = spec.eye_pos[2];
  }
  if (spec.eye_pos.size() == 6) {
    double phi = spec.eye_pos[3]*SG_DEGREES_TO_RADIANS;
    double tht = spec.eye_pos[4]*SG_DEGREES_TO_RADIANS;
    double psi = spec.eye_pos[5]*SG_DEGREES_TO_RADIANS;
    phithtpsi2Q(angle_offset, phi, tht, psi);
  }

  if (spec.overlay.size()) {
    // not implemented !
  }
}

bool PlibViewer::createControllable
(const GlobalId& master_id, const NameSet& cname, entryid_type entry_id,
 uint32_t creation_id, const std::string& data_class,
 const std::string& entry_label, Channel::EntryTimeAspect time_aspect)
{
  creation_key_t keypair(cname.name, creation_id);

  // check
  assert(active_objects.count(keypair) == 0);
  
  // create entry on the basis of data class and entry label
  PlibObject *op = NULL;
  WorldDataSpec obj = retrieveFactorySpec(data_class, entry_label, creation_id);

  // find the factory and create an object
  try {
    op = PlibObjectFactory::instance().create(obj.type, obj);
    op->connect(master_id, cname, entry_id, time_aspect);
    if (scene) {
      op->init(scene);
    }
    boost::intrusive_ptr<PlibObject> bop(op);
    active_objects[keypair] = bop;
    return true;
  }
  catch (const std::exception& problem) {
    cerr << "OgreViewer: When trying to create " << obj.type
         << " encountered: " <<  problem.what() << endl;
  }
  return false;
}

void PlibViewer::removeControllable(const NameSet& cname, uint32_t creation_id)
{
  active_objects.erase(std::make_pair(cname.name, creation_id));
}

  
void PlibViewer::redraw(bool wait, bool reset_context)
{
  // set the window number to 0 (counting bits)
  post_counter = 0;
  
  // now re-draw all windows
  for (WindowsMap::iterator ii = windows.begin(); 
       ii != windows.end(); ii++) {
    ii->second.redraw(wait);
  }
}

void PlibViewer::drawPostCall()
{
  // this will be called once for each view, after Plib drawing is
  // complete. 
  for (auto &ob: post_draw) {
    ob->draw(post_counter);
  }
  
  // post_counter keeps tab of the view number. You can use this to
  // selectively draw stuff in views
  post_counter++;
}

void PlibViewer::waitSwap()
{
  // maybe change this using glXSwapIntervalSGI on one of the windows,
  // and removing my own wait loop improves the timing

  // ask for swap on all buffers
  for (WindowsMap::iterator ii = windows.begin(); 
       ii != windows.end(); ii++) {
    ii->second.swap();
  }

  /*  unsigned int count;
  if (glXWaitVideoSyncSGI && windows.size()) {
    // make one of my contexts current
    windows.begin()->second.contextCurrent();

    int res = (*glXWaitVideoSyncSGI)(glx_sync_divisor, glx_sync_offset, &count);
    switch (res) {
    case 0:
      return;
    case GLX_BAD_VALUE:
      cerr << "error glXWaitVideoSyncSGI, bad params" << endl;
      break;
    case GLX_BAD_CONTEXT:
      cerr << "error glXWaitVideoSyncSGI, bad context" << endl;
    default:
      cerr << "error glXWaitVideoSyncSGI, " << res << endl;
    }
    glXWaitVideoSyncSGI = NULL;
    } */
}

void PlibViewer::setBase(TimeTickType tick,
                         const BaseObjectMotion& base, double late)
{
  memcpy(position, base.xyz, sizeof(position));
  memcpy(orientation, base.attitude_q, sizeof(orientation));

  // this either uses the base, a channel reader, or both, to update
  for (auto &obj: active_objects) {
    obj.second->iterate(tick, base, late);
  }
  for (auto &obj: static_objects) {
    obj->iterate(tick, base, late);
  }
}

bool PlibViewer::createStatic(const std::vector<std::string>& name)
{
  WorldDataSpec obj = retrieveFactorySpec
    (name[0], "", static_objects.size(), true);
  if (obj.type.size() == 0) {
    E_MOD("cannot find object type \"" << name[0] << "\" in the factory");
    return false;
  }

  if (name.size() >= 2) {
    obj.name = name[1];
  }
  try {
    PlibObject *op = PlibObjectFactory::instance().create(obj.type, obj);
    if (scene) {
      op->init(scene);
    }
    boost::intrusive_ptr<PlibObject> bop(op);
    static_objects.push_back(bop);
    if (bop->requirePostDrawAccess()) {
      post_draw.push_back(bop);
    }
    
    return true;
  }
  catch (const std::exception& problem) {
    cerr << "PlibViewer: When trying to create " << obj.type
         << " encountered: " <<  problem.what() << endl;
  }
  return false;
}    
 
// standard event mask
static long keysandpointer = KeyPressMask | KeyReleaseMask | ButtonPressMask | 
	     ButtonReleaseMask | PointerMotionMask | ButtonMotionMask;

bool PlibViewer::setEventMask(const std::string& window, 
			      unsigned long mask)
{
  WindowsMap::iterator ii = windows.find(window);
  if (ii != windows.end()) {
    // set the event mask on an opened window
    XSelectInput(ii->second.dpy, ii->second.xwin, mask & keysandpointer);
    
    return true;
  }

  for (std::list<WinSpec>::iterator jj = winspec.begin(); 
       jj != winspec.end(); jj++) {
    if (jj->name == window) {
      jj->mask = mask & keysandpointer;

      return true;
    }
  }

  cerr << "Cannot set event mask for window " << window << endl;
  return false;
}

const WorldViewerEvent& 
PlibViewer::getNextEvent(const std::string& window)
{
  // reset current event type
  current_event.type = WorldViewerEvent::WVNone;

  WindowsMap::iterator ii = windows.find(window);
  if (ii == windows.end()) {
    cerr << "Cannot find window for events feedback " << window << endl;
    return current_event;
  }
  
  XEvent x_event;
  Bool res = XCheckWindowEvent(ii->second.dpy, ii->second.xwin, 
			       keysandpointer, &x_event);
  if (!res) {
    return current_event;
  }

  // x and y are always there?
  switch (x_event.type) {
  case KeyPress:
  case KeyRelease:
    current_event.type = 
      (x_event.type == KeyPress) ? WorldViewerEvent::WVKeyPress : 
    WorldViewerEvent::WVKeyRelease;
    current_event.x = x_event.xkey.x;
    current_event.y = x_event.xkey.y;
    /*    current_event.code = XKeycodeToKeysym
	  (ii->second.dpy, x_event.xkey.keycode, 0); */
    current_event.code = XkbKeycodeToKeysym
      (ii->second.dpy, x_event.xkey.keycode, 0, 0);
    current_event.state = x_event.xkey.state;
    break;
  case ButtonPress:
  case ButtonRelease:
    current_event.type = 
      (x_event.type == ButtonPress) ? WorldViewerEvent::WVButtonPress : 
    WorldViewerEvent::WVButtonRelease;
    current_event.x = x_event.xbutton.x;
    current_event.y = x_event.xbutton.y;
    current_event.code = x_event.xbutton.button;
    current_event.state = x_event.xbutton.state;
    break;
  case MotionNotify:
    current_event.type = WorldViewerEvent::WVPointerMove;
    current_event.x = x_event.xmotion.x;
    current_event.y = x_event.xmotion.y;
    current_event.code = 0;
    current_event.state = x_event.xmotion.state;
    break;
  default:
    cerr << "Unhandled X type " << x_event.type << endl;
  }
  return current_event;
}
