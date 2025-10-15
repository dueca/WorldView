/* ------------------------------------------------------------------   */
/*      item            : VSGViewer.hxx
        made by         : Rene van Paassen
        date            : 090616
        category        : header file
        description     :
        changes         : 090616 first version
        language        : C++
*/

#ifndef VSGViewer_hxx
#define VSGViewer_hxx

#include "VSGObject.hxx"
#include <map>
#include <string>
#include <vector>

#include "VSGObjectFactory.hxx"
#include "VSGPBRShaderSet.hxx"
#include "VSGXMLReader.hxx"
#include <WorldViewerBase.hxx>
#include <vsg/all.h>
#include <vsg/core/Array.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/state/BufferInfo.h>
#include <vsg/state/PipelineLayout.h>
#include <vsgXchange/all.h>

// #define RG_PER_VIEWSET

// Used the vsgwindows example as inspiration

// resources:
// https://github.com/SaschaWillems/Vulkan
//

namespace vsgviewer {

/** This is a wrapper that can load and display VSG scenes, optionally
    in multiple windows and viewports. It is intended for
    encapsulation in a DUECA module. */
class VSGViewer : public WorldViewerBase
{
  // Advance definition, collection of data for a window.
  struct WindowSet;

  // Advance definition, collection of data for a viewport
  struct Private;

  std::list<WinSpec> winspec;

  /** scene manager */
  vsg::ref_ptr<vsg::StateGroup> root;

  /** observer transform will be updated with the ego motion */
  vsg::ref_ptr<vsg::AbsoluteTransform> observer_transform;

  /** observer is a node in the scene */
  vsg::ref_ptr<vsg::Group> observer;

  /** A single viewer, matching a single scene */
  vsg::ref_ptr<vsg::Viewer> viewer;

  /** Specific pipeline */
  vsg::ref_ptr<vsg::PipelineLayout> layout;

  /** Reader for xml definitions */
  boost::scoped_ptr<VSGXMLReader> xml_reader;

  /** Shadow settings, needed for the lights */
  vsg::ref_ptr<vsg::ShadowSettings> shadowSettings;

public:
  /** Options object */
  vsg::ref_ptr<vsg::Options> options;

private:
  /** counter dynamical creation */
  unsigned config_dynamic_created;

protected:
  /** Accept unknown/unconfigured objects */
  bool allow_unknown;

  /** Enable vulkan debug layer */
  bool debug_layer;

  /** Enable debug utils layer */
  bool debug_utils;

  /** Enable vulkan API dump */
  bool api_dump_layer;

  /** Enable Khronos synchronization layer */
  bool synchronization_layer;

private:
  /** This class can generate multiple views on the same world. A
      ViewSet encapsulates the stuff needed for a single view. */
  struct ViewSet
  {

    /** Name, for debugging purposes. */
    std::string name;

    /** The render camera set-up */
    vsg::ref_ptr<vsg::Camera> camera;

    /** The view of this camera */
    vsg::ref_ptr<vsg::View> view;

    /** The view matrix for the camera */
    vsg::ref_ptr<vsg::LookAt> view_matrix;

    /** The camera's offset from the base vehicle point (angle,
        distance or both) */
    vsg::t_mat4<double> eye_offset;

#ifdef RG_PER_VIEWSET
    /** And a rendergraph?*/
    vsg::ref_ptr<vsg::RenderGraph> render_graph;
#endif

    /** Constructor */
    ViewSet();

    /** Initialise a view in a window

        @param vs     Specification for the view; viewport coordinates and
                      perspective/frustum, eye position+orientation
        @param window Window specification
        @param viewer Overall scene viewer
        @param root   Scene root
        @param bg_col Background color (4 element)
        @param maxShadowDistance Tuning distance for shadow map size
        @param options Current VSG options
    */
    void init(const ViewSpec &vs, WindowSet &window,
              vsg::ref_ptr<vsg::Viewer> viewer, vsg::ref_ptr<vsg::Group> root,
              const std::vector<float> &bg_col, float maxShadowDistance,
              vsg::ref_ptr<vsg::Options> options);

    /** create the camera and window. */
    void complete();
  };

  /** Each render window needs a specification and possibly a set of
      views */
  struct WindowSet
  {

    /** Descriptive name */
    std::string name;

    /** Display on which it is presented */
    std::string display;

    /** The actual window */
    vsg::ref_ptr<vsg::Window> window;

