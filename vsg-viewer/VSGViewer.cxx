/* ------------------------------------------------------------------   */
/*      item            : VSGViewer.cxx
        made by         : Rene' van Paassen
        date            : 090616
        category        : body file
        description     :
        changes         : 090616 first version
        language        : C++
*/

#define VSGViewer_cxx
#include "VSGViewer.hxx"
#include "WorldObjectBase.hxx"
#include "AxisTransform.hxx"
#include <boost/lexical_cast.hpp>
#include <unistd.h>
#include <cmath>
#include <deque>
#define W_MOD
#define E_MOD
#include <debug.h>

template<class T>
inline T deg2rad(const T d)
{
  return M_PI/180.0*d;
}

using namespace vsg;
using namespace std;

VSGViewer::ViewSet::ViewSet() :
  camera(NULL),
  view_offset()
{
  //
}



void VSGViewer::ViewSet::init(const ViewSpec& spec,
                              WindowSet& ws,
                              vsg::ref_ptr<vsg::Viewer> viewer,
                              vsg::ref_ptr<vsg::Group> root,
                              int zorder,
                              const std::vector<double>& bg_color)
{
  cout << "Creating camera " << spec.name << endl;
  name = spec.name;  

  // aspect ratio
  double aspect = double(spec.portcoords[3])/double(spec.portcoords[2]);
  
  vsg::ref_ptr<vsg::Perspective> perspective;
  if (spec.frustum_data.size() == 3) {
    // from fov, aspect, near dist, far dist
    perspective = vsg::Perspective::create
      (spec.frustum_data[2], aspect,
       spec.frustum_data[0], spec.frustum_data[1]);
  }
  else if (spec.frustum_data.size() == 6) {
    perspective = vsg::RelativeProjection::create
      (vsg::perspective
       (spec.frustum_data[2], spec.frustum_data[3],
	spec.frustum_data[4], spec.frustum_data[5],
	spec.frustum_data[0], spec.frustum_data[1]));
  }
  
  auto viewportstate = vsg::ViewportState::create
    (spec.portcoords[0],spec.portcoords[1],
     spec.portcoords[2],spec.portcoords[3]);
  
  camera = vsg::Camera::create(perspective, view_offset, viewportstate);

  // set up the projection

  // camera->setComputeNearFarMode(vsg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

  // draw in the back buffer
  //camera->setDrawBuffer(GL_BACK);
  //camera->setReadBuffer(GL_BACK);

  auto commandgraph = vsg::createCommandGraphForView(window, camera, root);
  

  if (spec.eye_pos.size() == 3) {
    view_offset.makeTranslate(AxisTransform::vsgPos(spec.eye_pos));
  }
  else if (spec.eye_pos.size() == 6) {
    vsg::Matrixd trans;
    trans.makeTranslate(AxisTransform::vsgPos(spec.eye_pos));
    vsg::Matrixd rot = AxisTransform::vsgRotation
       (spec.eye_pos[3], spec.eye_pos[4], spec.eye_pos[5]);
    view_offset = trans * rot;
  }
  vsg::Matrixd viewinv = view_offset.inverse(view_offset);
  camera->setViewMatrix(viewinv);

  if (spec.overlay.size()) {
    cout << "Looking for overlay " << spec.overlay << endl;
    // not implemented
  }

}

VSGViewer::VSGViewer() :
  winspec(),
  root(NULL),
  base_gc(NULL),
  oviewer(NULL),
  cviewer(NULL),
  config_dynamic_created(0),
  allow_unknown(false),
  windows(),
  active_objects(),
  static_objects(),
  post_draw(),
  viewspec(),
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


VSGViewer::~VSGViewer()
{

}

/** Quick exception struct. */
struct DuecaVSGConfigError: public std::exception
{
  /** Say what is the problem */
  const char* what() { return "Configuration error VSG"; }
};

