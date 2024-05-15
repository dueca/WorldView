/* ------------------------------------------------------------------   */
/*      item            : OSGViewer.cxx
        made by         : Rene' van Paassen
        date            : 090616
        category        : body file
        description     :
        changes         : 090616 first version
        language        : C++
*/

#define OSGViewer_cxx
#include "OSGViewer.hxx"
#include "WorldObjectBase.hxx"
#include "AxisTransform.hxx"
#include <osgDB/FileUtils>
#include <osg/Fog>
#include <osgUtil/Optimizer>
#include <boost/lexical_cast.hpp>
#include <unistd.h>
#include <cmath>
#include <deque>
#include <dueca/Condition.hxx>
#define W_MOD
#define E_MOD
#include <debug.h>
#define DEBPRINTLEVEL 2
#include <debprint.h>


namespace {
  template<class T>
  inline T rad2deg(const T a)
  { return 180.0/M_PI*a; }

  template<class T>
  inline T deg2rad(const T d)
  { return M_PI/180.0*d; }
};

using namespace osg;
using namespace std;

OSGViewer::ViewSet::ViewSet() :
  camera(NULL),
  view_offset()
{
  //
}



void OSGViewer::ViewSet::init(const ViewSpec& spec,
                              WindowSet& ws,
                              osg::ref_ptr<osgViewer::CompositeViewer> cviewer,
                              osg::ref_ptr<osg::Group> root,
                              int zorder,
                              const std::vector<double>& bg_color,
                              osg::Camera::Camera::DrawCallback *cb)
{
  cout << "Creating camera " << spec.name << endl;
  name = spec.name;
  frustum_data = spec.frustum_data;

  // create this view
  sview = new osgViewer::Viewer;
  sview->setThreadingModel(osgViewer::Viewer::SingleThreaded);
  sview->setReleaseContextAtEndOfFrameHint(true);
  sview->setSceneData(root);
  sview->setCameraManipulator(NULL);

  // create a camera for the view
  camera = sview->getCamera(); // new osg::Camera;

  camera->setGraphicsContext(ws.gc);
  camera->setClearColor
    (osg::Vec4f(bg_color[0],bg_color[1],bg_color[2],
                bg_color.size() == 4 ? bg_color[3] : 1.0f));

  // set up view. If no data given, take full window size
  aspect = 1.0f;
  if (spec.portcoords.size() == 4) {
    camera->setViewport
      (new osg::Viewport
       (spec.portcoords[0],spec.portcoords[1],
        spec.portcoords[2],spec.portcoords[3]));
    aspect = double(spec.portcoords[3]) / double(spec.portcoords[2]);
  }
  else {
    camera->setViewport
      (new osg::Viewport
       (0, 0, ws.traits->width, ws.traits->height));
    aspect = double(ws.traits->height)/double(ws.traits->width);
  }

  setProjection();

  // add any callback
  if (cb) camera->setPostDrawCallback(cb);

  // if applicable, add to the composite viewer
  if (cviewer) {
    cviewer->addView(sview);
  }


  // set the view offset (position, angle) initial, and also for
  // composite view slaving
  if (spec.eye_pos.size() == 3) {
    view_offset.makeTranslate(AxisTransform::osgPos(spec.eye_pos));
  }
  else if (spec.eye_pos.size() == 6) {
    osg::Matrixd trans;
    trans.makeTranslate(AxisTransform::osgPos(spec.eye_pos));
    osg::Matrixd rot = AxisTransform::osgRotation
      (deg2rad(spec.eye_pos[3]),
       deg2rad(spec.eye_pos[4]),
       deg2rad(spec.eye_pos[5]));
    view_offset = trans * rot;
  }
  else {
    view_offset.makeIdentity();
  }
  osg::Matrixd viewinv = view_offset.inverse(view_offset);
  camera->setViewMatrix(viewinv);

  if (spec.overlay.size()) {
    cout << "Looking for overlay " << spec.overlay << endl;
    // not implemented
  }
  //sview->addSlave(camera);
  sview->realize();
}

std::ostream& operator << (std::ostream& os, const osg::Matrixd& m)
{
  os <<   '[' << m(0,0) << "," << m(0,1) << "," << m(0,2) << "," << m(0,3)
     << "\n " << m(1,0) << "," << m(1,1) << "," << m(1,2) << "," << m(1,3)
     << "\n " << m(2,0) << "," << m(2,1) << "," << m(2,2) << "," << m(2,3)
     << "\n " << m(3,0) << "," << m(3,1) << "," << m(3,2) << "," << m(3,3)
     << "]";
  return os;
}


