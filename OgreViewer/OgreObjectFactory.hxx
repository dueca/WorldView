/* ------------------------------------------------------------------   */
/*      item            : OgreObjectFactory.hxx
        made by         : Rene van Paassen
        date            : 100208
	category        : header file 
        description     : 
	changes         : 100208 first version
        language        : C++
*/

#ifndef OgreObjectFactory_hxx
#define OgreObjectFactory_hxx

#include "OgreObject.hxx"
#include <ConglomerateFactory.hxx>
#include <comm-objects.h>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/thread/detail/singleton.hpp>
#include <vector>

/** \file OgreObjectFactory.hxx

    This file defines the specialization of an ObjectFactory singleton
    for graphical objects to be shown with the OgreViewer. 

    This ObjectFactory is a conglomerate factory. "Subcontractors" in
    the factory are chosen on the basis of the key, in this case the 
    class of the DCO object used to transmit the data. Additionally, if 
    the label of the entry has a sub class separated by a colon (:) from
    the entry name, that will be combined and tried first.    
*/

/** Index for base types in the factory */
struct OgreObjectTypeKey
{
  /** map key, text defines typs of object */
  typedef std::string Key;
  
  /** Returned object is simply a pointer */
  typedef OgreObject* ProductBase;
  
  /** This is a generic strings + doubles object, flexible enough to
      define most stuff */
  typedef WorldDataSpec SpecBase;
};

/** Define the OgreObjectFactory as an instantiation of the default
    ConglomerateFactory, combined with the singleton template from
    boost.
    
    The OgreObjectFactory can create objects for visualisation with
    plib. 
*/
typedef boost::detail::thread::singleton
<ConglomerateFactory
 <OgreObjectTypeKey, 
  boost::shared_ptr<SubcontractorBase<OgreObjectTypeKey> > > > 
OgreObjectFactory;

/** Template for a simple (single case) subcontractor.

    If you want to make your object available for the OgreViewer
    object factory, create a static SubContractor, as follows:

    \code
    class MyGraphicObject: public OgreObject
    {
      // need a constructor that uses a WorldDataSpec object
      MyGraphicObject(const WorldDataSpec& spec);

      // for the rest, see what you need to redefine from OgreObject!
    };

    // and in your cxx file:
    static SubContractor<OgreObjectTypeKey, MyGraphicObject>
    *MyGraphicObject_maker = new 
    SubContractor<OgreObjectTypeKey, MyGraphicObject>("my-graphic-object");
    \endcode

    With this, the OgreViewer can figure out how to create objects of
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
    OgreObjectFactory::instance().addSubcontractor
      (this->key, boost::shared_ptr<SubcontractorBase<Xbase > >(this));
  }
  
  /** create a new object */
  typename Xbase::ProductBase create(const typename Xbase::Key& key, 
				     const typename Xbase::SpecBase& spec)
  {
    return new Derived(spec);
  }
};

/** The base subcontractor pointer in Ogre */
typedef boost::shared_ptr<SubcontractorBase<OgreObjectTypeKey> > SubconPtr;

#endif
