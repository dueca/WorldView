/* ------------------------------------------------------------------   */
/*      item            : OSGObjectCompatible.hxx
        made by         : Rene van Paassen
        date            : 180903
	category        : header file 
        description     : 
	changes         : 180903 first version
        language        : C++
        copyright       : (c) 2018 TUDelft-AE-C&S
*/

#ifndef OSGObjectCompatible_hxx
#define OSGObjectCompatible_hxx

#include "OSGObjectMoving.hxx"
#include <boost/scoped_ptr.hpp>
#include <dueca/dueca.h>

/** OSG objects controlled with the "older" ObjectMotion dco object. 

    This finalizes the object when the first data comes in, which informs
    about the object class and object name, rather than relying on the 
    entry label and dco class */
class OSGObjectCompatible: public OSGObjectMoving
{
  /** Mapping of "klass" to file name */
  static std::map<std::string,std::string> typemap;

  /** Keep a reference to the root node, for init */
  osg::ref_ptr<osg::Group> root;

  /** And the boss class */
  OSGViewer *master;
  
  /** Allow OSGViewer_DUECA to extend the typemap */
  friend class OSGViewer_Dueca;
  
public:

  /** Constructor */  
  OSGObjectCompatible(const WorldDataSpec& spec);

  /** Destructor */
  ~OSGObjectCompatible() final;

  /** Connect to a channel entry 
      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  void connect(const GlobalId& master_id, const NameSet& cname,
               entryid_type entry_id,
               Channel::EntryTimeAspect time_aspect) final;
  
  /** Overridden, to store a pointer to the root for later init. */
  void init(const osg::ref_ptr<osg::Group>& root, OSGViewer* master) final;

  /** Play, update, recalculate, etc. */
  void iterate(TimeTickType ts, const BaseObjectMotion& base,
	       double late) final;
};

#endif
