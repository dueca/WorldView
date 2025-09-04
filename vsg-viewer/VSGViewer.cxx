/* ------------------------------------------------------------------   */
/*      item            : VSGViewer.cxx
        made by         : Rene' van Paassen
        date            : 090616
        category        : body file
        description     :
        changes         : 090616 first version
        language        : C++
*/

#include "VSGPBRShaderSet.hxx"
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

namespace dueca {
extern int *p_argc;
extern char ***p_argv;
} // namespace dueca

namespace vsg {
  /** Could not initially find one? Frustum projection for VSG */
class VSG_DECLSPEC FrustumPerspective :
  public Inherit<ProjectionMatrix, FrustumPerspective>
{
public:
  FrustumPerspective() :
    left(-0.5),
    right(0.5),
    bottom(-0.5),
    top(0.5),
    nearDistance(1.0),
    farDistance(10000.0)
  {}

  FrustumPerspective(double left, double right, double bottom, double top,
                     double nd, double fd) :
    left(left),
    right(right),
    bottom(bottom),
    top(top),
    nearDistance(nd),
    farDistance(fd)
  {}

  dmat4 transform() const override
  {
    return perspective(left, right, bottom, top, nearDistance, farDistance);
  }

  void changeExtent(const VkExtent2D &, const VkExtent2D &newExtent) override
  {
      // no meaningful way to implement this
  }

  void read(Input &input) override
  {
    ProjectionMatrix::read(input);
    input.read("left", left);
    input.read("right", right);
    input.read("bottom", bottom);
    input.read("top", top);
    input.read("nearDistance", nearDistance);
    input.read("farDistance", farDistance);
  }
  void write(Output &output) const override
  {
    ProjectionMatrix::write(output);
    output.write("left", left);
    output.write("right", right);
    output.write("bottom", bottom);
    output.write("top", top);
    output.write("nearDistance", nearDistance);
    output.write("farDistance", farDistance);
  }

  double left;
  double right;
  double bottom;
  double top;
  double nearDistance;
  double farDistance;
};
VSG_type_name(vsg::FrustumPerspective);
} // namespace vsg

