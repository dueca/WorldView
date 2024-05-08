/* ------------------------------------------------------------------   */
/*      item            : OSGViewer_Dueca.cxx
        made by         : rvanpaassen
        from template   : DuecaModuleTemplate.cxx
        template made by: Rene van Paassen
        date            : Tue Jan 26 15:25:27 2010
        category        : body file
        description     :
        changes         : Tue Jan 26 15:25:27 2010 first version
        template changes: 030401 RvP Added template creation comment
        language        : C++
*/

#define OSGViewer_Dueca_cxx
// include the definition of the helper class
#include "OSGViewer_Dueca.hxx"
#include <dueca-version.h>

// include additional files needed for your calculation here
#include "OSGObjectFactory.hxx"

#define DO_INSTANTIATE
#include <CoreCreator.hxx>
#include <MemberCall.hxx>
#include <MemberCall2Way.hxx>
#include <VarProbe.hxx>

// include the debug writing header, by default, write warning and
// error messages
#define W_MOD
#define E_MOD
#include <debug.h>

USING_DUECA_NS;

// Parameters to be inserted
const ParameterTable *OSGViewer_Dueca::getParameterTable()
{
  static const ParameterTable parameter_table[] = {

    /* You can extend this table with labels and MemberCall or
       VarProbe pointers to perform calls or insert values into your
       class objects. Please also add a description (c-style string). */
    { "set-resourcepath",
      new VarProbe<_ThisObject_, string>(&_ThisObject_::resourcepath),
      "set the path to the resources" },

#if 0
    // disabled, for now
    { "sync-divisor",
      new VarProbe<_ThisObject_,int>
      (&_ThisObject_::glx_sync_divisor),
      "For video sync wait mode, set the divisor (e.g. 2 means 30 Hz at a\n"
      "60 Hz refresh rate)" },

    { "sync-offset",
      new VarProbe<_ThisObject_,int>
      (&_ThisObject_::glx_sync_offset),
      "For video sync wait mode, set the offset" },
#endif

    { "keep-cursor",
      new VarProbe<_ThisObject_, bool>(&_ThisObject_::keep_pointer),
      "By default, the cursor is removed. Set to true to keep the cursor" },

    { "add-window",
      new MemberCall<_ThisObject_, string>(&_ThisObject_::addWindow),
      "Add a window to this Scene drawer. Specify the window name. Do not\n"
      "forget to specify window size and (optionally) position" },

    { "window-size+pos",
      new MemberCall<_ThisObject_, vector<int>>(
        &_ThisObject_::setWindowPosition),
      "specify width, height and optionally x, y position of a new window" },

    { "window-x-screen",
      new MemberCall<_ThisObject_, string>(&_ThisObject_::setWindowXScreen),
      "Set the display string for this window" },

    { "add-viewport",
      new MemberCall<_ThisObject_, string>(&_ThisObject_::addViewport),
      "Add a viewport to this Scene drawer, specify a name" },

    { "viewport-window",
      new MemberCall<_ThisObject_, string>(&_ThisObject_::setViewportWindow),
      "Specify the window for this viewport" },

    { "viewport-pos+size",
      new MemberCall<_ThisObject_, vector<int>>(&_ThisObject_::setViewportSize),
      "Specify size and location (top left) for a viewport, 4 parameters" },

    { "eye-offset",
      new MemberCall<_ThisObject_, vector<float>>(&_ThisObject_::setEyeOffset),
      "Offset of the eye with respect to sent position; 3 parameters for\n"
      "x, y and z location, and three parameters for phi, theta, psi [deg]" },

    { "static-object",
      new MemberCall<_ThisObject_, vector<string>>(&_ThisObject_::createStatic),
      "Add a new static object, supply a class match and (optional) name" },

    { "add-object-class-coordinates",
      new MemberCall<_ThisObject_, vector<double>>(
        &_ThisObject_::addCoordinates),
      "Set coordinates for the object; x, y, z position, and phi, theta, psi\n"
      "orientation. Note that these may also be used differently, depending\n"
      "on the specified object-class\n"
      "- static: the coordinates give the object position and orientation\n"
      "- centered: non-zero x, y, z give object posn, zero x, y or z means\n"
      "            the object stays centered on the observer\n"
      "- tiled: zero x, y, z mean that that coordinate is absolute, fixed in\n"
      "         the world, negative means that coordinate follows observer\n"
      "         positive defines the jump with which the object follows the\n"
      "         observer\n"
      "- For all three object types you can also add sx, sy, sz factors\n"
      "For 'light' objects, specify the following with 'object-coordinates:\n"
      "- ambient, diffuse and specular reflection, 3x4 numbers (RGBA)\n"
      "- object position, 4 numbers. If the 4th number is 1, this describes\n"
      "  a light position, the 4th number 0 describes a light direction\n"
      "- object direction, 3 numbers. If these are 0, a point light results,\n"
      "  otherwise, gives the direction of a spot (beam)\n"
      "- constant attenuation (0 .. 1)\n"
      "- linear attenuation, (0 .. 1)\n"
      "- quadratic attenuation, (0 .. 1)\n"
      "- spot exponent, defines sharpness of beam edge\n"
      "- spot cutoff, defines beam width in degrees" },

    { "add-object-class-data",
      new MemberCall<_ThisObject_, std::vector<std::string>>(
        &_ThisObject_::addObjectClassData),
      "Add a class of objects, arguments:\n"
      " - <matchstring>: The first argument is the match string;\n"
      "   For objects controlled through a channel, matching is done on the\n"
      "   basis of the channel entry data type, with the object's label, e.g.\n"
      "   \"BaseObjectMotion:Cessna 550\". If after that, no match is found\n"
      "   matching continues with parent classes of the given entry data type\n"
      "   then matching is done on the data class only.\n"
      "   When creating static objects, the match is on the first argument of\n"
      "   the create-static\n"
      " - <object name>: The object name will be used as name in OSG.\n"
      "   if the last character is a '#', the name will be suffixed with the\n"
      "   channel entry creation number. The name can also be split into a\n"
      "   matching and naming section, separated by a :, e.g., \"B737:PHANH\"\n"
      "   then the matching will be done on the first part, and the objects'\n"
      "   name is determined by the latter part\n"
      " - <factory class>: The type of an added object, currently the\n"
      "   following are available: 'moving', 'static', 'centered' or 'tiled'\n"
      "                            'light', 'static-light', 'centered-light\n"
      " - [<additional strings>]: supply, depending on type of object, \n"
      "   additional parameters like filenames etc." },

    { "create-static",
      new MemberCall<_ThisObject_, std::vector<std::string>>(
        &_ThisObject_::createStatic),
      "Create an object through the factory that will not receive a\n"
      "connection to a channel entry, specify match string and name" },

    { "add-object-class",
      new MemberCall<_ThisObject_, vector<string>>(
        &_ThisObject_::addObjectClass),
      "Create a new class (type) of simple objects, compatible for old\n"
      "ObjectMotion types. Specify the type name, the name of a model file\n"
      "Based on the \"klass\"\n"
      "value in the ObjectMotion object, objects of this type will be\n"
      "created.\n" },

    { "set-frustum",
      new MemberCall<_ThisObject_, vector<float>>(&_ThisObject_::setFrustum),
      "Set frustum geometry, <near plane> <far plane>, and then either:\n"
      "<fov y> [rad] for specifying viewing with only a field-o-view angle,\n"
      "or <left> <right> <bottom> <top> of the near frustum plane\n"
      "in camera coordinates" },

    { "add-external-object",
      new MemberCall2Way<_ThisObject_, ScriptCreatable>(
        &_ThisObject_::addScriptObject),
      "Add a graphic object created in the script file. The object must\n"
      "derive from OSGObject." },

    { "set-bg-color",
      new VarProbe<_ThisObject_, std::vector<double>>(&_ThisObject_::bg_color),
      "set the background color, R, G, B, A components, scaled 0 -- 1" },

    { "set-fog",
      new MemberCall<_ThisObject_, std::vector<double>>(&_ThisObject_::setFog),
      "Set fog parameters, expects 8 numbers:\n"
      "{1|2|3 for linear, exp, exp2}, fog density,\n"
      "fog color (RGBA, 4 elts), fog start, fog end" },

    { "use-compositeviewer",
      new VarProbe<_ThisObject_, bool>(&_ThisObject_::use_compositeviewer),
      "Use openscenegraph composite viewer" },

    { "allow-unknown",
      new VarProbe<_ThisObject_, bool>(&_ThisObject_::allow_unknown),
      "ignore unknown or unconnected objects in world information channels" },

    { "add-draw-callback",
      new MemberCall2Way<_ThisObject_, ScriptCreatable>(
        &_ThisObject_::addDrawCallback),
      "Add a (post-draw) callback to the current viewport's camera,\n"
      "expects an object similar to an OSGCallback" },

      /* The table is closed off with NULL pointers for the variable
     name and MemberCall/VarProbe object. The description is used to
     give an overall description of the module. */
    { NULL, NULL,
      "Helper object that implements a view (or more) with the use of OSG\n"
      "as scene manager. To be used by a world-view module" }
  };

  return parameter_table;
}

