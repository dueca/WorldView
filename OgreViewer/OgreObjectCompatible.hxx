/* ------------------------------------------------------------------   */
/*      item            : OgreObjectCompatible.hxx
        made by         : Rene van Paassen
        date            : 180903
	category        : header file
        description     :
	changes         : 180903 first version
        language        : C++
        copyright       : (c) 2018 TUDelft-AE-C&S
*/

#ifndef OgreObjectCompatible_hxx
#define OgreObjectCompatible_hxx

#include "OgreObject.hxx"
#include <boost/scoped_ptr.hpp>
#include <dueca/dueca.h>

/** Ogre objects controlled with the "older" ObjectMotion dco object.

    This finalizes the object when the first data comes in, which informs
    about the object class and object name, rather than relying on the
    entry label and dco class */
class OgreObjectCompatible: public OgreObject
{
  /** Channel read token for motion input */
  boost::scoped_ptr<ChannelReadToken>     r_motion;

  struct MeshAndGroup {

    /** Ogre mesh name */
    std::string mesh;

    /** Ogre group name */
    std::string group;

    /** Constructor */
    MeshAndGroup(const std::string& mesh="",
                 const std::string& group="General");
  };

  /** Mapping of "klass" to mesh and group name */
  static std::map<std::string,MeshAndGroup> typemap;

  /** Store the manager, needed for delayed initialisation */
  Ogre::SceneManager*      manager;

  /** Number of attempts at initialization */
  bool                     failreport;

  /** Allow OgreViewer_DUECA to extend the typemap */
  friend class OgreViewer_Dueca;

public:

  /** Constructor */
  OgreObjectCompatible(const WorldDataSpec& spec);

  /** Destructor */
  ~OgreObjectCompatible();

  /** Initialisation must be delayed until the object class and mesh
      are known */
  void init(Ogre::SceneManager* manager);

  /** Connect to a channel entry

      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  virtual void connect(const GlobalId& master_id, const NameSet& cname,
                       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect);

  /** Play, update, recalculate, etc.

      @param ts        Time specification for update
      @param base      Movement of the base object, extrapolated if applicable
      @param late      Time elapsed since ts start tick
   */
  void iterate(TimeTickType ts, const BaseObjectMotion& base, double late, bool freeze) override;
};

#endif