namespace vsgviewer {

template <class T> inline T deg2rad(const T d) { return M_PI / 180.0 * d; }

using namespace vsg;
using namespace std;

VSGViewer::ViewSet::ViewSet()
{
    //
}

void VSGViewer::ViewSet::init(const ViewSpec &spec, WindowSet &ws,
                              vsg::ref_ptr<vsg::Viewer> viewer,
                              vsg::ref_ptr<vsg::Group> root,
                              const std::vector<float> &bg_color,
                              vsg::ref_ptr<vsg::Options> options)
{
  cout << "Creating camera " << spec.name << endl;
  name = spec.name;

    // aspect ratio
  double aspect = double(spec.portcoords[3]) / double(spec.portcoords[2]);

    // perspective transformation matrix
  vsg::ref_ptr<vsg::ProjectionMatrix> perspective;

    // option 1, fov, aspect, ...
  if (spec.frustum_data.size() == 3) {
      // from fov, aspect, near dist, far dist
    perspective =
      vsg::Perspective::create(spec.frustum_data[2], 1 / aspect,
                               spec.frustum_data[0], spec.frustum_data[1]);
  }

    // option 2, as frustum
  else if (spec.frustum_data.size() == 6) {
    perspective = vsg::FrustumPerspective::create(
      spec.frustum_data[2], spec.frustum_data[3], spec.frustum_data[4],
      spec.frustum_data[5], spec.frustum_data[0], spec.frustum_data[1]);
  }

    // viewport given
  auto viewportstate =
    vsg::ViewportState::create(spec.portcoords[0], spec.portcoords[1],
                               spec.portcoords[2], spec.portcoords[3]);

    // the view matrix transforms to the camera position. At this stage,
    // assume starting at origin, only component is the eye offset
  view_matrix = vsg::LookAt::create();
  if (spec.eye_pos.size() == 0) {
      // no eye offset, this creates a diagonal/unit matrix
    eye_offset = vsg::t_mat4<double>();
  }
  else if (spec.eye_pos.size() >= 3) {
    eye_offset = vsg::translate(vsgPos(vrange(spec.eye_pos, 0, 3)));
  }
  if (spec.eye_pos.size() == 6) {
    eye_offset =
      vsg::rotate(double(vsg::radians(spec.eye_pos[3])), 0.0, 0.0, 1.0) *
      vsg::rotate(-double(vsg::radians(spec.eye_pos[4])), 1.0, 0.0, 0.0) *
      vsg::rotate(double(vsg::radians(spec.eye_pos[5])), 0.0, 1.0, 0.0) *
      eye_offset;
  }
    // initial position and angle
  view_matrix->set(eye_offset);

    // create camera and view
  camera = vsg::Camera::create(perspective, view_matrix, viewportstate);
  view = vsg::View::create(camera, root);

    // render graph seems to be the command structure that is called when
    // window needs to refres views
#ifdef RG_PER_VIEWSET
  render_graph = vsg::RenderGraph::create(ws.window, view);
  render_graph->clearValues[0].color = { { bg_color[0], bg_color[1],
                                           bg_color[2], bg_color[3] } };
  ws.command_graph->addChild(render_graph);
#else
  ws.render_graph->addChild(view);
#endif

    // is there a mask overlay?
  if (spec.overlay.size()) {

    // clear the depth buffer before view2 gets rendered ??
    VkClearValue clearValue{};
    clearValue.depthStencil = { 0.0f, 0 };
    VkClearAttachment attachment{ VK_IMAGE_ASPECT_DEPTH_BIT, 1, clearValue };
    VkClearRect rect{ VkRect2D{
                        VkOffset2D{ spec.portcoords[0], spec.portcoords[1] },
                        VkExtent2D{ uint32_t(spec.portcoords[2]),
                                    uint32_t(spec.portcoords[3]) } },
                      0, 1 };
    auto clearAttachments = vsg::ClearAttachments::create(
      vsg::ClearAttachments::Attachments{ attachment },
      vsg::ClearAttachments::Rects{ rect });
    ws.render_graph->addChild(clearAttachments);

    // left, right bottom, top, near, far
    auto orthoproj =
      vsg::Orthographic::create(-0.5, 0.5, -0.5, 0.5, 100.0, 0.0);
    // eye (in front of screen), at (origin), up (y up in this case)
    auto viewmatrix =
      vsg::LookAt::create(vsg::dvec3(0.0, 0.0, 1.0), vsg::dvec3(0.0, 0.0, 0.0),
                          vsg::dvec3(0.0, 1.0, 0.0));
    // viewmatrix->set(dmat4());
    //  second, ortho camera
    auto maskcamera = vsg::Camera::create(orthoproj, viewmatrix, viewportstate);

    // read the image data
    auto image = vsg::read_cast<vsg::Data>(spec.overlay, options);

    // build a quad
    auto builder = vsg::Builder::create();
    vsg::StateInfo state;
    state.image = image;
    state.lighting = false;
    // state.two_sided = true;
    state.blending = true;
    // state.billboard = true;
    vsg::GeometryInfo geom;
    geom.position.set(0.0f, 0.0f, 0.0f);  // center
    geom.dy.set(0.0f, 1.0f, 0.0f);        // height
    // geom.dz.set(0.0f, -1.0f, 0.0f);
    auto ovscene = builder->createQuad(geom, state);

    // overlay view
    auto ovlview = vsg::View::create(maskcamera, ovscene);

    ws.render_graph->addChild(ovlview);
  }
}

VSGViewer::VSGViewer() :
  winspec(),
  root(),
  observer(),
  viewer(),
  options(),
  config_dynamic_created(0),
  allow_unknown(false),
  debug_layer(false),
  debug_utils(false),
  api_dump_layer(false),
  synchronization_layer(true),
  windows(),
  active_objects(),
  static_objects(),
  post_draw(),
  viewspec(),
  resourcepath(),
  keep_pointer(false),
  bg_color(4, 0.0),
  the_fog(FogValue::create()),
  enable_simple_fog(false),
  buffer_nsamples(8)
{
  // bg_color[3] = 1.0;
  // bg_color[2] = 0.45;
}

VSGViewer::~VSGViewer() {}

  /** Quick exception struct. */
struct DuecaVSGConfigError : public std::exception
{
    /** Say what is the problem */
  const char *what() { return "Configuration error VSG"; }
};

