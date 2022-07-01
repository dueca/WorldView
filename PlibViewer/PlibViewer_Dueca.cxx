/* ------------------------------------------------------------------   */
/*      item            : PlibViewer_Dueca.cxx
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

// include the definition of the helper class
#include "PlibViewer_Dueca.hxx"
#include <dueca-version.h>

// include the debug writing header, by default, write warning and 
// error messages
#define W_MOD
#define E_MOD
#include <debug.h>

// include additional files needed for your calculation here
#include "PlibObjectFactory.hxx"

#define DO_INSTANTIATE
#include <VarProbe.hxx>
#include <MemberCall.hxx>
#include <MemberCall2Way.hxx>
#include <CoreCreator.hxx>
USING_DUECA_NS;

// Parameters to be inserted
const ParameterTable* PlibViewer_Dueca::getParameterTable()
{
  static const ParameterTable parameter_table[] = {

    /* You can extend this table with labels and MemberCall or
       VarProbe pointers to perform calls or insert values into your
       class objects. Please also add a description (c-style string). */
    { "set-modelpath", 
      new VarProbe<PlibViewer_Dueca,string>
      (&PlibViewer_Dueca::modelpath),
      "set the path to the models" },

    { "set-texturepath", 
      new VarProbe<PlibViewer_Dueca,string>
      (&PlibViewer_Dueca::texturepath),
      "set the path to the texture files" },

    { "sync-divisor", 
      new VarProbe<PlibViewer_Dueca,int>
      (&PlibViewer_Dueca::glx_sync_divisor),
      "For video sync wait mode, set the divisor (e.g. 2 means 30 Hz at a\n"
      "60 Hz refresh rate)" },

    { "sync-offset", 
      new VarProbe<PlibViewer_Dueca,int>
      (&PlibViewer_Dueca::glx_sync_offset),
      "For video sync wait mode, set the offset" },

    { "keep-cursor", 
      new VarProbe<PlibViewer_Dueca,bool>
      (&PlibViewer_Dueca::keep_pointer), 
      "By default, the cursor is removed. Set to true to keep the cursor" },

    { "add-window", 
      new MemberCall<PlibViewer_Dueca,string>
      (&PlibViewer_Dueca::addWindow),
      "Add a window to this Scene drawer. Do not forget to specify\n"
      "window size and (optionally) position" },
    
    { "window-size+pos", 
      new MemberCall<PlibViewer_Dueca,vector<int> >
      (&PlibViewer_Dueca::setWindowPosition), 
      "specify width, height and optionally x, y position of a new window" },
    
    { "set-x-display",
      new MemberCall<PlibViewer_Dueca,string>
      (&PlibViewer_Dueca::setWindowXScreen),
      "Specify the X display (e.g. \":0.0\" for the window" },

    { "add-viewport", 
       new MemberCall<PlibViewer_Dueca,string>
      (&PlibViewer_Dueca::addViewport),
      "Add a viewport to this Scene drawer" },

    { "viewport-window", 
      new MemberCall<PlibViewer_Dueca,string>
      (&PlibViewer_Dueca::setViewportWindow), 
      "Specify the window for this viewport" },

    { "viewport-pos+size", 
      new MemberCall<PlibViewer_Dueca,vector<int> >
      (&PlibViewer_Dueca::setViewportSize),
      "Specify location (top left) and size for a viewport, 4 parameters" }, 

    { "eye-offset", 
      new MemberCall<PlibViewer_Dueca,vector<float> >
      (&PlibViewer_Dueca::setEyeOffset),
      "Offset of the eye with respect to sent position; 3 parameters for\n"
      "x, y and z location, and three parameters for phi, theta, psi [deg]" },
    
    { "object-coordinates",
      new MemberCall<PlibViewer_Dueca,vector<double> >
      (&PlibViewer_Dueca::addCoordinates),
      "Set coordinates for the object; x, y, z position, and phi, theta, psi\n"
      "orientation. Note that these may be used differently, depending on\n"
      "the behaviour specified" },
    
    { "object-class", 
      new MemberCall<PlibViewer_Dueca,string >
      (&PlibViewer_Dueca::setObjectBehavior),
      "set the type of an added object, currently the following are\n"
      "available: 'static', 'centered' or 'tiled'\n"
      "static: the coordinates give the object position and orientation\n"
      "centered: non-zero x, y, z give object position, zero x, y or z means\n"
      "          the object stays centered on the observer\n"
      "tiled: zero x, y, z mean that that coordinate is absolute, fixed in\n"
      "       the world, negative means that coordinate follows observer\n"
      "       positive defines the jump with which the object follows the\n"
      "       observer\n"
      "For all three object types you can also add sx, sy, sz scale factors" },

    { "add-object-class", 
      new MemberCall<PlibViewer_Dueca,vector<string> >
      (&PlibViewer_Dueca::addObjectClass),
      "Create a new class of objects. i.e. those that can be\n"
      "represented by one or more model files. Specify the following\n"
      "- <DCO classname>[:objectname]. This is used to match to either\n"
      "  DCO classname (all objects look like this) or DCO classname+label\n"
      "- the type of an added object, currently the following are\n"
      "  available: 'static', 'centered' or 'tiled'\n"
      "  static: the coordinates give the object position and orientation\n"
      "  centered: non-zero x, y, z give object position, zero x, y or z\n"
      "            means the object stays centered on the observer\n"
      "  tiled: zero x, y, z mean that that coordinate is absolute, fixed in\n"
      "       the world, negative means that coordinate follows observer\n"
      "       positive defines the jump with which the object follows the\n"
      "       observer\n"
      "- the necessary file names for the object type graphics" },

    { "set-frustum",
      new MemberCall<PlibViewer_Dueca,vector<float> >
      (&PlibViewer_Dueca::setFrustum),
      "Set frustum geometry, <near plane> <far plane>, and then either:\n"
      "<fov y> for specifying viewing with only a field-o-view angle, or\n"
      "<left> <right> <bottom> <top> of the near frustum plane\n"
      "in camera coordinates" },

    { "create-static",
      new MemberCall<PlibViewer_Dueca,std::vector<std::string> >
      (&PlibViewer_Dueca::createStatic),
      "Create a static object through the factory that will not receive a\n"
      "connection to a channel entry, specify object classname, matching the\n"
      "name " },