// constructor
OSGViewer_Dueca::OSGViewer_Dueca() :
  ScriptCreatable(),
  build_callback(NULL)
{}

bool OSGViewer_Dueca::complete()
{
  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */
  if (bg_color.size() != 3 && bg_color.size() != 4) {
    E_CNF("Need 3 or 4 components for background color");
    return false;
  }

  if (build_win_spec.sufficient()) {
    // cout << "adding last window specification" << endl;
    OSGViewer::addWindow(build_win_spec);
    build_win_spec = WinSpec();
  }

  if (build_view_spec.sufficient()) {
    // cout << "adding last viewport specification" << endl;
    OSGViewer::addViewport(build_view_spec);

    OSGViewer::setDrawCallback(build_view_spec.name, build_callback);
    build_callback = NULL;

    build_view_spec = ViewSpec();
  }
  else {
    E_CNF("Viewport specification not complete");
    return false;
  }

  // Information on the factory
  // std::cout << "OSG factory elements" << std::endl;
  // OSGObjectFactory::instance().catalogue(std::cout);

  return true;
}

// destructor
OSGViewer_Dueca::~OSGViewer_Dueca()
{
  //
}

bool OSGViewer_Dueca::addWindow(const std::string &window)
{
  // check whether there is a complete window
  if (build_win_spec.sufficient()) {
    OSGViewer::addWindow(build_win_spec);
    build_win_spec = WinSpec();
  }

  build_win_spec.name = window;
  return true;
}

