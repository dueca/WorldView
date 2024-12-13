/* ------------------------------------------------------------------   */
/*      item            : VSGViewer_Dueca.hxx
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

#ifndef VSGViewer_Dueca_hxx
#define VSGViewer_Dueca_hxx

// include the dueca header
#include <ScriptCreatable.hxx>
#include <stringoptions.h>
#include <ParameterTable.hxx>
#include <dueca_ns.h>
#include "VSGViewer.hxx"
#include "VSGXMLReader.hxx"
#include "comm-objects.h"

USING_DUECA_NS;

namespace vsgviewer {

  /** This class implements the DUECA interface for the VSGViewer 3D
      graphics interface class.

      VSGViewer uses VulkanSceneGraph to produce a 3D visualisation. It
      has the following functionalities:

      <ol>

      <li> Create a view on a 3D world, following the viewpoint
      controlled by its user.

      <li> Add visible objects of class VSGObject to this world. These
      objects can have any form, they may be overlays or 3D models. With
      overlays one can add instrument panels, masks and the like to the
      drawing, and the 3D models may be static, for creating lights and
      scenery, or controlled by external data, for creating other moving
      objects in the world.

      <li> Report keyboard and cursor events back to the user.

      </ol>

      All object adding uses a standard mechanism. Object creation uses
      a factory of object types; this VSGViewer implementation comes
      with a number of standard types, but the factory pattern is
      extensible, and one may, e.g., create a specific instrument
      overlay type.

      In the script interface, a link is created between a match name,
      the factory class type, and supplemental files and coordinates.
      These are stored into a "factory inventory", that can be
      subsequently used to explicitly create static objects, or its
      contents may match against entries in DUECA channels for the
      creation of dynamic, controlled objects.

      Two examples:

      * Object match string "static:world", object name "world",
      with factory type "static",
      uses file "schiphol.vsg" and is located at coordinates 0 0 0

      * Object class string "BaseObjectMotion:KLMBoeing737", object name
      "KLM737 #", with factory type "moving", uses file "KLM737.vsg",
      no coordinates given, since the BaseObjectMotion DCO object in the
      connected DUECA channel will provide position information.

      The static world can be created from the script, using the argument
      "add_static" (Python) or 'add-static (Scheme). e.g.

      @code
      'add-static "static:world"
      @endcode

      In this case, the factory inventory is searched for the class
      "static:world". The parameters for that class are found and used to
      create a static object, with the name given ("world"). It is also
      possible to add a second argument to 'add-static, to override the
      name, e.g. to make a row of houses and give them all different names.

      For dynamic objects, the channel "ObjectMotion://world" channel is
      monitored. If an entry is published in that channel with data class
      BaseObjectMotion and label KLMBoeing737, that entry will match to
      the object class name. The corresponding data ("KLM737.vsg") is used
      to create the object. Since the name ends with a # token, the name
      will be modified to include an integer suffix, e.g. "KLM737 #1"

      An entry with a class *derived* from BaseObjectMotion will also match,
      if it does not first find a match with its derived class.

      An entry with a specific label written as "KLMBoeing737|PH-ANH" will
      also match, the match will be on the first part of the label, and the
      name will be overwritten as "PH-ANH".

      Note that you are not limited to working with BaseObjectMotion or
      similar classes. You could easily add an entry with "MyHUDData" in
      it, program an overlay class, add it to the factory, and use that
      one.

      Some common uses of this VSGViewer interface are the following:

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
      the object coordinates that are true 0.0 stay centered, the others
      are fixed to the world. E.g., with coordinates 0.0, 0.0, 1e-20 you
      can create a skydome that moves with the observer in x and y
      coordinates, but keeps its height.

      <li> Adding objects that stay centered on the observer with a
      tiling step, for example a tiled floor, a repeating grassland
      pattern, etc. See the description below for the interpretation
      of coordinates.

      <li> Adding overlays, give the object a name and a file name, and
      use 'object-coordinates to specify a list with viewports the overlay
      should be on.

      <li> Adding a class of simple objects that can be represented by a
      single 3d model, use 'add-object-class with the name for the class
      and a name for the model file. This adds to the "repertoire" of
      objects that can be created.

      <li> Add another object alltogether, with the

      </ol>

      The channel for the "other" objects in the external world that is
      read by the WorldView module and fed to the VSGViewer module
      contains class names for the type of these objects. These objects
      are automatically added to the drawn environment, based on the
      data in the channel. Use 'add-object-class to define these classes
      for simple objects defined by a single 3D file. However, it is
      also possible to add new classes programmatically. For that,
      include VSGObjectFactory.hxx, and create a SubContractor with the
      VSG model factory.

      This class has been derived from the ScriptCreatable base class,
      and has a (scheme) script command to create it and optionally add
      parameters. This class encapsulates the VSGViewer objects, in
      this way these can be made and specified from a DUECA script.

      The instructions to create an object of this class from the Scheme
      script are:

      \verbinclude vsg-viewer-dueca.scm
  */
  class VSGViewer_Dueca: public ScriptCreatable, public VSGViewer
  {
  private: // simulation data
    /** self-define the module type, to ease writing the parameter table */
    typedef VSGViewer_Dueca _ThisObject_;

  public: // construction and further specification
    /** Constructor. Is normally called from scheme/the creation script. */
    VSGViewer_Dueca();

    /** Continued construction. This is called after all script
        parameters have been read and filled in, according to the
        parameter table. */
    bool complete();

    /** Destructor. */
    ~VSGViewer_Dueca();

    /** Obtain a pointer to the parameter table. */
    static const ParameterTable* getParameterTable();

  private:
    /** Specification of a viewport */
    ViewSpec   build_view_spec;

    /** Specification of a window */
    WinSpec    build_win_spec;

    /** Specification of a visual object in the world */
    WorldDataSpec build_object_spec;

    /** Reader for xml definitions */
    boost::scoped_ptr<VSGXMLReader>   xml_reader;

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
    bool addScriptObject(ScriptCreatable& ava, bool in);

    /** Set the coordinates for an object in the world */
    bool setObjectCoordinates(const std::vector<double>& coord);

    /** Define the behaviour wrt the observer */
    bool setObjectBehavior(const std::string& beh);

    /** Add a possible class of object to the world. */
    bool addObjectClass(const std::vector<std::string>& names);

    /** Add a possible class of object to the world. */
    bool addObjectClassData(const std::vector<std::string>& names);

    /** Set the fog parameters */
    bool setFog(const std::vector<double>& fog);

    /** Create the XML reader */
    bool setXMLReader(const std::string& definitions);

    /** Read an XML file with object data */
    bool readModelFromXML(const std::string& file);

  public:
    /** Default script linkage. */
    SCM_FEATURES_DEF;

  };

} // namespace

#endif
