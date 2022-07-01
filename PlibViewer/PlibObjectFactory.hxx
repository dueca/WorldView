/* ------------------------------------------------------------------   */
/*      item            : PlibObjectFactory.hxx
        made by         : Rene van Paassen
        date            : 100208
	category        : header file 
        description     : 
	changes         : 100208 first version
        language        : C++
*/

#ifndef PlibObjectFactory_hxx
#define PlibObjectFactory_hxx

#include "PlibObject.hxx"
#include <ConglomerateFactory.hxx>
#include "comm-objects.h"
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/thread/detail/singleton.hpp>
#include <vector>

/** \file PlibObjectFactory.hxx

    This file defines the specialization of an ObjectFactory singleton
    for graphical objects to be shown with the PlibViewer */

/** Index for base types in the factory */
struct PlibObjectTypeKey
{
  /** map key, text defines typs of object */
  typedef std::string Key;

  /** Returned object is simply a pointer */
  typedef PlibObject* ProductBase;

  /** This is a generic strings + doubles object, flexible enough to
      define most stuff */
  typedef WorldDataSpec SpecBase;
};

/** Define the PlibObjectFactory as an instantiation of the default
    ConglomerateFactory, combined with the singleton template from
    boost.
    
    The PlibObjectFactory can create objects for visualisation with
    plib. 
*/
typedef boost::detail::thread::singleton
<ConglomerateFactory
 <PlibObjectTypeKey, 
  boost::shared_ptr<SubcontractorBase<PlibObjectTypeKey> > > > 
PlibObjectFactory;

/** Template for a simple (single case) subcontractor.

    If you want to make your object available for the PlibViewer
    object factory, create a static SubContractor, as follows:

    \code
    class MyGraphicObject: public PlibObject
    {
      // need a constructor that uses a WorldDataSpec object
      MyGraphicObject(const WorldDataSpec& spec);

      // for the rest, see what you need to redefine from PlibObject!
    };

    // and in your cxx file:
    static SubContractor<PlibObjectTypeKey, MyGraphicObject>
    *MyGraphicObject_maker = new 
    SubContractor<PlibObjectTypeKey, MyGraphicObject>("my-graphic-object");
    \endcode

    With this, the PlibViewer can figure out how to create objects of
    type "my-graphic-object". 
*/
template<typename Xbase, typename Derived>
class SubContractor: public SubcontractorBase<Xbase>
{
  /** Basically the class name of the type of objects created here. */
  typename Xbase::Key key;
public:
  /** Constructor */
  SubContractor(const char* key) :
    key(key)
  {
    PlibObjectFactory::instance().addSubcontractor
      (this->key, boost::shared_ptr<SubcontractorBase<Xbase > >(this));
  }
  
  /** create a new object */
  typename Xbase::ProductBase create(const typename Xbase::Key& key, 
				     const typename Xbase::SpecBase& spec)
  {
    return new Derived(spec);
  }
};

/** The base subcontractor pointer in Plib */
typedef boost::shared_ptr<SubcontractorBase<PlibObjectTypeKey> > SubconPtr;

#endif
