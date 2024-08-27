/* ------------------------------------------------------------------   */
/*      item            : OgreViewer_Dueca.cxx
        made by         : rvanpaassen
	from template   : DuecaModuleTemplate.cxx
        template made by: Rene van Paassen
        date            : Mon Jan 25 15:47:13 2010
	category        : body file 
        description     : 
	changes         : Mon Jan 25 15:47:13 2010 first version
	template changes: 030401 RvP Added template creation comment
        language        : C++
*/

#define OgreViewer_Dueca_cxx
// include the definition of the helper class
#include "OgreViewer_Dueca.hxx"

// include additional files needed for your calculation here
#include "OgreObjectFactory.hxx"
#include "OgreObjectCompatible.hxx"
#include "OgreObjectMoving.hxx"

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

// Parameters to be inserted
const ParameterTable* OgreViewer_Dueca::getParameterTable()
{
  static const ParameterTable parameter_table[] = {

    /* You can extend this table with labels and MemberCall or
       VarProbe pointers to perform calls or insert values into your
       class objects. Please also add a description (c-style string). */
    { "add-window", 
      new MemberCall<_ThisObject_,string>
      (&_ThisObject_::addWindow),
      "Add a window to this Scene drawer. Do not forget to specify\n"
      "window size and (optionally) position" },

    { "set-x-display",
      new MemberCall<_ThisObject_,string>
      (&_ThisObject_::setXDisplay),
      "Specify the X display (e.g. \":0.0\" for the window" },

    { "window-size+pos", 
      new MemberCall<_ThisObject_,vector<int> >
      (&_ThisObject_::setWindowPosition), 
      "specify width, height and optionally x, y position of a new window" },
    
    { "add-viewport", 
       new MemberCall<_ThisObject_,string>
      (&_ThisObject_::addViewport),
      "Add a viewport to this Scene drawer" },

    { "viewport-window", 
      new MemberCall<_ThisObject_,string>
      (&_ThisObject_::setViewportWindow), 
      "Specify the window for this viewport" },

    { "viewport-pos+size", 
      new MemberCall<_ThisObject_,vector<int> >
      (&_ThisObject_::setViewportSize),
      "Specify location (top left) and size for a viewport, 4 parameters" }, 

    { "eye-offset", 
      new MemberCall<_ThisObject_,vector<float> >
      (&_ThisObject_::setEyeOffset),
      "Offset of the eye with respect to sent position; 3 parameters for\n"
      "x, y and z location, and three parameters for phi, theta, psi [deg]" },
    
    { "viewport-overlay" , 
      new MemberCall<_ThisObject_,string>
      (&_ThisObject_::setViewportOverlay),
      "Specify an overlay for this viewport" },

    { "load-scene",
      new MemberCall<_ThisObject_,vector<string> >
      (&_ThisObject_::addScene),
      "load a scene file, specify scene file and resource group" },
    
    { "set-frustum",
      new MemberCall<_ThisObject_,vector<float> >
      (&_ThisObject_::setFrustum),
      "Set frustum geometry, <near plane> <far plane>, and then either:\n"
      "<fov y>, (degrees) or <left> <right> <bottom> <top> of the near\n"
      "frustum plane in camera coordinates" },
    
    { "sync-divisor", 
      new VarProbe<_ThisObject_,int>
      (&_ThisObject_::glx_sync_divisor),
      "For video sync wait mode, set the divisor (e.g. 2 means 30 Hz at a\n"
      "60 Hz refresh rate)" },
    
    { "sync-offset", 
      new VarProbe<_ThisObject_,int>
      (&_ThisObject_::glx_sync_offset),
      "For video sync wait mode, set the offset" },
    
    { "keep-cursor", 
      new VarProbe<_ThisObject_,bool>
      (&_ThisObject_::keep_pointer), 
      "By default, the cursor is removed. Set to true to keep the cursor" },
    
    { "add-object-class-data", 
      new MemberCall<_ThisObject_,vector<string> >
      (&_ThisObject_::addObjectClassData),
      "Create a new class (type) of simple objects, i.e. those that can be\n"
      "represented by one or more model files. Specify the following\n"
      "- <DCO classname>[:objectname]. This is used to match to either\n"
      "  DCO classname (all objects look like this) or DCO classname+label\n"
      "- Graphic object class name, accessible through the factory\n"
      "  mesh file name[s], optionally as Group/Mesh\n" },

    { "add-object-class-coordinates",
      new MemberCall<_ThisObject_,vector<double> >
      (&_ThisObject_::addCoordinates),
      "optionally add coordinates to the latest created objectclass with\n"
      "\"add-object-class-data\", example is position + (phi, theta, psi) for\n"
      "OgreObjectCarried"
    },

    { "create-static",
      new MemberCall<_ThisObject_,std::vector<std::string> >
      (&_ThisObject_::createStatic),
      "Create an object through the factory that will not receive a\n"
      "connection to a channel entry, specify object class and name" },
    
    { "add-object-class", 
      new MemberCall<_ThisObject_,vector<string> >
      (&_ThisObject_::addObjectClass),
      "Create a new class (type) of simple objects, compatible for old\n"
      "ObjectMotion types. Specify the type name, the\n"
      "mesh name and the resource group name. Based on the \"klass\"\n"
      "value in the ObjectMotion object, objects of this mesh/group will be\n"
      "created.\n" },
    
    { "add-resource-location",
      new MemberCall<_ThisObject_,vector<string> >
      (&_ThisObject_::addResourceLocation),
      "Add a new location for resources. Specify the folder or zip file, the\n"
      "resource type (FileSystem,Zip), the resource group name, optionally\n"
      "add the keyword \"recursive\"" },
    
    { "render-shadows",
      new MemberCall<_ThisObject_,bool >
      (&_ThisObject_::addShadows),
      "Add this call and set to #t when you want to render shadows in your\n"
      "scene. For now, this only is tested and works in OGRE." },
    
    
    { "set-shadow-technique",
      new MemberCall<_ThisObject_,string >
      ( &_ThisObject_::setShadowRenderer ),
      "Add this call to select the type of render technique for generating\n"
      "shadows. You can choose between Stencil and Texture.\n"
      "For now, this only is tested and works, in OGRE."
    },
    
    { "set-shadow-colour",
      new MemberCall<_ThisObject_,vector<float> >
      ( &_ThisObject_::setShadowColour ),
      "Add this call to set the shadow colour. Needs RGB values\n"
      "For now, this only is tested and works, in OGRE."
    },
    
    { "set-shadow-far-distance",
      new MemberCall<_ThisObject_, float >
      ( &_ThisObject_::setShadowFarDistance ),
      "Add this call to set the shadow far distance to clip unnecessary\n"
      "far shadow rendering. Setting this to a small value will save you a\n"
      "lot of rendering overhead. For now, this only is tested and works\n"
      "in OGRE."
    },

    { "allow-unknown",
      new VarProbe<_ThisObject_,bool>(&_ThisObject_::allow_unknown),
      "ignore unknown or unconnected objects in world information channels" },
	  
    /* The table is closed off with NULL pointers for the variable
       name and MemberCall/VarProbe object. The description is used to
       give an overall description of the module. */
    { NULL, NULL, "This helper class interfaces to Ogre"} };

  return parameter_table;
}