#if 0
    { "add-external-object",
      new MemberCall2Way<PlibViewer_Dueca,ScriptCreatable> 
      (&PlibViewer_Dueca::addScriptObject),
      "Add a graphic object created in the script file. The object must\n"
      "derive from PlibObject." },
#endif
    { "set-bg-color", 
      new VarProbe<PlibViewer_Dueca,std::vector<double> >
      (&PlibViewer_Dueca::bg_color), 
      "set the background color, R, G, B components, scaled 0 -- 1" },

    /* The table is closed off with NULL pointers for the variable
       name and MemberCall/VarProbe object. The description is used to
       give an overall description of the module. */
    { NULL, NULL, 
      "Helper object that implements a view (or more) with the use of PLIB\n"
      "as scene manager. To be used by a world-view module"} 
  };

  return parameter_table;
}

// constructor
PlibViewer_Dueca::PlibViewer_Dueca() :
  ScriptCreatable()
{
  
}

bool PlibViewer_Dueca::complete()
{
  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */
  if (bg_color.size() != 3) {
    E_CNF("Need 3 components for background color");
    return false;
  }
  
  if (build_win_spec.sufficient()) {
    cout << "adding last window specification" << endl;
    PlibViewer::addWindow(build_win_spec);
    build_win_spec = WinSpec();
  } 
  
  if (build_view_spec.sufficient()) {
    cout << "adding last viewport specification" << endl;
    PlibViewer::addViewport(build_view_spec);
    build_view_spec = ViewSpec();
  }
  else {
    E_CNF("Viewport specification not complete");
    return false;
  }

  return true;
}

// destructor
PlibViewer_Dueca::~PlibViewer_Dueca()
{
  //
}