  // to simplify programming
VSGViewer::WindowSet::WindowSet(const WinSpec &ws,
                                vsg::ref_ptr<vsg::Group> root,
                                const std::map<std::string, WindowSet> &windows,
                                std::vector<float> bg_color,
                                unsigned buffer_nsamples, bool debug_layer,
                                bool debug_utils, bool api_dump_layer,
                                bool synchronization_layer) :
  name(ws.name),
  display(ws.display),
  window(),
  traits(vsg::WindowTraits::create()),
  command_graph(),
#ifndef RG_PER_VIEWSET
  render_graph(),
#endif
  viewset()
{
  // set trait options
  traits->debugLayer = debug_layer;
  traits->apiDumpLayer = api_dump_layer;
  traits->debugUtils = debug_utils;

  // get screen size
  traits->windowTitle = ws.name;

    // do we share a device?
  for (auto const &ow : windows) {
    if (ow.second.display == ws.display) {
      I_MOD("VSG window '" << ws.name << "' shares with '" << ow.second.name
                           << "'");
      traits->device = ow.second.window->getOrCreateDevice();
    }
  }

    // Full screen?
  if (ws.size_and_position.size() == 0) {
    traits->fullscreen = true;
  }
  else {
      // Check for position information
    if (ws.size_and_position.size() == 4) {
      traits->x = ws.size_and_position[2];
      traits->y = ws.size_and_position[3];
    }
    traits->width = ws.size_and_position[0];
    traits->height = ws.size_and_position[1];
    traits->fullscreen = false;
  }

    // double buffer
  traits->swapchainPreferences.imageCount = 2;
  traits->synchronizationLayer = synchronization_layer;

    // multi sampling options
  for (unsigned sbits = 0; sbits < buffer_nsamples; sbits++) {
    traits->samples |= (1U << sbits);
  }

  window = vsg::Window::create(traits);
  command_graph = vsg::CommandGraph::create(window);

#ifndef RG_PER_VIEWSET
  render_graph = vsg::RenderGraph::create(window);
  render_graph->clearValues[0].color = { { bg_color[0], bg_color[1],
                                           bg_color[2], bg_color[3] } };

    // ensure render graph is called
  command_graph->addChild(render_graph);
#endif
}

void VSGViewer::init(bool waitswap)
{
    // based on vsgcustomshaderset

    // process what is in the commandline
  vsg::CommandLine arguments(p_argc, *p_argv);

    // create root
  options = vsg::Options::create();
  options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
  options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
  options->sharedObjects = vsg::SharedObjects::create();

    // add vsgXchange reading and writing of 3rd party file formats
  options->add(vsgXchange::all::create());
  arguments.read(options);

    // ensure pbr use my new set of shaders.
  auto pbr = vsgPBRShaderSet(options, the_fog);
  options->shaderSets["pbr"] = pbr;

    // create scene graph root
  root = vsg::StateGroup::create();
  root->setValue("name", std::string("root"));
  D_MOD("VSG create root node");

  // the "inherit option in customshaderset"
  layout = pbr->createPipelineLayout({}, { 0, 2  });

  uint32_t vds_set = 1;
  root->add(vsg::BindViewDescriptorSets::create(VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                      layout, vds_set));
  uint32_t cm_set = 0;
  auto cm_dsl = pbr->createDescriptorSetLayout({}, cm_set);
  auto cm_db = vsg::DescriptorBuffer::create(the_fog);
  auto cm_ds = vsg::DescriptorSet::create(cm_dsl, vsg::Descriptors{ cm_db });
  auto cm_bds = vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS,
                                               layout, cm_ds);
  root->add(cm_bds);

  options->inheritedState = root->stateCommands;

    // and the observer/eye group
  observer = vsg::Group::create();
  observer->setValue("name", std::string("observer"));
  root->addChild(observer);
  std::list<vsg::ref_ptr<vsg::Group>> observer_path;
  observer_path.push_back(observer);

  auto viewmatrix = vsg::TrackingViewMatrix::create(observer_path);

  // create viewer
  viewer = vsg::Viewer::create();

  // If no window specified, give a dummy default specification
  if (winspec.empty()) {
    WinSpec window;
    window.name = "DUECA/VSG default window";
    float size_and_position[] = { 400, 300 };
    window.size_and_position.resize(2);
    copy(&size_and_position[0], &size_and_position[2],
         window.size_and_position.begin());
    window.display = "";
    addWindow(window);
  }