void OSGViewer::ViewSet::setProjection()
{
  DEB("default projection " << camera->getProjectionMatrix());

  // set up the projection
  if (frustum_data.size() == 3) {
    camera->setProjectionMatrixAsPerspective
      (rad2deg(frustum_data[2]), aspect,
       frustum_data[0], frustum_data[1]);
  }
  else if (frustum_data.size() == 6) {
    osg::Matrixd f;
    f.makeFrustum(frustum_data[2], frustum_data[3],
		  frustum_data[4], frustum_data[5],
		  frustum_data[0], frustum_data[1]);
    camera->setProjectionMatrix(f);

    // left, right, bottom, top, near, far
    //camera->setProjectionMatrixAsFrustum
    //  (frustum_data[2], frustum_data[3],
    //   frustum_data[4], frustum_data[5],
    //   frustum_data[0], frustum_data[1]);
  }
  DEB("new projection " << camera->getProjectionMatrix());
}

OSGViewer::OSGViewer() :
  winspec(),
  root(NULL),
  base_gc(NULL),
  config_dynamic_created(0),
  use_compositeviewer(false),
  allow_unknown(false),
  windows(),
  active_objects(),
  static_objects(),
  post_draw(),
  viewspec(),
  global_draw_callback(NULL),
  draw_callbacks(),
  resourcepath(),
  keep_pointer(false),
  bg_color(3, 0.0),
  fog_mode(Off),
  fog_density(0.0),
  fog_colour(1.0, 1.0, 1.0, 0.001),
  fog_start(10000.0),
  fog_end(100000.0)
{
  // root is created upon window init
}


OSGViewer::~OSGViewer()
{

}

/** Quick exception struct. */
struct DuecaOSGConfigError: public std::exception
{
  /** Say what is the problem */
  const char* what() { return "Configuration error OSG"; }
};

// to simplify programming
inline OSGViewer::WindowSet
OSGViewer::myCreateWindow(const WinSpec &ws, osg::ref_ptr<osg::Group> root,
                          osg::ArgumentParser& arguments)
{
  // result, to be returned
  WindowSet res;
  res.name = ws.name;
  res.traits = new osg::GraphicsContext::Traits;

  // check windowing system
  osg::GraphicsContext::WindowingSystemInterface* wsi =
    osg::GraphicsContext::getWindowingSystemInterface();
  if (!wsi) {
    cerr << "cannot access windowing system" << endl;
    throw (DuecaOSGConfigError());
  }

  // How many screens?
  // osg::GraphicsContext::ScreenIdentifier si;
  res.traits->readDISPLAY();
  if (!wsi->getNumScreens(*(res.traits))) {
    cerr << "No screens available" << endl;
    throw (DuecaOSGConfigError());
  }
  //cerr << "number of screens " << wsi->getNumScreens(*(res.traits));
  res.traits->setUndefinedScreenDetailsToDefaultScreen();

  // get screen size
  osg::GraphicsContext::ScreenSettings sdata;
  wsi->getScreenSettings(*(res.traits), sdata);

  // set the window
  res.traits->windowName = ws.name;
  res.traits->doubleBuffer = true;
  res.traits->vsync = true;
  res.traits->useCursor = false;

  // Full screen?
  if (ws.size_and_position.size() == 0) {
    res.traits->x = 0; res.traits->y = 0;
    res.traits->width = sdata.width;
    res.traits->height = sdata.height;
    res.traits->windowDecoration = false;
  }
  else {
    // Check for position information
    if (ws.size_and_position.size() == 4) {
      res.traits->x = ws.size_and_position[2];
      res.traits->y = ws.size_and_position[3];
    }
    res.traits->width = ws.size_and_position[0];
    res.traits->height = ws.size_and_position[1];
    res.traits->windowDecoration = true;
    res.traits->supportsResize = true;
  }
  res.traits->sharedContext = base_gc;
  res.gc = osg::GraphicsContext::createGraphicsContext(res.traits);
  if (base_gc == NULL) {
    base_gc = res.gc;
  }

  if (res.gc.valid()) {
    res.gc->setClearColor
      (osg::Vec4f(bg_color[0],bg_color[1],bg_color[2],
                  bg_color.size() == 4 ? bg_color[3] : 1.0f));
    res.gc->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  } else {
    cerr << "cannot create window \"" << ws.name.c_str() << '"' << endl;
    throw (DuecaOSGConfigError());
  }

  if (use_compositeviewer) {
    res.cviewer = new osgViewer::CompositeViewer(arguments);
    res.cviewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    res.cviewer->setReleaseContextAtEndOfFrameHint(true);
    res.cviewer->realize();
  }

  return res;
}