    /** Traits of the window */
    vsg::ref_ptr<vsg::WindowTraits> traits;

#ifndef RG_PER_VIEWSET
    /** And a rendergraph?*/
    vsg::ref_ptr<vsg::RenderGraph> render_graph;
#endif

    /** Each window has a command graph */
    vsg::ref_ptr<vsg::CommandGraph> command_graph;

    /** A list of view sets; these represent the different render
        areas within the window */
    std::map<std::string, ViewSet> viewset;

    /** Constructor */
    WindowSet(const WinSpec &ws, vsg::ref_ptr<vsg::Group> root,
              const std::map<std::string, WindowSet> &windows,
              std::vector<float> bg_color, unsigned buffer_nsamples,
              bool debug_layer, bool debug_utils, bool api_dump_layer,
              bool synchronization_layer);
  };

  /** Map of windows */
  typedef std::map<std::string, WindowSet> WindowsMap;

  /** map with windows. */
  WindowsMap windows;

  /** Object list type */
  typedef std::list<boost::intrusive_ptr<VSGObject>> ObjectListType;

  /** Map with created (due to presence in the world channel)
      objects. They are indexed with a creation key combining channel
      name and channel entry index, and removed from the map when the
      entry is removed from the channel. */
  typedef std::map<creation_key_t, boost::intrusive_ptr<VSGObject>>
    created_objects_t;

  /** Objects linked to a channel entry */
  created_objects_t controlled_objects;

  /** Objects that are static, just get calls about new positioning */
  ObjectListType active_objects;

  /** Objects that are static, dont get calls about new positioning */
  ObjectListType static_objects;

  /** Objects that need post-draw access */
  ObjectListType post_draw;

  /** List of specifications for the wiews, will be applied later */
  std::list<ViewSpec> viewspec;

private:
  /** Helper function, loads resources from resources.cfg */
  void setupResources();

public:
  /** Constructor */
  VSGViewer();

  /** Destructor */
  ~VSGViewer();

  /** Open the window(s) */
  void init(bool waitswap);

  /** Add a window */
  void addViewport(const ViewSpec &vp);

  /** Add a window */
  void addWindow(const WinSpec &window) { winspec.push_back(window); }

  /** Pass the base camera position to the views.

      @param tick  Current DUECA time.
      @param base  Motion definition of the base observer/vehicle.
      @param late  Time [s] into current DUECA time tick.
  */
  void setBase(TimeTickType tick, const BaseObjectMotion &base, double late,
               bool freeze);

  /** Create a controllable object. Object creation depends on class of
      data supplied, further init may rely on fist data entering. */
  bool createControllable(const GlobalId &master_id, const NameSet &cname,
                          entryid_type entry_id, uint32_t creation_id,
                          const std::string &data_class,
                          const std::string &entry_label,
                          Channel::EntryTimeAspect time_aspect);

  /** Remove a controllable */
  void removeControllable(const dueca::NameSet &cname, uint32_t creation_id);

  /** Create a static (not controlled) controllable object. */
  bool createStatic(const std::vector<std::string> &classname);

  /** Create a static (not controlled) controllable object. */
  bool createStatic(const WorldDataSpec &spec);

  /** Do a re-draw
      @param wait   If true, do now swap the buffers. The application
                    must later wait and swap with the waitSwap function.
    */
  void redraw(bool wait = false, bool reset_context = false) final;

  /** Wait for the swap. */
  void waitSwap();

  /** Change the configuration of the scene graph, returns true if
      successful */
  bool adaptSceneGraph(const WorldViewConfig &adapt);

  /** Create the XML reader */
  bool setXMLReader(const std::string &definitions);

  /** Read an XML file with object data */
  bool readModelFromXML(const std::string &file);

  /** Shadow maps? */
  inline vsg::ref_ptr<vsg::ShadowSettings> getShadowSettings() const
  {
    return shadowSettings;
  }

  /** Clear models */
  void clearModels();

protected:
  /** Path to the resources */
  std::string resourcepath;

  /** Keep the cursor or not? */
  bool keep_pointer;

  /** background/clear color */
  std::vector<float> bg_color;

  /** Fog initialization value */
  Fog my_fog;

  /** Fog object pointer */
  vsg::ref_ptr<vsg::Value<Fog>> the_fog;

  /** Enable simple fog. */
  bool enable_simple_fog;

  /** Multi-sampling */
  unsigned buffer_nsamples;

  /** Shadow radius, fading */
  float penumbraRadius;

  /** Shadow maps */
  int shadowMapCount;

  /** Shadow max distance */
  float maxShadowDistance;
};

}; // namespace vsgviewer

#endif
