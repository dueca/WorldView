/* ------------------------------------------------------------------   */
/*      item            : OSGCallback.hxx
        made by         : Olaf Stroosma
        date            : 200327
	category        : header file 
        description     : base class for script-creatable objects that
	                  do things in a post-draw callback (e.g. a HUD)
	changes         : 200327 first version
        language        : C++
*/

#ifndef OSGCallback_hxx
#define OSGCallback_hxx

#include <osg/RenderInfo>
#include <osg/Camera>

#include <ScriptCreatable.hxx>
USING_DUECA_NS;

/** A script-creatable post-draw callback object for e.g. HUDs. */
class OSGCallback: public ScriptCreatable, public osg::Camera::Camera::DrawCallback
{
protected:
  
public:
  /** Constructor */
  OSGCallback();
  
  /** Destructor */
  ~OSGCallback();
  
public: // osg::Camera::DrawCallback
  /** Actual callback function, called by renderer. */
  virtual void operator()(osg::RenderInfo& renderInfo) const;
 
public: // ScriptCreatable
  /** This is a scheme-level callable class */
  SCM_FEATURES_DEF;
  
protected:
  /** have we been initialized? */
  mutable bool initialized;
};

#endif