// to simplify programming
inline VSGViewer::WindowSet
VSGViewer::myCreateWindow(const WinSpec &ws, vsg::ref_ptr<vsg::Group> root)
{
  // result, to be returned
  WindowSet res;
  res.name = ws.name;

  // check windowing system
  vsg::GraphicsContext::WindowingSystemInterface* wsi =
    vsg::GraphicsContext::getWindowingSystemInterface();
  if (!wsi) {
    cerr << "cannot access windowing system" << endl;
    throw (DuecaVSGConfigError());
  }

  // How many screens?
  // vsg::GraphicsContext::ScreenIdentifier si;
  res.traits->readDISPLAY();
  if (!wsi->getNumScreens(*(res.traits))) {
    cerr << "No screens available" << endl;
    throw (DuecaVSGConfigError());
  }
  //cerr << "number of screens " << wsi->getNumScreens(*(res.traits));
  res.traits->setUndefinedScreenDetailsToDefaultScreen();

  // get screen size
  vsg::GraphicsContext::ScreenSettings sdata;
  wsi->getScreenSettings(*(res.traits), sdata);
  //  unsigned int width, height;
  // wsi->getScreenResolution(vsg::GraphicsContext::ScreenIdentifier(0),
  //                       width, height);

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
  res.gc = vsg::GraphicsContext::createGraphicsContext(res.traits);
  if (base_gc == NULL) {
    base_gc = res.gc;
  }

  if (res.gc.valid()) {
    res.gc->setClearColor
      (vsg::Vec4f(bg_color[0],bg_color[1],bg_color[2],
                  bg_color.size() == 4 ? bg_color[3] : 1.0f));
    res.gc->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  } else {
    cerr << "cannot create window \"" << ws.name.c_str() << '"' << endl;
    throw (DuecaVSGConfigError());
  }

  return res;
}

namespace dueca {
  extern int* p_argc;
  extern char*** p_argv;
}

void VSGViewer::init(bool waitswap)
{
  // create root
  root = vsg::Group::create();
  options = vsg::Options::create();
  options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
  options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
  options->add(vsgXchange::all::create());

  // create viewer
  oviewer = vsg::Viewer::create();
  oviewer->setCameraManipulator(NULL);

  // create windows
  if (winspec.empty()) {
    WinSpec window;
    window.name = "DUECA/VSG default window";
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
    windows[winspec.front().name] = myCreateWindow(winspec.front(), root);
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

      // init view
      ii->second.viewset[viewspec.front().name].init
        (viewspec.front(), ii->second, cviewer, oviewer,
         root, zorder++, bg_color, cb);
    }
    viewspec.pop_front();
  }

  // if applicable, initialize static objects and dynamic objects
  for (auto &ao: active_objects) { ao.second->init(root, this); }
  for (auto &so: static_objects) { so->init(root, this); }

  // set fog parameters on the root
  if (fog_mode != Off) {
    vsg::Fog* fog = new vsg::Fog();
    switch(fog_mode) {
    case Off:
    case Linear:
      fog->setMode(vsg::Fog::LINEAR);
      break;
    case Exponential:
      fog->setMode(vsg::Fog::EXP);
      break;
    case Exponential2:
      fog->setMode(vsg::Fog::EXP2);
      break;
    }
    fog->setDensity(fog_density);
    fog->setColor(fog_colour);
    fog->setStart(fog_start);
    fog->setEnd(fog_end);
    vsg::StateSet* rootstate = root->getOrCreateStateSet();
    rootstate->setAttribute(fog, vsg::StateAttribute::ON);
    rootstate->setMode(GL_FOG, vsg::StateAttribute::ON);
    //root->setStateSet(rootstate);
  }

  // add it all to the viewer
  vsgUtil::Optimizer optimizer;
  optimizer.optimize(root);

  oviewer->setSceneData(root);
  oviewer->setThreadingModel(vsg::Viewer::SingleThreaded);
  oviewer->setReleaseContextAtEndOfFrameHint(true);
  oviewer->realize();
}

void VSGViewer::addViewport(const ViewSpec& vp)
{
  viewspec.push_back(vp);
}

void VSGViewer::redraw(bool wait, bool reset_context)
{
  oviewer->frame();
}

void VSGViewer::waitSwap()
{
#if 0
  WindowsMap::const_iterator ii = windows.begin();
  if (ii == windows.end()) {
    // strange, no windows to swap
    return;
  }

  // wait for vsync and swap the first buffer
  ii->second.window->swapBuffers(true); ii++;

  // now quickly do the others, let's hope we are real-time enough
  for ( ; ii != windows.end(); ii++) {
    ii->second.window->swapBuffers(false);
  }
#endif
}

