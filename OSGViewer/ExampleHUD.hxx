/* ------------------------------------------------------------------   */
/*      item            : ExampleHUD.hxx
        made by         : Rene van Paassen
        date            : 190430
        category        : header file
        description     :
        changes         : 190430 first version
        language        : C++
        copyright       : (c) 2019 TUDelft-AE-C&S
*/

#ifndef ExampleHUD_hxx
#define ExampleHUD_hxx

#include "OSGObject.hxx"
#include <boost/scoped_ptr.hpp>
#include <dueca/dueca.h>
#include <osg/PositionAttitudeTransform>
#include <osgText/Text>

/** Class for drawing a hud

    NOT FINISHED YET!
*/
class ExampleHUD : public WorldObjectBase
{
  /** Temporary node mask?? */
  unsigned int nodemask;

  /** Window to connect to */
  std::string wname;

  /** Viewport to look into */
  std::string vname;

protected:
  /** Channel read token for hud control input */
  boost::scoped_ptr<ChannelReadToken> r_hud;

  /** Ortho camera for the view */
  osg::ref_ptr<osg::Camera> camera;

  /** Transformation */
  osg::ref_ptr<osg::PositionAttitudeTransform> transform;

  /** Speed text */
  osg::ref_ptr<osgText::Text> text;

public:
  /** Constructor */
  ExampleHUD(const WorldDataSpec &spec);

  /** Destructor */
  ~ExampleHUD();

  /** Connect to a channel entry
      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  virtual void connect(const GlobalId &master_id, const NameSet &cname,
                       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect);

  /** Initialise the hud with the OSG scene */
  virtual void init(const osg::ref_ptr<osg::Group> &root, OSGViewer *master);

  /** De-initialise the hud with the OSG scene */
  virtual void unInit(const osg::ref_ptr<osg::Group> &root);

  /** Control visibility */
  virtual void visible(bool vis);

  /** Play, update, recalculate, etc. */
  void iterate(TimeTickType ts, const BaseObjectMotion &base, double late,
               bool freeze) override;
};

#endif
