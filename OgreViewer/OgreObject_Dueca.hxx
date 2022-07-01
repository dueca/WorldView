/* ------------------------------------------------------------------   */
/*      item            : OgreObject_Dueca.hxx
        made by         : Rene van Paassen
        date            : 100125
	category        : header file 
        description     : 
	changes         : 100125 first version
        language        : C++
*/

#ifndef OgreObject_Dueca_hxx
#define OgreObject_Dueca_hxx

// include the dueca headers
#include <ScriptCreatable.hxx>
#include <stringoptions.h>
#include <ParameterTable.hxx>
#include <dueca_ns.h>
#include "OgreObject.hxx"
#include "comm-objects.h"

USING_DUECA_NS;

/** A class definition for a DUECA helper class
    
    An OgreObject_Dueca links an entity in the DUECA world (an
    avatar, movable entity-like thing) to an entity *already created*
    in Ogre, e.g. by means of loading a .scene file. The names of the
    Ogre object and the Dueca side object need to be identical. These
    objects, once created, are understood by the OgreViewer class and
    can be fed with the appropriate data from DUECA.

    This class has been derived from the ScriptCreatable base class,
    and has a (scheme) script command to create it and optionally add
    parameters.

    The instructions to create an object of this class from the Scheme
    script are:

    \verbinclude dueca-ogre-attachment.scm
 */
class OgreObject_Dueca: public ScriptCreatable, public OgreObject
{
  
public:
  /** Constructor */
  OgreObject_Dueca();

  /** Destructor */
  ~OgreObject_Dueca();

  /** Continued construction. This is called after all script
      parameters have been read and filled in, according to the
      parameter table. */
  bool complete();

  /** Obtain a pointer to the parameter table. */
  static const ParameterTable* getParameterTable();

public:
  /** Default script linkage. */
  SCM_FEATURES_DEF;
};

#endif