// constructor
OgreViewer_Dueca::OgreViewer_Dueca() :
  ScriptCreatable()
{
  
}

bool OgreViewer_Dueca::complete()
{
  if (build_win_spec.sufficient()) {
    cout << "adding last window specification" << endl;
    OgreViewer::addWindow(build_win_spec);
    build_win_spec = WinSpec();
  } 
  
  if (build_view_spec.sufficient()) {
    cout << "adding last viewport specification" << endl;
    OgreViewer::addViewport(build_view_spec);
    build_view_spec = ViewSpec();
  }
  else {
    E_CNF("Viewport specification not complete");
    return false;
  }
  

  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */
  return OgreViewer::complete();
}

// destructor
OgreViewer_Dueca::~OgreViewer_Dueca()
{
  //
}

bool OgreViewer_Dueca::addWindow(const std::string& window)
{
  // check whether there is a complete window
  if (build_win_spec.sufficient()) {
    OgreViewer::addWindow(build_win_spec);
    build_win_spec = WinSpec();
  }
  
  build_win_spec.name = window;
  return true;
}

bool OgreViewer_Dueca::setWindowPosition(const std::vector<int>& pos)
{
  if (pos.size() != 2 && pos.size() != 4) {
    E_CNF("Need 2 or 4 parameters for window size and position");
    return false;
  }
  build_win_spec.size_and_position = pos;
  return true;
}

bool  OgreViewer_Dueca::setXDisplay(const std::string& display)
{
  build_win_spec.display = display;
  return true;
}

bool OgreViewer_Dueca::addViewport(const std::string& viewport)
{
  // check whether there is a complete window
  if (build_view_spec.sufficient()) {
    OgreViewer::addViewport(build_view_spec);
    build_view_spec = ViewSpec();
  }
  build_view_spec.name = viewport;
  return true;
}

bool OgreViewer_Dueca::setViewportWindow(const std::string& vp_window)
{
  build_view_spec.winname = vp_window;
  return true;
}
  
bool OgreViewer_Dueca::setViewportSize(const std::vector<int>& vpwin)
{
  if (vpwin.size() != 4) {
    E_CNF("Need for parameter for viewport size");
    return false;
  }
  build_view_spec.portcoords = vpwin;
  return true;
}

bool OgreViewer_Dueca::setViewportOverlay(const std::string& overlay)
{
  build_view_spec.overlay = overlay;
  return true;
}

