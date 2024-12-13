/* ------------------------------------------------------------------   */
/*      item            : VSGObjectFactory.hxx
        made by         : Rene van Paassen
        date            : 100208
        category        : header file
        description     :
        changes         : 100208 first version
        language        : C++
*/

#ifndef VSGObjectFactory_hxx
#define VSGObjectFactory_hxx

#include "VSGObject.hxx"
#include <ConglomerateFactory.hxx>
#include "comm-objects.h"
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/thread/detail/singleton.hpp>
#include <vector>

namespace vsgviewer {

  /** \file VSGObjectFactory.hxx

      This file defines the specialization of an ObjectFactory singleton
      for graphical objects to be shown with the VSGViewer */

  /** Index for base types in the factory */
  struct VSGObjectTypeKey
  {
    /** map key, text defines type of object */
    typedef std::string Key;

    /** Returned object is simply a pointer */
    typedef VSGObject* ProductBase;

    /** This is a generic strings + doubles object, flexible enough to
        define most stuff */
    typedef WorldDataSpec SpecBase;
  };

  /** Define the VSGObjectFactory as an instantiation of the default
      ConglomerateFactory, combined with the singleton template from
      boost.

      The VSGObjectFactory can create objects for visualisation with
      OpenSceneGraph.
  */
  typedef boost::detail::thread::singleton
  <ConglomerateFactory
   <VSGObjectTypeKey,
    boost::shared_ptr<SubcontractorBase<VSGObjectTypeKey> > > >
  VSGObjectFactory;

  /** Template for a simple (single case) subcontractor.

      If you want to make your object available for the VSGViewer
      object factory, create a static SubContractor, as follows:

      \code
      class MyGraphicObject: public VSGObject
      {
      // need a constructor that uses a WorldDataSpec object
      MyGraphicObject(const WorldDataSpec& spec);

      // for the rest, see what you need to redefine from VSGObject!
      };

      // and in your cxx file:
      static SubContractor<VSGObjectTypeKey, MyGraphicObject>
      *MyGraphicObject_maker = new
      SubContractor<VSGObjectTypeKey, MyGraphicObject>("my-graphic-object");
      \endcode

      With this, the VSGViewer can figure out how to create objects of
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
      VSGObjectFactory::instance().addSubcontractor
        (this->key, boost::shared_ptr<SubcontractorBase<Xbase > >(this));
    }

    /** create a new object */
    typename Xbase::ProductBase create(const typename Xbase::Key& key,
                                       const typename Xbase::SpecBase& spec)
    {
      return new Derived(spec);
    }
  };

  /** The base subcontractor pointer in VSG */
  typedef boost::shared_ptr<SubcontractorBase<VSGObjectTypeKey> > SubconPtr;

};

#endif
