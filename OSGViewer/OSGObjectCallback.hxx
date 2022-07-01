/* ------------------------------------------------------------------   */
/*      item            : OSGObjectCallback.hxx
        made by         : Olaf Stroosma / Rene van Paassen
        date            : 200403
	category        : header file 
        description     : base class for script-creatable objects that
	                  do things in a post-draw callback (e.g. a HUD)
	changes         : 200327 first version
        language        : C++
*/

#ifndef OSGObjectCallback_hxx
#define OSGObjectCallback_hxx

#include <osg/RenderInfo>
#include <osg/Camera>
#include "OSGObject.hxx"

USING_DUECA_NS;

/** A script-creatable post-draw callback object for e.g. HUDs. 

    This class by itself does nothing, subclass it and implement your
    post-draw actions in the operator() callback. 

    Implement the following functions:

    - @code
      void operator()(osg::RenderInfo& renderInfo) const;
      @endcode

      This is the OSD post draw callback function. 

    - @code
      void connect(const GlobalId& master_id, const NameSet& cname,
                   entryid_type entry_id,
                   Channel::EntryTimeAspect time_aspect);
      @endcode

      Use this function to connect to the data entry in the 
      ObjectMotion://world channel

    - @code
      void init(const osg::ref_ptr<osg::Group>& root, 
                OSGViewer* master);
      @endcode

      This is called to offer initialisation with the OSG viewer. Use
      this to install the callback

    - @code
      void iterate(TimeTickType ts, const BaseObjectMotion& base,
                   double late);
      @endcode

      Called in each draw cycle, use this to update to-be-drawn information

    Then in your body file add an entry to the OSG object factory:

    @code
    #include "OSGObjectFactory.hxx"

    static auto MyDerivedObject_maker = new
      SubContractor<OSGObjectTypeKey,MyDerivedObject>("mypostdrawcallback");
    @endcode

    After this, declare under what conditions a "mypostdrawcallback"
    entry should be created by the factory, with the
    add_object_class_data argument to the OSGViewer creation in your
    startup script.
*/
class OSGObjectCallback:public OSGObject,
                         public osg::Camera::Camera::DrawCallback
{
public:
  /** Constructor */  
  OSGObjectCallback();
  
  /** Destructor */
  ~OSGObjectCallback();
};

#endif