bool OgreViewer_Dueca::setFrustum(const std::vector<float>& frustum)
{
  if (frustum.size() != 3 && frustum.size() != 6) {
    E_CNF("Need 3 or 6 parameters for frustum size");
    return false;
  }
  build_view_spec.frustum_data = frustum;
  return true;
}

bool OgreViewer_Dueca::setEyeOffset(const std::vector<float>& eye)
{
  if (eye.size() != 3 && eye.size() != 6) {
    E_CNF("Need 3 or 6 parameters for eye offset");
    return false;
  }
  build_view_spec.eye_pos = eye;
  return true;
}

bool OgreViewer_Dueca::addObjectClass(const std::vector<std::string>& names)
{
  // old, for compatibility with ObjectMotion use, with "klass" and "name"
  // information in object data, rather than in label
  
  if (names.size() < 2 || names.size() > 3) {
    E_CNF("Specify a DCO class name, object class name and" <<
          " mesh and group");
    return false;
  }
  OgreObjectCompatible::typemap[names[0]] = (names.size() == 2) ?
    OgreObjectCompatible::MeshAndGroup(names[1]) :
    OgreObjectCompatible::MeshAndGroup(names[1], names[2]);

  if (!hasFactorySpec("ObjectMotion")) {
    WorldDataSpec obj;
    obj.type = "OgreObjectCompatible";
    obj.name = "compatible #";
    addFactorySpec("ObjectMotion", obj);
  }

  return true;
}

bool OgreViewer_Dueca::addObjectClassData(const std::vector<std::string>& names)
{
  if (names.size() < 3) {
    E_CNF("Specify a match string for creation, object name, factory type and"
          << " zero or more mesh/group combinations");
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

#if 0
bool OgreViewer_Dueca::addObjectClassCoordinates
(const std::vector<double>& coords)
{
  return 
  if (latest_classdata.size() == 0) {
    E_CNF("First call \"add-object-class-data\"");
    return false;
  }
  
  factoryspecs[latest_classdata].coordinates = coords;
  return true;
}
#endif

bool OgreViewer_Dueca::addScene(const std::vector<std::string>& sc)
{
  if (sc.size() != 2 && sc.size() != 1) {
    E_CNF("Specify scene file name and optionally a group name");
    return false;
  }
  if (sc.size() == 1)
    return OgreViewer::addScene(sc[0], "General");
  else
    return OgreViewer::addScene(sc[0], sc[1]);
}

bool OgreViewer_Dueca::addResourceLocation(const std::vector<std::string>& sc)
{
  if (sc.size() != 3 && sc.size() != 4) {
    E_CNF("Specify 3 or 4 string arguments");
  }
  resourcespec.push_back
    (ResourceSpec(sc[0], sc[1], sc[2], sc.size() == 4 && 
		  (sc[3] == "recursive")));
  return true;
}

bool OgreViewer_Dueca::addShadows(const bool& render_shadows)
{
  build_view_spec.render_shadows = render_shadows;
  return true;
}
  
bool OgreViewer_Dueca::setShadowRenderer(const string& render_technique)
{
	if( (render_technique == "stencil") || (render_technique == "Stencil") || (render_technique == "STENCIL") )
	{
		build_view_spec.shadow_technique = "stencil";
	}
	else if ((render_technique == "texture") || (render_technique == "Texture") || (render_technique == "TEXTURE") )
	{
		build_view_spec.shadow_technique = "texture";
	}
	else
	{
		E_CNF("set-shadow-technique unknown shadow technique. Choices are Texture or Stencil");
	}
	return true;
}
  
bool OgreViewer_Dueca::setShadowColour(const std::vector<float>& colour)
{
	if(colour.size() != 3)
	{
		E_CNF("Wrong number of arguments for set-shadow-colour. Requires three arguments, one for each (r,g,b) colour value!");
	}
	else
	{
		build_view_spec.shadow_colour = colour;
	}
	return true;
}
  
bool OgreViewer_Dueca::setShadowFarDistance(const float& distance)
{
	if(distance<0)
	{
		E_CNF("Invalid shadow far distance. set-shadow-far-distance needs to be >= zero!");
	}
	else
	{
		build_view_spec.shadow_farDistance = distance;
	}
	return true;
}

// script access macro
SCM_FEATURES_NOIMPINH(OgreViewer_Dueca, ScriptCreatable, "ogre-viewer-dueca");

// Make a CoreCreator object for this module, the CoreCreator
// will check in with the scheme-interpreting code, and enable the
// creation of objects of this type
#ifdef SCRIPT_PYTHON
static CoreCreator<OgreViewer_Dueca> a(OgreViewer_Dueca::getParameterTable(),
				       "OgreViewer");
#else
static CoreCreator<OgreViewer_Dueca> a(OgreViewer_Dueca::getParameterTable());
#endif