bool OSGViewer_Dueca::setWindowPosition(const std::vector<int> &pos)
{
  if (pos.size() != 2 && pos.size() != 4) {
    E_CNF("Need 2 or 4 parameters for window size and position");
    return false;
  }
  build_win_spec.size_and_position = pos;
  return true;
}

bool OSGViewer_Dueca::setWindowXScreen(const std::string &scr)
{
  build_win_spec.display = scr;
  return true;
}

bool OSGViewer_Dueca::addViewport(const std::string &viewport)
{
  // check whether the viewport currently under construction
  // is complete, and finalize it
  if (build_view_spec.sufficient()) {
    OSGViewer::addViewport(build_view_spec);

    // add current callback to current viewport
    OSGViewer::setDrawCallback(build_view_spec.name, build_callback);
    build_callback = NULL;

    // prepare the next viewport
    build_view_spec = ViewSpec();
  }
  build_view_spec.name = viewport;
  return true;
}

bool OSGViewer_Dueca::setViewportWindow(const std::string &vp_window)
{
  build_view_spec.winname = vp_window;
  return true;
}

bool OSGViewer_Dueca::setViewportSize(const std::vector<int> &vpwin)
{
  if (vpwin.size() != 4) {
    E_CNF("Need four parameter for viewport size");
    return false;
  }
  build_view_spec.portcoords = vpwin;
  return true;
}

bool OSGViewer_Dueca::setFrustum(const std::vector<float> &frustum)
{
  if (frustum.size() != 3 && frustum.size() != 6) {
    E_CNF("Need 3 or 6 parameters for frustum size");
    return false;
  }
  build_view_spec.frustum_data = frustum;
  return true;
}

bool OSGViewer_Dueca::setEyeOffset(const std::vector<float> &eye)
{
  if (eye.size() != 3 && eye.size() != 6) {
    E_CNF("Need 3 or 6 parameters for eye offset");
    return false;
  }
  build_view_spec.eye_pos = eye;
  return true;
}

/** This is for adding dueca script created object. */
bool OSGViewer_Dueca::addScriptObject(ScriptCreatable &ava, bool in)
{
  // check direction
  if (!in)
    return false;

  // try a dynamic cast
  OSGObject *object = dynamic_cast<OSGObject *>(&ava);
  if (object == NULL) {
    E_CNF("must supply an ObjectBase for the visual");
    return false;
  }

#if DUECA_VERSION_NUM < DUECA_VERSION(2, 3, 0)
  // now make sure Scheme does not clean this helper from right
  // under our noses:
  scheme_id.addReferred(ava.scheme_id.getSCM());
#endif
  assert(0);
  // OSGViewer::addObject(object);

  return true;
}

