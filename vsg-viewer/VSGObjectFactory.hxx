/* ------------------------------------------------------------------   */
/*      item            : OSGObjectFactory.hxx
        made by         : Rene van Paassen
        date            : 100208
        category        : header file
        description     :
        changes         : 100208 first version
        language        : C++
*/

#ifndef OSGObjectFactory_hxx
#define OSGObjectFactory_hxx

#include "OSGObject.hxx"
#include <ConglomerateFactory.hxx>
#include "comm-objects.h"
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/thread/detail/singleton.hpp>
#include <vector>

/** \file OSGObjectFactory.hxx

    This file defines the specialization of an ObjectFactory singleton
    for graphical objects to be shown with the OSGViewer */

/** Index for base types in the factory */
struct OSGObjectTypeKey
{
  /** map key, text defines type of object */
  typedef std::string Key;

  /** Returned object is simply a pointer */
  typedef OSGObject* ProductBase;

  /** This is a generic strings + doubles object, flexible enough to
      define most stuff */
  typedef WorldDataSpec SpecBase;
};

/** Define the OSGObjectFactory as an instantiation of the default
    ConglomerateFactory, combined with the singleton template from
    boost.

    The OSGObjectFactory can create objects for visualisation with
    OpenSceneGraph.
*/
typedef boost::detail::thread::singleton
<ConglomerateFactory
 <OSGObjectTypeKey,
  boost::shared_ptr<SubcontractorBase<OSGObjectTypeKey> > > >
OSGObjectFactory;

/** Template for a simple (single case) subcontractor.

    If you want to make your object available for the OSGViewer
    object factory, create a static SubContractor, as follows:

    \code
    class MyGraphicObject: public OSGObject
    {
      // need a constructor that uses a WorldDataSpec object
      MyGraphicObject(const WorldDataSpec& spec);

      // for the rest, see what you need to redefine from OSGObject!
    };

    // and in your cxx file:
    static SubContractor<OSGObjectTypeKey, MyGraphicObject>
    *MyGraphicObject_maker = new
    SubContractor<OSGObjectTypeKey, MyGraphicObject>("my-graphic-object");
    \endcode

    With this, the OSGViewer can figure out how to create objects of
    type "my-graphic-object".
*/
template<typename Xbase, typename Derived>
class SubContractor: public SubcontractorBase<Xbase>
{
  /** Basically the class name of the type of objects created here. */
  typename Xbase::Key key;
public:
  /** Constructor */
  SubContractor(const char* key, const char* description = NULL) :
    key(key)
  {
    OSGObjectFactory::instance().addSubcontractor
      (this->key, boost::shared_ptr<SubcontractorBase<Xbase > >(this));
  }

  /** create a new object */
  typename Xbase::ProductBase create(const typename Xbase::Key& key,
                                     const typename Xbase::SpecBase& spec)
  {
    return new Derived(spec);
  }
};

/** The base subcontractor pointer in OSG */
typedef boost::shared_ptr<SubcontractorBase<OSGObjectTypeKey> > SubconPtr;

#endif
