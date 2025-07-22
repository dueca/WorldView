/* ------------------------------------------------------------------   */
/*      item            : VSGViewer_Dueca.cxx
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


#define VSGViewer_Dueca_cxx
// include the definition of the helper class
#include "VSGViewer_Dueca.hxx"
#include <dueca-version.h>

// include additional files needed for your calculation here
#include "VSGObjectFactory.hxx"

#define DO_INSTANTIATE
#include <VarProbe.hxx>
#include <MemberCall.hxx>
#include <MemberCall2Way.hxx>
#include <CoreCreator.hxx>

// include the debug writing header, by default, write warning and
// error messages
#define W_MOD
#define E_MOD
#include <debug.h>

USING_DUECA_NS;

namespace vsgviewer {

  // Parameters to be inserted
  const ParameterTable* VSGViewer_Dueca::getParameterTable()
  {
    static const ParameterTable parameter_table[] = {

      /* You can extend this table with labels and MemberCall or
         VarProbe pointers to perform calls or insert values into your
         class objects. Please also add a description (c-style string). */
      { "set-resourcepath",
        new VarProbe<_ThisObject_,string>
        (&_ThisObject_::resourcepath),
        "set the path to the resources" },

      { "keep-cursor",
        new VarProbe<_ThisObject_,bool>
        (&_ThisObject_::keep_pointer),
        "By default, the cursor is removed. Set to true to keep the cursor" },

      { "add-window",
        new MemberCall<_ThisObject_,string>
        (&_ThisObject_::addWindow),
        "Add a window to this Scene drawer. Give an identifying name for the\n"
        "window. After this, call 'window-size+pos'" },

      { "window-size+pos",
        new MemberCall<_ThisObject_,vector<int> >
        (&_ThisObject_::setWindowPosition),
        "specify width, height and optionally x, y position of a new window" },

      { "window-display",
        new MemberCall<_ThisObject_,string>
        (&_ThisObject_::setWindowXScreen),
        "Set the display string identification for this window" },

      { "add-viewport",
        new MemberCall<_ThisObject_,string>
        (&_ThisObject_::addViewport),
        "Add a new viewport. Give an identifying name, then use\n"
        "'viewport-window' to associate it with a window, and\n"
        "'viewport-pos+size' to give the position and size of the viewport" },

      { "viewport-window",
        new MemberCall<_ThisObject_,string>
        (&_ThisObject_::setViewportWindow),
        "Specify the window for this viewport" },

      { "viewport-pos+size",
        new MemberCall<_ThisObject_,vector<int> >
        (&_ThisObject_::setViewportSize),
        "Specify size and location (top left) for a viewport, 4 parameters" },

      { "eye-offset",
        new MemberCall<_ThisObject_,vector<float> >
        (&_ThisObject_::setEyeOffset),
        "Offset of the eye with respect to sent position for the current\n"
        "viewport; 3 parameters for x, y and z location, and three parameters\n"
        "for phi, theta, psi [deg]" },

      { "set-frustum",
        new MemberCall<_ThisObject_,vector<float> >
        (&_ThisObject_::setFrustum),
        "Set view geometry, <near plane> <far plane>, and then either:\n"
        "<fov y> for specifying viewing with only a field-o-view angle, or\n"
        "<left> <right> <bottom> <top> of the near frustum plane\n"
        "in camera coordinates" },

      { "add-object-class",
        new MemberCall<_ThisObject_,std::vector<std::string> >
        (&_ThisObject_::addObjectClassData),
        "Add a class of objects, arguments:\n"
        " - <matchstring>: The first argument is the match string;\n"
        "   For objects controlled through a channel, matching is done on the\n"
        "   basis of the channel entry data type, with the object's label, e.g.\n"
        "   \"BaseObjectMotion:Cessna 550\". If after that, no match is found\n"
        "   matching continues with parent classes of the given entry data type\n"
        "   and the object label, then matching is done on the data class only.\n"
        " - <object name>: The object name will be used as name in the VSG\n"
        "   scene graph. If the last character of the name is a '#', the name\n"
        "   will be suffixed with the channel entry creation number. When the\n"
        "   object name consists of two parts with a '/' to separate these, the\n"
        "   first part denotes the parent node in the scene graph, the second\n"
        "   becomes the name. Note that special parent nodes \"root\" and\n"
        "   \"observer\" are available to fix resp. carry objects.\n"
        " - <factory class>: The type of an added object, currently the\n"
        "   following are available:\n"
        "   - static-model   - Object with parent's position/orientation\n"
        "   - model          - Object that gets external 3D position\n"
        "   - ambient-light  - Ambient light definition\n"
        "   - directional-light - Light with direction\n"
        "   - point-light    - Point-type light\n"
        "   - spot-light     - Spotlight\n"
        "   - static-transform - Constant transform (scale, rotate, translate)\n"
        "   - transform      - Transform with external 3D position\n"
        "   - centered-transform - Transform with position attached to observer\n"
        "   Transformations are either absolute (with root as parent), or can be\n"
        "   made relative by using a moving parent (\"observer\" or other)\n"
        "   Note that the factory is extendable, by adding object files with\n"
        "   factory connections, e.g., to create a HUD overlay\n."
        " - [<additional strings>]: supply, depending on type of object, \n"
        "   additional parameters like filenames etc."
      },

      { "add-object-class-parameters",
        new MemberCall<_ThisObject_,vector<double> >
        (&_ThisObject_::addCoordinates),
        "Set parameters for the object class\n"
        "transformations: x, y, z, phi, theta, psi, optionally scale (3x)\n"
        "all lights: color (r, g, b), intensity\n"
        "directional-light: + light direction dx, dy, dz\n"
        "point light: + light location x, y, z + radius\n"
        "spot light: + position (x, y, z), span, inner angle, outer angle\n"
        "            direction (dx, dy, dz)\n"
        "To move lights / static-model, give them a transform as parent\n"
      },

      { "create-static",
        new MemberCall<_ThisObject_,std::vector<std::string> >
        (&_ThisObject_::createStatic),
        "Create an object through the factory that will not receive a\n"
        "connection to a channel entry, specify match string and optionally\n"
        "a string to override the name" },

      { "set-bg-color",
        new VarProbe<_ThisObject_,std::vector<float> >
        (&_ThisObject_::bg_color),
        "set the background color, R, G, B, A components, scaled 0 -- 1" },

      { "set-fog",
        new MemberCall<_ThisObject_, std::vector<double> >
        (&_ThisObject_::setFog),
        "Set fog parameters, expects 7 numbers:\n"
        "fog density,\n"
        "fog color (RGB, 3 elts)\n"
        "fog start (when linear)\n"
        "fog end\n"
        "fog exponent" },

      { "allow-unknown",
        new VarProbe<_ThisObject_,bool>(&_ThisObject_::allow_unknown),
        "Ignore unknown or unconnected objects in world information channels" },

      { "set-xml-definitions",
        new MemberCall<_ThisObject_,std::string>(&_ThisObject_::setXMLReader),
        "Initialise the XML reader, argument is an XML file with mappings\n"
        "from named coordinates/parameters to ranges in the coordinate vectors.\n"
        "See 'vsgobjects.xsd' for the format, and 'vsgobjects.xml' for an\n"
        "example." },

      { "read-xml-definitions",
        new MemberCall<_ThisObject_,std::string>(&_ThisObject_::readModelFromXML),
        "Read the graphics models from an XML file definition. See\n"
        "'vsgworld.xsd' for the format. This provides an alternative to using\n"
        "the add_object_class, add_object_class_parameters and static_object\n"
        "parameters.\n" },

      /* The table is closed off with NULL pointers for the variable
         name and MemberCall/VarProbe object. The description is used to
         give an overall description of the module. */
      { NULL, NULL,
        "Helper object that implements a view (or more) with the use of VSG\n"
        "as scene manager. To be used by a world-view module"}
    };

    return parameter_table;
  }

  // constructor
  VSGViewer_Dueca::VSGViewer_Dueca() :
    ScriptCreatable()
  {

  }

  bool VSGViewer_Dueca::complete()
  {
    /* All your parameters have been set. You may do extended
       initialisation here. Return false if something is wrong. */
    if (bg_color.size() != 3 && bg_color.size() != 4) {
      E_CNF("Need 3 or 4 components for background color");
      return false;
    }

    if (build_win_spec.sufficient()) {
      // cout << "adding last window specification" << endl;
      VSGViewer::addWindow(build_win_spec);
      build_win_spec = WinSpec();
    }

    if (build_view_spec.sufficient()) {
      // cout << "adding last viewport specification" << endl;
      VSGViewer::addViewport(build_view_spec);

      build_view_spec = ViewSpec();
    }
    else {
      E_CNF("Viewport specification not complete");
      return false;
    }

    // Information on the factory
    // std::cout << "VSG factory elements" << std::endl;
    // VSGObjectFactory::instance().catalogue(std::cout);

    return true;
  }

  // destructor
  VSGViewer_Dueca::~VSGViewer_Dueca()
  {
    //
  }

  bool VSGViewer_Dueca::addWindow(const std::string& window)
  {
    // check whether there is a complete window
    if (build_win_spec.sufficient()) {
      VSGViewer::addWindow(build_win_spec);
      build_win_spec = WinSpec();
    }

    build_win_spec.name = window;
    return true;
  }

  bool VSGViewer_Dueca::setWindowPosition(const std::vector<int>& pos)
  {
    if (pos.size() != 2 && pos.size() != 4) {
      E_CNF("Need 2 or 4 parameters for window size and position");
      return false;
    }
    build_win_spec.size_and_position = pos;
    return true;
  }

  bool VSGViewer_Dueca::setWindowXScreen(const std::string& scr)
  {
    build_win_spec.display = scr;
    return true;
  }

  bool VSGViewer_Dueca::addViewport(const std::string& viewport)
  {
    // check whether the viewport currently under construction
    // is complete, and finalize it
    if (build_view_spec.sufficient()) {
      VSGViewer::addViewport(build_view_spec);

      // prepare the next viewport
      build_view_spec = ViewSpec();
    }
    build_view_spec.name = viewport;
    return true;
  }

  bool VSGViewer_Dueca::setViewportWindow(const std::string& vp_window)
  {
    build_view_spec.winname = vp_window;
    return true;
  }

  bool VSGViewer_Dueca::setViewportSize(const std::vector<int>& vpwin)
  {
    if (vpwin.size() != 4) {
      E_CNF("Need four parameter for viewport size");
      return false;
    }
    build_view_spec.portcoords = vpwin;
    return true;
  }

  bool VSGViewer_Dueca::setFrustum(const std::vector<float>& frustum)
  {
    if (frustum.size() != 3 && frustum.size() != 6) {
      E_CNF("Need 3 or 6 parameters for frustum size");
      return false;
    }
    build_view_spec.frustum_data = frustum;
    return true;
  }

  bool VSGViewer_Dueca::setEyeOffset(const std::vector<float>& eye)
  {
    if (eye.size() != 3 && eye.size() != 6) {
      E_CNF("Need 3 or 6 parameters for eye offset");
      return false;
    }
    build_view_spec.eye_pos = eye;
    return true;
  }

  /** This is for adding dueca script created object. */
  bool VSGViewer_Dueca::addScriptObject(ScriptCreatable& ava, bool in)
  {
    // check direction
    if (!in) return false;

    // try a dynamic cast
    VSGObject* object = dynamic_cast<VSGObject*> (&ava);
    if (object == NULL) {
      E_CNF("must supply an ObjectBase for the visual");
      return false;
    }

#if DUECA_VERSION_NUM < DUECA_VERSION(2,3,0)
    // now make sure Scheme does not clean this helper from right
    // under our noses:
    scheme_id.addReferred(ava.scheme_id.getSCM());
#endif
    assert(0);
    //VSGViewer::addObject(object);

    return true;
  }


  bool VSGViewer_Dueca::setObjectCoordinates(const std::vector<double>& coord)
  {
    build_object_spec.coordinates = coord;
    return true;
  }

  bool VSGViewer_Dueca::setObjectBehavior(const std::string& beh)
  {
    if (beh.size() != 0) {
      build_object_spec.type = beh;
      return true;
    }
    E_CNF("Specify a non-zero string for the type");
    return false;
  }

  bool VSGViewer_Dueca::addObjectClassData(const std::vector<std::string>& names)
  {
    if (names.size() < 3) {
      E_CNF("Specify at least a match name, object name and the class type");
      return false;
    }

    WorldDataSpec obj;
    obj.type = names[2];

    // check if the name is given as parent/name
    auto slash = names[1].find('/');
    if (slash != std::string::npos) {
      obj.parent = names[1].substr(0, slash);
      obj.name = names[1].substr(slash+1);
    }
    else {
      obj.name = names[1];
    }

    for (size_t ii = 3; ii < names.size(); ii++) {
      obj.filename.push_back(names[ii]);
    }

    addFactorySpec(names[0], obj);
    return true;
  }

  bool VSGViewer_Dueca::setFog(const std::vector<double>& fog)
  {
    if (fog.size() >= 1) {
      the_fog.density = fog[0];
    }
    else if (fog.size() >= 4) {
      the_fog.color = {float(fog[1]), float(fog[2]), float(fog[3]) };
    }
    else if (fog.size() >= 5) {
      the_fog.start = fog[4];
    }
    else if (fog.size() >= 6) {
      the_fog.start = fog[5];
    }
    else if (fog.size() == 7) {
      the_fog.exponent = fog[6];
    }
    else {
      E_CNF("Wrong number of arguments for for fog");
      return false;
    }
    enable_simple_fog = true;
    return true;
  }

  bool VSGViewer_Dueca::setXMLReader(const std::string& definitions)
  {
    if (xml_reader) {
      E_MOD("Error, second attempt to use set_xml_definitions or use of this "
            " call after using read_xml_defintions");
      return false;
    }
    try{
      xml_reader.reset(new VSGXMLReader(definitions));
    }
    catch (const std::exception& e) {
      E_MOD("Error in initialising XML reader: " << e.what());
      return false;
    }
    return true;
  }

  bool VSGViewer_Dueca::readModelFromXML(const std::string& file)
  {
    try {
      if (!xml_reader) {
        I_MOD("Creating default xml reader");
        xml_reader.reset
          (new VSGXMLReader("../../../../WorldView/vsg-viewer/vsgobjects.xml"));
      }
      return xml_reader->readWorld(file, *this);
    }
    catch (const std::exception& e) {
      E_MOD("Error in reading xml definitions from " << file);
      return false;
    }
  }

} // namespace

  // script access macro, needed for Scheme implementations
SCM_FEATURES_NOIMPINH(vsgviewer::VSGViewer_Dueca,
                      ScriptCreatable, "vsg-viewer-dueca");

// Make a CoreCreator object for this module, the CoreCreator
// will check in with the scheme-interpreting code, and enable the
// creation of objects of this type

#ifdef SCRIPT_PYTHON
static CoreCreator<vsgviewer::VSGViewer_Dueca>
a(vsgviewer::VSGViewer_Dueca::getParameterTable(), "VSGViewer");
#else
static CoreCreator<vsgviewer::VSGViewer_Dueca>
a(vsgviewer::VSGViewer_Dueca::getParameterTable());
#endif
