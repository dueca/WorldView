/* ------------------------------------------------------------------   */
/*      item            : OSGObjectCallback.cxx
	made by         : Olaf Stroosma / Rene van Paassen
        date            : 200403
	category        : body file
        description     : base class for script-creatable objects that
	                  do things in a post-draw callback (e.g. a HUD)
	changes         : 200327 first version
        language        : C++
*/

#define OSGObjectCallback_cxx
#include "OSGObjectCallback.hxx"

OSGObjectCallback::OSGObjectCallback() :
  OSGObject(),
  osg::Camera::Camera::DrawCallback()
{
  
}

OSGObjectCallback::~OSGObjectCallback()
{

}

#if 0
/** Example callback */
void OSGObjectCallback::operator()(osg::RenderInfo& renderInfo) const
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

#if DUECA_VERSION_NUM >= DUECA_VERSION(3,2,0)
#define OPT(A) , A
#else
#define OPT(A)
#endif


/** Example sign-in with the factory */
#include "OSGObjectFactory.hxx"
static auto OSGObjectCallback_maker = new
  SubContractor<OSGObjectTypeKey,OSGObjectCallback>
  ("mypostdrawcallback" OPT("Post draw callback routine"));

#endif

