/* ------------------------------------------------------------------   */
/*      item            : VSGTransform.hxx
        made by         : Rene van Paassen
        date            : 230126
        category        : header file
        description     :
        changes         : 230126 first version
        language        : C++
        copyright       : (c) 23 TUDelft-AE-C&S
*/

#pragma once
#include "VSGObject.hxx"

namespace vsgviewer {

/** Absolute location change base class. */
class VSGStaticAbsoluteTransform: public VSGObject
{
protected:
  /** VSG transform */
  vsg::ref_ptr<vsg::AbsoluteTransform> transform;

public:
  /** Constructor */
  VSGStaticAbsoluteTransform(const WorldDataSpec& data);

  /** Destructor */
  ~VSGStaticAbsoluteTransform();

  /** Initialise the transform with the VSG scene */
  void init(const vsg::ref_ptr<vsg::Group>& root,
	    VSGViewer* master) final;
};

/** Matrix location change base class. */
class VSGStaticMatrixTransform: public VSGObject
{
  /** VSG transform */
  vsg::ref_ptr<vsg::MatrixTransform> transform;

public:
  /** Constructor */
  VSGStaticMatrixTransform(const WorldDataSpec& data);

  /** Destructor */
  ~VSGStaticMatrixTransform();

  /** Initialise the transform with the VSG scene */
  void init(const vsg::ref_ptr<vsg::Group>& root,
	    VSGViewer* master) final;
};


/** Absolute location change base class. */
class VSGAbsoluteTransform: virtual public VSGObject
{
protected:
  /** VSG transform */
  vsg::ref_ptr<vsg::AbsoluteTransform> transform;

  /** Scale */
  vsg::t_mat4<double>                  scale;

  /** Channel read token for motion input */
  boost::scoped_ptr<ChannelReadToken>     r_motion;

public:
  /** Constructor */
  VSGAbsoluteTransform(const WorldDataSpec& data);

  /** Destructor */
  virtual ~VSGAbsoluteTransform();

  /** Connect to a channel entry
      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  virtual void connect(const GlobalId& master_id, const NameSet& cname,
                       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect) override;

  /** Play, update, recalculate, etc. */
  virtual void iterate(TimeTickType ts, const BaseObjectMotion& base,
		       double late) override;

  /** Initialise the transform with the VSG scene */
  virtual void init(const vsg::ref_ptr<vsg::Group>& root,
                    VSGViewer* master) override;
};

/** Matrix location change base class. */
class VSGMatrixTransform: virtual public VSGObject
{
protected:
  /** VSG transform */
  vsg::ref_ptr<vsg::MatrixTransform> transform;

  /** Scale */
  vsg::t_mat4<double>                scale;

  /** Channel read token for motion input */
  boost::scoped_ptr<ChannelReadToken>     r_motion;

public:
  /** Constructor */
  VSGMatrixTransform(const WorldDataSpec& data);

  /** Destructor */
  ~VSGMatrixTransform();

  /** Connect to a channel entry
      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  virtual void connect(const GlobalId& master_id, const NameSet& cname,
                       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect);

  /** Play, update, recalculate, etc. */
  void iterate(TimeTickType ts, const BaseObjectMotion& base, double late);

  /** Initialise the transform with the VSG scene */
  virtual void init(const vsg::ref_ptr<vsg::Group>& root,
                    VSGViewer* master) override;
};

}; // namespace
