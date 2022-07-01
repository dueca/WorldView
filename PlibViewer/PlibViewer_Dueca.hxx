/* ------------------------------------------------------------------   */
/*      item            : PlibViewer_Dueca.hxx
        made by         : rvanpaassen
	from template   : DuecaHelperTemplate.hxx
        template made by: Rene van Paassen
        date            : Tue Jan 26 15:25:27 2010
	category        : header file 
        description     : 
	changes         : Tue Jan 26 15:25:27 2010 first version
	template changes: 050825 RvP Added template creation comment
        language        : C++
*/

#ifndef PlibViewer_Dueca_hxx
#define PlibViewer_Dueca_hxx

// include the dueca header
//#include <ScriptCreatable.hxx>
//#include <stringoptions.h>
//#include <ParameterTable.hxx>
#include "PlibViewer.hxx"
#include <dueca_ns.h>
#include "comm-objects.h"

USING_DUECA_NS;

/** This class implements the DUECA interface for the PlibViewer 3D
    graphics interface class.

    PlibViewer uses plib to produce a 3D visualisation. It has the
    following functionalities:

    <ol>
    <li> Create a view on 3D world, following the viewpoint
    controlled by its user. 
    <li> Add visible objects of class PlibObject to this world. These
    objects can have any form, they may be overlays or 3D models. With
    overlays one can add instrument panels, masks and the like to the
    drawing, and the 3D models may be static or controlled by external
    data.
    <li> Report keyboard and cursor events back to the user. 
    </ol>

    Some common uses of this PlibViewer interface are the following:

    <ol>

    <li> Adding windows and specifying viewports in the interface, see
    add-window and add-viewport, and the instructions below. Each
    viewport can have its own frustum or view defined, and each can
    also have an eye offset, defining the position and orientation of
    the eye with respect to the own vehicle position or base pilot
    position.

    <li> Adding static objects in the world. Use the 'load-object
    specification, specify a name for the object and a 3d file, and
    use 'object-type "static".
    
    <li> Adding objects that stay centered on the observer, like a
    skydome. Use 'object-type "centered", and 'object-coordinates . 
    the object coordinates that are true 0.0 stay centered. 
    
    <li> Adding objects that stay centered on the observer with a
    tiling step, for example a tiled floor, a repeating grassland
    pattern, etc. 

    <li> Adding overlays, give the object a name and a file name, and
    use 'object-coordinates to specify a list with viewports the overlay
    should be on. 

    <li> Adding a class of simple objects that can be represented by a
    single 3d model, use 'add-object-class with the name for the class
    and a name for the model file. This adds to the "repertoire" of
    objects that can be created. 
    
    </ol>

    The channel for the "other" objects in the external world that is
    read by the WorldView module and fed to the PlibViewer module
    contains class names for the type of these objects. These objects
    are automatically added to the drawn environment, based on the
    data in the channel. Use 'add-object-class to define these classes
    for simple objects defined by a single 3D file. However, it is
    also possible to add new classes programmatically. For that,
    include PlibObjectFactory.hxx, and create a SubContractor with the
    plib model factory. 

    This class has been derived from the ScriptCreatable base class,
    and has a (scheme) script command to create it and optionally add
    parameters. This class encapsulates the PlibViewer objects, in
    this way these can be made and specified from a DUECA script.

    The instructions to create an object of this class from the Scheme
    script are:

    \verbinclude plib-viewer-dueca.scm
 */
class PlibViewer_Dueca: public ScriptCreatable, public PlibViewer
{
public: // construction and further specification
  /** Constructor. Is normally called from scheme/the creation script. */
  PlibViewer_Dueca();

  /** Continued construction. This is called after all script
      parameters have been read and filled in, according to the
      parameter table. */
  bool complete();

  /** Destructor. */
  ~PlibViewer_Dueca();

  /** Obtain a pointer to the parameter table. */
  static const ParameterTable* getParameterTable();

private:
  /** Specification of a viewport */
  ViewSpec   build_view_spec;

  /** Specification of a window */
  WinSpec    build_win_spec;

  /** Specification of a visual object in the world */
  WorldDataSpec build_object_spec;

  /** Specify a window */
  bool addWindow(const std::string& window);

  /** Window position */
  bool setWindowPosition(const std::vector<int>& pos);

  /** X screen selection */
  bool setWindowXScreen(const std::string& scr);
 
  /** Specify a viewport */
  bool addViewport(const std::string& viewport);
  
  /** Viewport window */
  bool setViewportWindow(const std::string& vp_window);
  
  /** Viewport size */
  bool setViewportSize(const std::vector<int>& vpwin_size);

  /** frustum data */
  bool setFrustum(const std::vector<float>& frustum);

  /** eye offset data */
  bool setEyeOffset(const std::vector<float>& frustum);

  /** Add an object in the world. */
  bool addObject(const std::vector<std::string>& names);
  
  /** Add an object in the world. */
  bool addScriptObject(ScriptCreatable& ava, bool in);
  
  /** Set the coordinates for an object in the world */
  bool setObjectCoordinates(const std::vector<double>& coord);

  /** Define the behaviour wrt the observer */
  bool setObjectBehavior(const std::string& beh);

  /** Add a possible class of object to the world. */
  bool addObjectClass(const std::vector<std::string>& names);

public:
  /** Default script linkage. */
  SCM_FEATURES_DEF;
 
};

#endif