template <typename T>
inline static void updateTransform(vsg::Node* tf, const T& v)
{
  vsg::PositionAttitudeTransform* t =
    dynamic_cast<vsg::PositionAttitudeTransform*>(tf);
  if (t == NULL) return;
  if (v.size() >= 3) {
    t->setPosition(AxisTransform::vsgPos(v.data()));
  }
  if (v.size() >= 7) {
    t->setAttitude(AxisTransform::vsgQuat(v.data()+3));
  }
  if (v.size() >= 10) {
    t->setScale(AxisTransform::vsgScale(v.data()+7));
  }
}


bool VSGViewer::adaptSceneGraph(const WorldViewConfig& adapt)
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
      W_MOD("VSGViewer " << adapt.command << " is not implemented");
    }
  }
  catch (const std::exception& e) {
    W_MOD("VSGViewer caught " << e.what());
    return false;
  }
  return true;
}

void VSGViewer::setBase(TimeTickType tick, const BaseObjectMotion& ownm,
                        double late)
{
  // \todo: modify to update all cameras, as they are in the viewset list

  // get the world position of the base point in VSG space
  vsg::Matrixd base;
  base.makeTranslate(ownm.xyz[1], ownm.xyz[0], -ownm.xyz[2]);

  // rotate the camera according to view angles
  vsg::Matrixd camrot(AxisTransform::vsgQuat(ownm.attitude_q));

  static vsg::Matrixd projectorrot =
    vsg::Matrixd::rotate(M_PI*0.5, vsg::Vec3(1,0,0));

  vsg::Matrixd total = projectorrot * camrot * base;
  vsg::Matrixd inverse = total.inverse(total);

  // set the master camera only; slave cameras will follow suit
  oviewer->getCamera()->setViewMatrix(inverse);

  for (auto &obj : active_objects) {
    obj.second->iterate(tick, ownm, late);
  }
}

bool VSGViewer::createControllable
(const GlobalId& master_id, const NameSet& cname, entryid_type entry_id,
 uint32_t creation_id, const std::string& data_class,
 const std::string& entry_label, Channel::EntryTimeAspect time_aspect)
{
  creation_key_t keypair(cname.name, creation_id);

  // check we don't have this one yet
  assert(active_objects.count(keypair) == 0);

  // not found, create entry on the basis of data class and entry label
  VSGObject *op = NULL;
  WorldDataSpec obj;

  try {
    obj = retrieveFactorySpec(data_class, entry_label, creation_id);

    op = VSGObjectFactory::instance().create(obj.type, obj);
    op->connect(master_id, cname, entry_id, time_aspect);
    if (root) {
      op->init(root, this);
    }
    boost::intrusive_ptr<VSGObject> bop(op);
    active_objects[keypair] = bop;
    return true;
  }
  catch (const CFCannotMake& problem) {
    if (!allow_unknown) {
      W_MOD("VSGViewer: factory cannot create for " << data_class <<
            " encountered: " <<  problem.what());
      throw(problem);
    }
    W_MOD("VSGViewer: factory cannot create for " << data_class <<
          ", ignoring channel " << cname << " entry " << entry_id);
  }
  catch (const MapSpecificationError& problem) {
    if (!allow_unknown) {
      W_MOD("VSGViewer: not configured for " << data_class <<
            " encountered: " <<  problem.what());
      throw(problem);
    }
    W_MOD("VSGViewer: not configured for " << data_class <<
          ", ignoring channel " << cname << " entry " << entry_id);
  }
  catch (const std::exception& problem) {
    cerr << "VSGViewer: When trying to create for " << data_class
         << " encountered: " <<  problem.what() << endl;
  }
  return false;
}

bool VSGViewer::createStatic(const std::vector<std::string>& name)
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
    VSGObject* op =
      VSGObjectFactory::instance().create(obj.type, obj);
    if (root) { op->init(root, this); }
    boost::intrusive_ptr<VSGObject> bop(op);
    static_objects.push_back(bop);
    return true;
  }
  catch (const std::exception& problem) {
    cerr << "VSGViewer: When trying to create " << obj.type
         << " encountered: " <<  problem.what() << endl;
  }
  return false;
}

void VSGViewer::removeControllable(const NameSet& cname, uint32_t creation_id)
{
  active_objects[std::make_pair(cname.name, creation_id)].reset();
}


vsg::ref_ptr<vsg::Camera>
VSGViewer::getMainCamera(const std::string& wname,
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
