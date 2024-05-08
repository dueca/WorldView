/* ------------------------------------------------------------------   */
/*      item            : PlibObject.hxx
        made by         : Rene van Paassen
        date            : 100126
        category        : header file
        description     :
        changes         : 100126 first version
        language        : C++
*/

#ifndef PlibObject_hxx
#define PlibObject_hxx

#include <WorldObjectBase.hxx>
#include <boost/scoped_ptr.hpp>
#include <dueca/dueca.h>
#include <plib/ssg.h>
#include <string>

/** Base simple objects drawn with plib/ssg. */
class PlibObject : public WorldObjectBase
{
protected:
  /** File name of the model */
  std::string modelfile;

  /** Transformation to the object. */
  ssgTransform *to_model;

  /** The object itself */
  ssgEntity *model;

  /** scale */
  double scale[3];

  /** Load a plib entity from a file with ac3d, wavefront, 3ds or
      vrml1 model */
  static ssgEntity *load(const std::string &file);

  /** Channel read token for motion input */
  boost::scoped_ptr<ChannelReadToken> r_motion;

public:
  /** Constructor */
  PlibObject(const WorldDataSpec &specification);

  /** Destructor */
  ~PlibObject();

  /** Initialise with the scene manager */
  virtual void init(ssgRoot *scene);

  /** Draw the object directly, usually OpenGL */
  virtual void draw(int viewno);

  /** Place and orient the object */
  virtual void update(double xyz[3], double quat[4]);

  /** Place and orient the object */
  virtual void update(const ObjectMotion &o, double tick, double late);

  /** Place, orient and scale the object */
  void update(double xyz[3], double quat[4], double scale[3]);

  /** Control visibility */
  virtual void visible(bool vis);

  /** Visualisation depends on observer position */
  virtual void observerUpdate(const double xyz[3], const double quat[4]);

  /** Connect to a channel entry
      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  virtual void connect(const GlobalId &master_id, const NameSet &cname,
                       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect);

  /** Play, update, recalculate, etc. */
  virtual void iterate(TimeTickType ts, const BaseObjectMotion &base,
                       double late, bool freeze = false) override;

public:
  /** Returns true if the object needs information on the observer
      position */
  virtual bool requireInfoObserver() { return false; }

  /** Returns true if the object needs drawing post-access */
  virtual bool requirePostDrawAccess() { return false; }
};

#endif