namespace dueca {
  extern int* p_argc;
  extern char*** p_argv;
}

struct MySwapCb: public osg::GraphicsContext::SwapCallback
{
  MySwapCb() : osg::GraphicsContext::SwapCallback()
  {
    //
  }

  /** Callback from the swap */
  void swapBuffersImplementation(osg::GraphicsContext* gc) final
  {
    // DEB("Swap callback");

    gc->swapBuffersImplementation();
#if 0
    unsigned int counter;
    glXGetVideoSyncSGI(&counter);
    glXWaitVideoSyncSGI(1, 0, &counter);
#else
    //glFinish();
#endif
  }
};

void OSGViewer::init(bool waitswap)
{
  // create root
  root = new osg::Group();

  // set/extend resource path
  osgDB::FilePathList filepath;
  osgDB::convertStringPathIntoFilePathList(resourcepath, filepath);
  osgDB::appendPlatformSpecificResourceFilePaths(filepath);
  filepath.insert(filepath.begin(), osgDB::getDataFilePathList().begin(),
                  osgDB::getDataFilePathList().end());
  osgDB::setDataFilePathList(filepath);

  // parse arguments, for later
  osg::ArgumentParser arguments(p_argc, *p_argv);

  // create windows
  if (winspec.empty()) {
    WinSpec window;
    window.name = "DUECA/OSG default window";
    float size_and_position[] = { 400, 300};
    window.size_and_position.resize(2);
    copy(&size_and_position[0], &size_and_position[2],
         window.size_and_position.begin());
    window.display = "";
    addWindow(window);
  }

  while (!winspec.empty()) {
    if (windows.find(winspec.front().name) != windows.end()) {
      cerr << "Already specified a window " << winspec.front().name
           << " ignoring second one" << endl;
    }
    windows[winspec.front().name] = myCreateWindow(winspec.front(), root,
                                                   arguments);
    winspec.pop_front();
  }

  // create cameras and viewports
  int zorder = 0; // should do zorder per window??
  while (!viewspec.empty()) {
    // find the appropriate window
    WindowsMap::iterator ii = windows.find(viewspec.front().winname);
    if (ii == windows.end()) {
      E_MOD("Could not find window \"" << viewspec.front().winname
            << "\" for view \"" << viewspec.front().name << '"');
    }
    else if (ii->second.viewset.count(viewspec.front().name)) {
      E_MOD("Already have view \"" << viewspec.front().name <<
            "\" in window \"" << viewspec.front().winname << '"');
    }
    else {
      ii->second.viewset[viewspec.front().name] = ViewSet();

      // find specific callback or fall back on global callback (if any)
      osg::Camera::Camera::DrawCallback *cb = NULL;
      map<string, osg::Camera::Camera::DrawCallback *>::iterator cb_iter =
        draw_callbacks.find(viewspec.front().name);
      if(cb_iter != draw_callbacks.end()) {
        cb = cb_iter->second;
      } else { cb = global_draw_callback; }

      // init view
      ii->second.viewset[viewspec.front().name].init
        (viewspec.front(), ii->second, ii->second.cviewer,
         root, zorder++, bg_color, cb);
    }
    viewspec.pop_front();
  }

  // imperfect, but set swap cb on first window gc
  //#define OSG_DOES_NOT_WAIT_ON_X11_SWAP
#ifdef OSG_DOES_NOT_WAIT_ON_X11_SWAP
  if (waitswap) {
    swapcb = new MySwapCb;
    windows.begin()->second.gc->setSwapCallback(swapcb);
  }
#endif

  // if applicable, initialize static objects and dynamic objects
  for (auto &ao: active_objects) { ao.second->init(root, this); }
  for (auto &so: static_objects) { so->init(root, this); }

  // set fog parameters on the root
  if (fog_mode != Off) {
    osg::Fog* fog = new osg::Fog();
    switch(fog_mode) {
    case Off:
    case Linear:
      fog->setMode(osg::Fog::LINEAR);
      break;
    case Exponential:
      fog->setMode(osg::Fog::EXP);
      break;
    case Exponential2:
      fog->setMode(osg::Fog::EXP2);
      break;
    }
    fog->setDensity(fog_density);
    fog->setColor(fog_colour);
    fog->setStart(fog_start);
    fog->setEnd(fog_end);
    osg::StateSet* rootstate = root->getOrCreateStateSet();
    rootstate->setAttribute(fog, osg::StateAttribute::ON);
    rootstate->setMode(GL_FOG, osg::StateAttribute::ON);
  }

  // optimize the root tree
  osgUtil::Optimizer optimizer;
  optimizer.optimize(root);


}

