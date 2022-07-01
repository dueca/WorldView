/* ------------------------------------------------------------------   */
/*      item            : OSGCallback.cxx
	made by         : Olaf Stroosma
        date            : 200327
	category        : body file
        description     : base class for script-creatable objects that
	                  do things in a post-draw callback (e.g. a HUD)
	changes         : 200327 first version
        language        : C++
*/

#define OSGCallback_cxx
#include "OSGCallback.hxx"

OSGCallback::OSGCallback() :
  ScriptCreatable(),
  osg::Camera::Camera::DrawCallback()
{

}

OSGCallback::~OSGCallback()
{

}

/** Example callback */
void OSGCallback::operator()(osg::RenderInfo& renderInfo) const
{
  // initialize when OSG has set up everything and is already calling us
  if(!initialized) {

    // do the initialization here
    
    initialized = true;
  }

  // render politely
  osg::State* ri = renderInfo.getState();

  // do the rendering here

  // clean up
  renderInfo.setState(ri);
}