bool PlibViewer_Dueca::addWindow(const std::string& window)
{
  // check whether there is a complete window
  if (build_win_spec.sufficient()) {
    PlibViewer::addWindow(build_win_spec);
    build_win_spec = WinSpec();
  }
  
  build_win_spec.name = window;
  return true;
}

bool PlibViewer_Dueca::setWindowPosition(const std::vector<int>& pos)
{
  if (pos.size() != 2 && pos.size() != 4) {
    E_CNF("Need 2 or 4 parameters for window size and position");
    return false;
  }
  build_win_spec.size_and_position = pos;
  return true;
}
  
bool PlibViewer_Dueca::setWindowXScreen(const std::string& scr)
{
  build_win_spec.display = scr;
  return true;
}

bool PlibViewer_Dueca::addViewport(const std::string& viewport)
{
  // check whether there is a complete window
  if (build_view_spec.sufficient()) {
    PlibViewer::addViewport(build_view_spec);
    build_view_spec = ViewSpec();
  }
  build_view_spec.name = viewport;
  return true;
}

bool PlibViewer_Dueca::setViewportWindow(const std::string& vp_window)
{
  build_view_spec.winname = vp_window;
  return true;
}
  
bool PlibViewer_Dueca::setViewportSize(const std::vector<int>& vpwin)
{
  if (vpwin.size() != 4) {
    E_CNF("Need four parameter for viewport size");
    return false;
  }
  build_view_spec.portcoords = vpwin;
  return true;
}

bool PlibViewer_Dueca::setFrustum(const std::vector<float>& frustum)
{
  if (frustum.size() != 3 && frustum.size() != 6) {
    E_CNF("Need 3 or 6 parameters for frustum size");
    return false;
  }
  build_view_spec.frustum_data = frustum;
  return true;
}

bool PlibViewer_Dueca::setEyeOffset(const std::vector<float>& eye)
{
  if (eye.size() != 3 && eye.size() != 6) {
    E_CNF("Need 3 or 6 parameters for eye offset");
    return false;
  }
  build_view_spec.eye_pos = eye;
  return true;
}

bool PlibViewer_Dueca::setObjectBehavior(const std::string& beh)
{
  if (beh.size() != 0) {
    build_object_spec.type = beh;
    return true;
  }
  E_CNF("Specify a non-zero string for the type");
  return false;
}

#if 0
class SimplePlibObject: public PlibObject
{
public:
  SimplePlibObject(const string& name, const string& filename) 
  {
    this->modelfile = filename;
    this->name = name;
  }
};

class SimplePlibSubcontractor: public SubcontractorBase<PlibObjectTypeKey>
{
  /** File with the graphic model */
  string filename;

public:
  /** Constructor */
  SimplePlibSubcontractor(const string& filename) :
    filename(filename) { }
  
  /** create the object */
  PlibObjectTypeKey::ProductBase create
  (const PlibObjectTypeKey::Key& key, 
   const PlibObjectTypeKey::SpecBase& spec) {
    return new SimplePlibObject(key, filename);
  }
};
#endif

bool PlibViewer_Dueca::addObjectClass(const std::vector<std::string>& names)
{
  if (names.size() < 3) {
    E_CNF("Specify a match string for creation, object class name and" <<
          " one or more mesh/group combinations");
    return false;
  }

  WorldDataSpec obj;
  obj.type = names[1];
  for (size_t ii = 2; ii < names.size(); ii++) {
    obj.filename.push_back(names[ii]);
  }

  addFactorySpec(names[0], obj);
  return true;
}


// script access macro
SCM_FEATURES_NOIMPINH(PlibViewer_Dueca, ScriptCreatable, "plib-viewer-dueca");

// Make a CoreCreator object for this module, the CoreCreator
// will check in with the scheme-interpreting code, and enable the
// creation of objects of this type
#ifdef SCRIPT_PYTHON
static CoreCreator<PlibViewer_Dueca> a(PlibViewer_Dueca::getParameterTable(),
				       "PlibViewer");
#else
static CoreCreator<PlibViewer_Dueca> a(PlibViewer_Dueca::getParameterTable());
#endif