void OSGViewer::addViewport(const ViewSpec& vp)
{
  viewspec.push_back(vp);
}

void OSGViewer::redraw(bool wait, bool reset_context)
{
  if (use_compositeviewer) {
    for (auto &ws: windows) {
      ws.second.cviewer->frame();
    }
  }
  else {
    for (auto &ws: windows) {
      for (auto &vs: ws.second.viewset) {
        vs.second.sview->frame();
      }
    }
  }
}

void OSGViewer::waitSwap()
{
  //if (swapcb) swapcb->waitForSwap();
}

template <typename T>
inline static void updateTransform(osg::Node* tf, const T& v)
{
  osg::PositionAttitudeTransform* t =
    dynamic_cast<osg::PositionAttitudeTransform*>(tf);
  if (t == NULL) return;
  if (v.size() >= 3) {
    t->setPosition(AxisTransform::osgPos(v.data()));
  }
  if (v.size() >= 7) {
    t->setAttitude(AxisTransform::osgQuat(v.data()+3));
  }
  if (v.size() >= 10) {
    t->setScale(AxisTransform::osgScale(v.data()+7));
  }
}


bool OSGViewer::adaptSceneGraph(const WorldViewConfig& adapt)
{
  try {

    switch (adapt.command) {

    case WorldViewConfig::ClearModels: {

      // to be updated, only remove static objects
      for (auto &so: static_objects) {
        so->unInit(root);
      }
      static_objects.clear();
    }
      break;

    case WorldViewConfig::RemoveNode: {
      for (int ii = root->getNumChildren(); ii--; ) {
        if (root->getChild(ii)->getName() == adapt.config.name) {
          root->removeChild(ii);
        }
      }
    }
      break;

    case WorldViewConfig::LoadObject: {

      // creates and adds a specific configuration
      std::string dclass = "on-the-fly-object_" +
        boost::lexical_cast<std::string>(++config_dynamic_created);
      this->addFactorySpec(dclass, adapt.config);

      // run the createStatic call to create the object
      std::vector<std::string> createconf;
      createconf.push_back(dclass);
      this->createStatic(createconf);
    }
      break;

    case WorldViewConfig::MoveObject: {
      for (int ii = root->getNumChildren(); ii--; ) {
        if (root->getChild(ii)->getName() == adapt.config.name) {
          updateTransform(root->getChild(ii), adapt.config.coordinates);
        }
      }
    }
      break;
    case WorldViewConfig::ListNodes:
    case WorldViewConfig::LoadOverlay:
    case WorldViewConfig::RemoveOverlay:
    case WorldViewConfig::ReadScene:
      W_MOD("OSGViewer " << adapt.command << " is not implemented");
    }
  }
  catch (const std::exception& e) {
    W_MOD("OSGViewer caught " << e.what());
    return false;
  }
  return true;
}

void OSGViewer::setBase(TimeTickType tick, const BaseObjectMotion& ownm,
                        double late, bool freeze)
{
  // \todo: modify to update all cameras, as they are in the viewset list

  // get the world position of the base point in OSG space
  osg::Matrixd base;
  base.makeTranslate(ownm.xyz[1], ownm.xyz[0], -ownm.xyz[2]);

  // rotate the camera according to view angles
  osg::Matrixd camrot(AxisTransform::osgQuat(ownm.attitude_q));

  static osg::Matrixd projectorrot =
    osg::Matrixd::rotate(M_PI*0.5, osg::Vec3(1,0,0));

  // RvP, add the eye rotation to this
  osg::Matrixd total = projectorrot * camrot * base;
  osg::Matrixd inverse = total.inverse(total);

  // set the master camera only; slave cameras will follow suit
  if (use_compositeviewer) {
    for (auto &ws: windows) {
      total = projectorrot * camrot * base;
      osgViewer::ViewerBase::Cameras cams;
      ws.second.cviewer->getCameras(cams);
      for (auto &cam: cams) { cam->setViewMatrix(inverse); }
    }
  }
  else {
    for (auto &ws: windows) {
      for (auto &vs: ws.second.viewset) {
        total = projectorrot * vs.second.view_offset * camrot * base;
        inverse = total.inverse(total);
	// vs.second.setProjection();
        vs.second.sview->getCamera()->setViewMatrix(inverse);
	//DEB("new projection " <<
	//    vs.second.sview->getCamera()->getProjectionMatrix());
      }
    }
  }

  for (auto &obj : active_objects) {
    obj.second->iterate(tick, ownm, late);
  }
}