    // now create all windows
  while (!winspec.empty()) {
    if (windows.find(winspec.front().name) != windows.end()) {
      cerr << "Already specified a window " << winspec.front().name
           << " ignoring second one" << endl;
      winspec.pop_front();
      continue;
    }
    try {
      auto newwin = windows.emplace(
        std::piecewise_construct, std::forward_as_tuple(winspec.front().name),
        std::forward_as_tuple(winspec.front(), root, windows, bg_color,
                              buffer_nsamples, debug_layer, debug_utils,
                              api_dump_layer, synchronization_layer));
      if (newwin.second == false) {
        throw(DuecaVSGConfigError());
      }
      viewer->addWindow(newwin.first->second.window);
    }
    catch (const vsg::Exception &ve) {
      E_MOD("Trying to create window '" << winspec.front().name
                                        << "' vsg error: " << ve.message);
    }
    winspec.pop_front();
  }

    // create cameras and viewports
  while (!viewspec.empty()) {
      // find the appropriate window
    WindowsMap::iterator ii = windows.find(viewspec.front().winname);
    if (ii == windows.end()) {
      E_MOD("Could not find window \"" << viewspec.front().winname
                                       << "\" for view \""
                                       << viewspec.front().name << '"');
    }
    else if (ii->second.viewset.count(viewspec.front().name)) {
      E_MOD("Already have view \"" << viewspec.front().name << "\" in window \""
                                   << viewspec.front().winname << '"');
    }
    else {
      try {
        ii->second.viewset[viewspec.front().name] = ViewSet();

          // init view
        ii->second.viewset[viewspec.front().name].init(
          viewspec.front(), ii->second, viewer, root, bg_color, options);
      }
      catch (const vsg::Exception &ve) {
        E_MOD("Trying to create view '" << viewspec.front().name
                                        << "' vsg error: " << ve.message);
      }
    }
    viewspec.pop_front();
  }

    // if applicable, initialize static objects and dynamic objects
  for (auto &ao : active_objects) {
    try {
      ao.second->init(root, this);
    }
    catch (const vsg::Exception &ve) {
      E_MOD("Trying to create object '" << ao.first
                                        << "' vsg error: " << ve.message);
    }
  }
  for (auto &so : static_objects) {
    so->init(root, this);
  }

    // add it all to the viewer
    // vsgUtil::Optimizer optimizer;
    // optimizer.optimize(root);

#if 1
    // if not created, windows are not drawn
  CommandGraphs cgs;
  for (auto const &win : windows) {
    cgs.push_back(win.second.command_graph);
  }

  viewer->assignRecordAndSubmitTaskAndPresentation(cgs);
#endif
  viewer->compile();
}

void VSGViewer::addViewport(const ViewSpec &vp) { viewspec.push_back(vp); }

void VSGViewer::redraw(bool wait, bool reset_context)
{
  if (wait || viewer->advanceToNextFrame()) {
    viewer->handleEvents();
    viewer->update();
    viewer->recordAndSubmit();
    viewer->present();
  }
}

void VSGViewer::waitSwap() { viewer->advanceToNextFrame(); }

bool VSGViewer::adaptSceneGraph(const WorldViewConfig &adapt)
{
  try {

    switch (adapt.command) {

    case WorldViewConfig::ClearModels: {

        // to be updated, only remove static objects
      for (auto &so : static_objects) {
        so->unInit(root);
      }
      static_objects.clear();
    } break;

    case WorldViewConfig::RemoveNode:
        // TODO
      break;

    case WorldViewConfig::LoadObject: {

        // creates and adds a specific configuration
      std::string dclass =
        "on-the-fly-object_" +
        boost::lexical_cast<std::string>(++config_dynamic_created);
      this->addFactorySpec(dclass, adapt.config);

        // run the createStatic call to create the object
      std::vector<std::string> createconf;
      createconf.push_back(dclass);
      this->createStatic(createconf);
    } break;

    case WorldViewConfig::MoveObject: {
        // TODO
        /*for (int ii = root->getNumChildren(); ii--; ) {
if (root->getChild(ii)->getName() == adapt.config.name) {
updateTransform(root->getChild(ii), adapt.config.coordinates);
}
}*/
    } break;
    case WorldViewConfig::ListNodes:
    case WorldViewConfig::LoadOverlay:
    case WorldViewConfig::RemoveOverlay:
    case WorldViewConfig::ReadScene:
      W_MOD("VSGViewer " << adapt.command << " is not implemented");
    }
  }
  catch (const std::exception &e) {
    W_MOD("VSGViewer caught " << e.what());
    return false;
  }
  return true;
}

