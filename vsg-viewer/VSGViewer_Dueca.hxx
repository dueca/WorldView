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
    objects can have any form, usually transforms and 3D models.

    <li> Specify how channel entries in channels defining visual
    objects should be visualised.

    <li> Report keyboard and cursor events back to the user.
    </ol>

    All object adding uses a standard mechanism. Object creation uses
    a factory of object types; this VSGViewer implementation comes
    with a number of standard types, but the factory pattern is
    extensible, and one may, e.g., create a specific instrument
    overlay type.

    The visual world is defined in an xml-formatted script. This includes
    both static objects, and templates for creating/moving objects
    when entries in the channels driving the visualization appear.

    The types of objects or templates are provided by the VSGObject
    factory. Through extending this factory, new user-defined types
    can be added

    Two examples:

    * A static model, placement defined by a static transform, and
      defined by a single visual file:
    @code{.xml}
    <static name="schiphol" type="static-model">
      <file>schiphol.vsg</file>
    </static>
    <static name="model-placement" root="true">
      <param name="xyz">0, 0, 0</param>
      <child>schiphol</child>
    </static>
    @endcode

    uses file "schiphol.vsg" and is located at coordinates 0 0 0

    * Objects matching "BaseObjectMotion:KLMBoeing737", object name
    "KLM737 #", use file "KLM737.vsg",
    no coordinates given, since the BaseObjectMotion DCO object in the
    connected DUECA channel will provide position information. The # will
    be replaced by a number.

    @code
    <template key="BaseObjectMotion:KLMBoeing737" type="model" name="KLM737 #">
      <file>KLM737.vsg</file>
    </template>
    @endcode

    For dynamic objects, the channel "ObjectMotion://world" channel is
    by default monitored. If an entry is published in that channel with
    data class BaseObjectMotion and label KLMBoeing737, that entry will
    match to the template key. The corresponding file ("KLM737.vsg") is used
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

    <li> Adding static objects in the world by defining these in the
    world xml.

    <li> Adding objects that stay centered on the observer, like a
    skydome. Use type="centered-transform", and position ("xyz"),
    parameters. Parameters that are truly 0 stay centered, the others
    are fixed to the world. E.g., with coordinates 0.0, 0.0, 1e-20 you
    can create a skydome that moves with the observer in x and y
    coordinates, but keeps its height.

    <li> Adding objects that stay centered on the observer with a
    tiling step, for example a tiled floor, a repeating grassland
    pattern, etc. See the description below for the interpretation
    of coordinates. (type="tiled-transform")

    </ol>

    The channel for the "other" objects in the external world that is
    read by the WorldView module and fed to the VSGViewer module
    contains class names for the type of these objects. These objects
    are automatically added to the drawn environment, based on the
    data in the channel. Use a `<template>` to  define these classes
    for simple objects defined by a single 3D file. However, it is
    also possible to add new classes programmatically. For that,
    include VSGObjectFactory.hxx, and create a SubContractor with the
    VSG model factory.

    This class has been derived from the ScriptCreatable base class,
    and has a script command to create it and optionally add
    parameters. This class encapsulates the VSGViewer objects, in
    this way these can be made and specified from a DUECA script.

    The instructions to create an object of this class from the Scheme
    script are:

    \verbinclude vsg-viewer-dueca.scm
*/
class VSGViewer_Dueca : public ScriptCreatable, public VSGViewer
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
  static const ParameterTable *getParameterTable();

private:
  /** Specification of a viewport */
  ViewSpec build_view_spec;

  /** Specification of a window */
  WinSpec build_win_spec;

  /** Specification of a visual object in the world */
  WorldDataSpec build_object_spec;

  /** Specify a window */
  bool addWindow(const std::string &window);

  /** Window position */
  bool setWindowPosition(const std::vector<int> &pos);

  /** X screen selection */
  bool setWindowXScreen(const std::string &scr);

  /** Specify a viewport */
  bool addViewport(const std::string &viewport);

  /** Viewport window */
  bool setViewportWindow(const std::string &vp_window);

  /** Viewport size */
  bool setViewportSize(const std::vector<int> &vpwin_size);

  /** frustum data */
  bool setFrustum(const std::vector<float> &frustum);

  /** frustum data */
  bool setMaskImage(const std::string &image);

  /** eye offset data */
  bool setEyeOffset(const std::vector<float> &frustum);

  /** Add an object in the world. */
  bool addScriptObject(ScriptCreatable &ava, bool in);

  /** Set the coordinates for an object in the world */
  bool setObjectCoordinates(const std::vector<double> &coord);

  /** Define the behaviour wrt the observer */
  bool setObjectBehavior(const std::string &beh);

  /** Add a possible class of object to the world. */
  bool addObjectClass(const std::vector<std::string> &names);

  /** Add a possible class of object to the world. */
  bool addObjectClassData(const std::vector<std::string> &names);

  /** Set the fog parameters */
  bool setFog(const std::vector<double> &fog);

public:
    /** Default script linkage. */
  SCM_FEATURES_DEF;
};

} // namespace vsgviewer

#endif