bool OSGViewer::createControllable
(const GlobalId& master_id, const NameSet& cname, entryid_type entry_id,
 uint32_t creation_id, const std::string& data_class,
 const std::string& entry_label, Channel::EntryTimeAspect time_aspect)
{
  creation_key_t keypair(cname.name, creation_id);

  // check we don't have this one yet
  assert(active_objects.count(keypair) == 0);

  // not found, create entry on the basis of data class and entry label
  OSGObject *op = NULL;
  WorldDataSpec obj;

  try {
    obj = retrieveFactorySpec(data_class, entry_label, creation_id);

    op = OSGObjectFactory::instance().create(obj.type, obj);
    op->connect(master_id, cname, entry_id, time_aspect);
    if (root) {
      op->init(root, this);
    }
    boost::intrusive_ptr<OSGObject> bop(op);
    active_objects[keypair] = bop;
    return true;
  }
  catch (const CFCannotMake& problem) {
    if (!allow_unknown) {
      W_MOD("OSGViewer: factory cannot create for " << data_class <<
            " encountered: " <<  problem.what());
      throw(problem);
    }
    W_MOD("OSGViewer: factory cannot create for " << data_class <<
          ", ignoring channel " << cname << " entry " << entry_id);
  }
  catch (const MapSpecificationError& problem) {
    if (!allow_unknown) {
      W_MOD("OSGViewer: not configured for " << data_class <<
            " encountered: " <<  problem.what());
      throw(problem);
    }
    W_MOD("OSGViewer: not configured for " << data_class <<
          ", ignoring channel " << cname << " entry " << entry_id);
  }
  catch (const std::exception& problem) {
    cerr << "OSGViewer: When trying to create for " << data_class
         << " encountered: " <<  problem.what() << endl;
  }
  return false;
}

bool OSGViewer::createStatic(const std::vector<std::string>& name)
{
  if (!name.size()) {
    E_MOD("Need to specify type of static object");
    return false;
  }

  auto obj = retrieveFactorySpec(name[0], "", static_objects.size(), true);
  if (obj.type.size() == 0) {
    E_MOD("Cannot find object type \"" << name[0] << "\" in the factory");
    return false;
  }

  if (name.size() >= 2) {
    obj.name = name[1];
  }
  try {
    OSGObject* op =
      OSGObjectFactory::instance().create(obj.type, obj);
    if (root) { op->init(root, this); }
    boost::intrusive_ptr<OSGObject> bop(op);
    static_objects.push_back(bop);
    return true;
  }
  catch (const std::exception& problem) {
    cerr << "OSGViewer: When trying to create " << obj.type
         << " encountered: " <<  problem.what() << endl;
  }
  return false;
}

void OSGViewer::removeControllable(const NameSet& cname, uint32_t creation_id)
{
  auto idx = active_objects.find(std::make_pair(cname.name, creation_id));
  idx->second->unInit(root);
  active_objects.erase(std::make_pair(cname.name, creation_id));
}


osg::ref_ptr<osg::Camera>
OSGViewer::getMainCamera(const std::string& wname,
                         const std::string& vname)
{
  auto winset = windows.find(wname);
  if (winset == windows.end()) {
    W_MOD("No window \"" << wname << '"');
    return NULL;
  }
  auto view = winset->second.viewset.find(vname);
  if (view == winset->second.viewset.end()) {
    W_MOD("No view \"" << vname << '"');
    return NULL;
  }
  return view->second.camera;
}

void OSGViewer::setDrawCallback(const string& view_spec_name,
				osg::Camera::Camera::DrawCallback *cb)
{
  draw_callbacks[view_spec_name] = cb;
}

void OSGViewer::installPostDrawCallback(osg::Camera::Camera::DrawCallback *cb,
                                        const std::string& viewname)
{
  if (viewname.size()) {
    // when windows and views have already been created ...
    if (windows.size()) {
      for (auto w: windows) {
        for (auto v: w.second.viewset) {
          if (v.first == viewname) {
            if (v.second.camera->getPostDrawCallback()) {
              W_MOD("Window " << w.second.name << " view " << v.second.name <<
                    " already has a post-draw callback, ignoring new one");
            }
            else {
              v.second.camera->setPostDrawCallback(cb);
            }
          }
        }
      }
    }
    else {
      draw_callbacks[viewname] = cb;
    }
  }
  else {
    if (windows.size()) {
      for (auto w: windows) {
        for (auto v: w.second.viewset) {
          if (v.second.camera->getPostDrawCallback()) {
              W_MOD("Window " << w.second.name << " view " << v.second.name <<
                    " already has a post-draw callback, ignoring new one");
          }
          else {
            v.second.camera->setPostDrawCallback(cb);
          }
        }
      }
    }
    else {
      global_draw_callback = cb;
    }
  }
}