void VSGViewer::setBase(TimeTickType tick, const BaseObjectMotion &ownm,
                        double late, bool freeze)
{
  BaseObjectMotion o2(ownm);
  if (!freeze) {
    o2.extrapolate(late);
  }

  auto camerapnt =
      //    vsg::rotate(0.5*vsg::PI, 1.0, 0.0, 0.0) *
    vsg::rotate(Q2phi(o2.attitude_q), 0.0, 0.0, 1.0) *
    vsg::rotate(-Q2tht(o2.attitude_q), 1.0, 0.0, 0.0) *
    vsg::rotate(Q2psi(o2.attitude_q), 0.0, 1.0, 0.0) *
    vsg::rotate(vsg::PI, 0.0, 0.0, 1.0) *
    vsg::rotate(0.5 * vsg::PI, 1.0, 0.0, 0.0) *
    vsg::translate(o2.xyz[1], o2.xyz[0], o2.xyz[2]);

    // update all cameras, as they are in the viewset list
  for (auto &win : windows) {
    for (auto &view : win.second.viewset) {
      auto world2orig = vsg::inverse(view.second.eye_offset * camerapnt);
      // D_MOD("Setting view " << view.second.name << " to " << world2orig);
      view.second.camera->viewMatrix.cast<vsg::LookAt>()->set(world2orig);
    }
  }

  // run through all active objects, and inform about the vehicle
  // position & time
  for (auto &obj : active_objects) {
    obj.second->iterate(tick, o2, late);
  }
}

bool VSGViewer::createControllable(const GlobalId &master_id,
                                   const NameSet &cname, entryid_type entry_id,
                                   uint32_t creation_id,
                                   const std::string &data_class,
                                   const std::string &entry_label,
                                   Channel::EntryTimeAspect time_aspect)
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
      viewer->compile();
    }
    boost::intrusive_ptr<VSGObject> bop(op);
    active_objects[keypair] = bop;
    return true;
  }
  catch (const CFCannotMake &problem) {
    if (!allow_unknown) {
      W_MOD("VSGViewer: factory cannot create for "
            << data_class << " encountered: " << problem.what());
      throw(problem);
    }
    W_MOD("VSGViewer: factory cannot create for "
          << data_class << ", ignoring channel " << cname << " entry "
          << entry_id);
  }
  catch (const MapSpecificationError &problem) {
    if (!allow_unknown) {
      W_MOD("VSGViewer: not configured for "
            << data_class << " encountered: " << problem.what());
      throw(problem);
    }
    W_MOD("VSGViewer: not configured for "
          << data_class << ", ignoring channel " << cname << " entry "
          << entry_id);
  }
  catch (const std::exception &problem) {
    cerr << "VSGViewer: When trying to create for " << data_class
         << " encountered: " << problem.what() << endl;
  }
  return false;
}

bool VSGViewer::createStatic(const std::vector<std::string> &name)
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
  return createStatic(obj);
}

bool VSGViewer::createStatic(const WorldDataSpec &obj)
{
  try {
    VSGObject *op = VSGObjectFactory::instance().create(obj.type, obj);
    if (root) {
      op->init(root, this);
    }
    boost::intrusive_ptr<VSGObject> bop(op);
    if (op->forceActive()) {
      creation_key_t keypair(obj.name, 0);
      if (active_objects.count(keypair)) {
        W_MOD("Object name (" << obj.name << ",0) already taken!");
        static_objects.push_back(bop);
      }
      else {
        active_objects[keypair] = bop;
      }
    }
    else {
      static_objects.push_back(bop);
    }
    return true;
  }
  catch (const std::exception &problem) {
    W_MOD("VSGViewer: When trying to create "
          << obj.type << " encountered: " << problem.what());
  }
  return false;
}
void VSGViewer::removeControllable(const NameSet &cname, uint32_t creation_id)
{
  active_objects[std::make_pair(cname.name, creation_id)].reset();
}

}; // namespace vsgviewer
