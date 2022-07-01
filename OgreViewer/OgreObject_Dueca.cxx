/* ------------------------------------------------------------------   */
/*      item            : OgreObject_Dueca.cxx
        made by         : Rene' van Paassen
        date            : 100125
	category        : body file 
        description     : 
	changes         : 100125 first version
        language        : C++
*/

#define OgreObject_Dueca_cxx
#include "OgreObject_Dueca.hxx"
#include "AxisTransform.hxx"

#define W_MOD
#define E_MOD
#include <debug.h>

#define DO_INSTANTIATE
#include <VarProbe.hxx>
#include <MemberCall.hxx>
#include <MemberCall2Way.hxx>
#include <CoreCreator.hxx>

USING_DUECA_NS;

// Parameters to be inserted
const ParameterTable* OgreObject_Dueca::getParameterTable()
{
  static const ParameterTable parameter_table[] = {

    /* You can extend this table with labels and MemberCall or
       VarProbe pointers to perform calls or insert values into your
       class objects. Please also add a description (c-style string). */
    { "name", 
      new VarProbe<OgreObject_Dueca, string>(&OgreObject_Dueca::name),
      "set the name of the ogre object" },

    /* The table is closed off with NULL pointers for the variable
       name and MemberCall/VarProbe object. The description is used to
       give an overall description of the module. */
    { NULL, NULL, 
      "Simple link between dueca and Ogre"} };

  return parameter_table;
}


OgreObject_Dueca::OgreObject_Dueca() :
  ScriptCreatable(),
  OgreObject()
{
  //
}

bool OgreObject_Dueca::complete()
{
  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */
  if (!name.size()) {
    E_CNF("set the name of the attachment");
    return false;
  }

  return true;
}


OgreObject_Dueca::~OgreObject_Dueca()
{
  //
}

#if 0
// disable for now? Not complete any more?

SCM_FEATURES_NOIMPINH(OgreObject_Dueca, ScriptCreatable, 
		      "ogre-object-dueca");

#ifdef SCRIPT_PYTHON
static CoreCreator<OgreObject_Dueca> a(OgreObject_Dueca::getParameterTable(),
				       "OgreObject");
#else
static CoreCreator<OgreObject_Dueca> a(OgreObject_Dueca::getParameterTable(),
				       "ogre-object-dueca");
#endif

#endif