bool OSGViewer_Dueca::setObjectCoordinates(const std::vector<double> &coord)
{
  build_object_spec.coordinates = coord;
  return true;
}

bool OSGViewer_Dueca::setObjectBehavior(const std::string &beh)
{
  if (beh.size() != 0) {
    build_object_spec.type = beh;
    return true;
  }
  E_CNF("Specify a non-zero string for the type");
  return false;
}

/** Pre-cooked single-file movable object that can be controlled with
    a BaseObjectMotion entry. */
class SimpleOSGObject : public OSGObject
{
public:
  /** Constructor.

      Creates a single-file viewable object.

      @param name      Name of the object.
      @param filename  3D model to be read.
  */
  SimpleOSGObject(const string &name, const string &filename)
  {
    this->modelfile = filename;
    this->name = name;
  }

  /** Connect to a channel entry

      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel
      @param time_aspect How should the channel data be read. */
  virtual void connect(const GlobalId &master_id, const NameSet &cname,
                       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect) override
  {}

  /** Play, update, recalculate, etc. */
  void iterate(TimeTickType ts, const BaseObjectMotion &base, double late,
               bool freeze = false) override
  {}
};

class SimpleOSGSubcontractor : public SubcontractorBase<OSGObjectTypeKey>
{
  /** File with the graphic model */
  string filename;

public:
  /** Constructor */
  SimpleOSGSubcontractor(const string &filename) :
    filename(filename)
  {}

  /** create the object */
  OSGObjectTypeKey::ProductBase create(const OSGObjectTypeKey::Key &key,
                                       const OSGObjectTypeKey::SpecBase &spec)
  {
    return new SimpleOSGObject(key, filename);
  }
};

bool OSGViewer_Dueca::addObjectClass(const std::vector<std::string> &names)
{
  if (names.size() != 2) {
    E_CNF("Specify a class name and a file describing the object");
    return false;
  }

  OSGObjectFactory::instance().addSubcontractor(
    names[0], SubconPtr(new SimpleOSGSubcontractor(names[1])));
  return true;
}

bool OSGViewer_Dueca::addObjectClassData(const std::vector<std::string> &names)
{
  if (names.size() < 3) {
    E_CNF("Specify at least a match name, object name and the class type");
    return false;
  }

  WorldDataSpec obj;
  obj.type = names[2];
  obj.name = names[1];
  for (size_t ii = 3; ii < names.size(); ii++) {
    obj.filename.push_back(names[ii]);
  }

  addFactorySpec(names[0], obj);
  return true;
}

bool OSGViewer_Dueca::setFog(const std::vector<double> &fog)
{
  if (fog.size() != 8) {
    E_CNF("Specify 8 numbers for fog");
    return false;
  }
  if (fog[0] < 1 || fog[0] > 3) {
    E_CNF("specify fog type 1, 2, or 3");
  }
  fog_mode = FogMode(int(round(fog[0])));
  fog_density = fog[1];
  fog_colour.set(fog[2], fog[3], fog[4], fog[5]);
  fog_start = fog[6];
  fog_end = fog[7];
  return true;
}

bool OSGViewer_Dueca::addDrawCallback(ScriptCreatable &cb, bool in)
{
  // check direction
  if (!in)
    return false;

  // try a dynamic cast
  build_callback = dynamic_cast<osg::Camera::Camera::DrawCallback *>(&cb);
  if (build_callback == NULL) {
    E_CNF("must supply an osg DrawCallback for the visual");
    return false;
  }

  return true;
}

// script access macro
SCM_FEATURES_NOIMPINH(OSGViewer_Dueca, ScriptCreatable, "osg-viewer-dueca");

// Make a CoreCreator object for this module, the CoreCreator
// will check in with the scheme-interpreting code, and enable the
// creation of objects of this type

#ifdef SCRIPT_PYTHON
static CoreCreator<OSGViewer_Dueca> a(OSGViewer_Dueca::getParameterTable(),
                                      "OSGViewer");
#else
static CoreCreator<OSGViewer_Dueca> a(OSGViewer_Dueca::getParameterTable());
#endif
