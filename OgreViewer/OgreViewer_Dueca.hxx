/* ------------------------------------------------------------------   */
/*      item            : OgreViewer_Dueca.hxx
        made by         : rvanpaassen
	from template   : DuecaHelperTemplate.hxx
        template made by: Rene van Paassen
        date            : Mon Jan 25 15:47:13 2010
	category        : header file 
        description     : 
	changes         : Mon Jan 25 15:47:13 2010 first version
	template changes: 050825 RvP Added template creation comment
        language        : C++
*/

#ifndef OgreViewer_Dueca_hxx
#define OgreViewer_Dueca_hxx

#include "OgreViewer.hxx"

// include the dueca header
#include <ScriptCreatable.hxx>
#include <stringoptions.h>
#include <ParameterTable.hxx>
#include <dueca_ns.h>

USING_DUECA_NS;

/** A class definition for a DUECA helper class
    
    This class has been derived from the ScriptCreatable base class,
    and has a (scheme) script command to create it and optionally add
    parameters. This class encapsulates the OgreViewer objects, in
    this way these can be made from a dueca script. 

    The helper class controls the parent OgreViewer class. There are a
    number of steps to take before you can produce a view:

    <ol>
    <li> Create one or more windows; set window size and optionally position 

    <li> Within the windows, select viewports

    <li> Each viewport can have an eye offset; basically from the
         "vehicle" or "head" position you send, a viewport can look
         more left, right, etc.
    
    <li> Each viewport needs a frustum definition

    <li> Load a scene file with the world

    <li> Add animated objects to the world. There are a number of additional
         objects:

         <ul>

         <li> Single-mesh objects, controlled with a BaseObjectMotion
              entry in the ObjectMotion://world channel. To define
              such an object use the "add-object-class-data"
              parameter. The first argument to that parameter defines
              the match, initial match will be on
              BaseObjectMotion:<label>, but a match can also be
              against any BaseObjectMotion DCO object. The second must
              then be OgreObjectMoving, and the third is the
              group/mesh combination.

         <li> Compatibility objects, controlled with the old ObjectMotion 
              entry. These have a "klass" and a "name" parameter, the "klass"
              parameter will provide the link to the mesh, use the 
              "add-object-class" parameter. 
              
         <li> Carried objects. These move or are placed relative to the 
              viewpoints. In that case, use an OgreObjectCarried class to 
              "add-object-class-data", and specify like normal single-mesh 
              objects. 
         
         <li> Custom objects. It is possible to add objects to this system,
              using the SubContractor scheme. 
     
         </ul>

    </li>

    The instructions to create an object of this class from the Scheme
    or Python script are:

    \verbinclude ogre-viewer-dueca.scm
    
 */
class OgreViewer_Dueca: public ScriptCreatable, public OgreViewer
{
private: // simulation data
  /** self-define the module type, to ease writing the parameter table */
  typedef OgreViewer_Dueca _ThisObject_;
  
public: // construction and further specification
  /** Constructor. Is normally called from scheme/the creation script. */
  OgreViewer_Dueca();

  /** Continued construction. This is called after all script
      parameters have been read and filled in, according to the
      parameter table. */
  bool complete();

  /** Destructor. */
  ~OgreViewer_Dueca();

  /** Obtain a pointer to the parameter table. */
  static const ParameterTable* getParameterTable();

private:
  /** Specification of a viewport */
  ViewSpec   build_view_spec;

  /** Specification of a window */
  WinSpec    build_win_spec;

  /** Specify a window */
  bool addWindow(const std::string& window);

  /** Specify the x display */
  bool setXDisplay(const std::string& display);

  /** Window position */
  bool setWindowPosition(const std::vector<int>& pos);

  /** Specify a viewport */
  bool addViewport(const std::string& viewport);
  
  /** Viewport window */
  bool setViewportWindow(const std::string& vp_window);
  
  /** Viewport size */
  bool setViewportSize(const std::vector<int>& vpwin_size);

  /** Overlay on this viewport */
  bool setViewportOverlay(const std::string& overlay);

  /** frustum data */
  bool setFrustum(const std::vector<float>& frustum);

  /** eye offset data */
  bool setEyeOffset(const std::vector<float>& frustum);

  /** Linking objects to movement data */
  bool addObjectClass(const std::vector<std::string>& link);

  /** Currently handled objectclass */
  std::string latest_classdata;
  
  /** Linking objects to movement data */
  bool addObjectClassData(const std::vector<std::string>& link);

  /** Linking objects to movement data */
  bool addObjectClassCoordinates(const std::vector<double>& l);
  
  /** Add a scene */
  bool addScene(const std::vector<std::string>& sc);
  
  /** Add a resource location */
  bool addResourceLocation(const std::vector<std::string>& rl);
  
  /** Set rendering of shadows on (TRUE) or off (FALSE). Only tested and working for OGRE for now */
  bool addShadows(const bool& render_shadows);
  
  /** set the shadow renderer technique you want to use - STENCIL or TEXTURE */
  bool setShadowRenderer(const string& render_technique);
  
  /** set the shadow colour you want, using r, g, b values to set the colour */
  bool setShadowColour(const std::vector<float>& colour);
  
  /** set the shadow render distance */
  bool setShadowFarDistance(const float& distance);
  
public:
  /** Default script linkage. */
  SCM_FEATURES_DEF;
};

#endif
