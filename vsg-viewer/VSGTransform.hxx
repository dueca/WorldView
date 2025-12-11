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

/** Transform base class */
class VSGBaseTransform : public VSGObject
{
protected:
  /** VSG transform */
  vsg::ref_ptr<vsg::MatrixTransform> transform;

public:
  /** Constructor */
  VSGBaseTransform(const WorldDataSpec &data);

  /** Destructor */
  ~VSGBaseTransform();

  /** Undo the initialisation */
  void unInit(vsg::ref_ptr<vsg::Group> root) override;
};

/** Fixed, static transform, can be child of any node. When child
of the "root" node, this designates a fixed location and orientation, when
child of the "eye" */
class VSGStaticMatrixTransform : public VSGBaseTransform
{
  /** Base transform */
  vsg::dmat4 base_transform;

public:
  /** Constructor */
  VSGStaticMatrixTransform(const WorldDataSpec &data);

  /** Destructor */
  ~VSGStaticMatrixTransform();

  /** Initialise the transform with the VSG scene */
  void init(vsg::ref_ptr<vsg::Group> root, VSGViewer *master) final;

  /** Adapt the model */
  void adapt(const WorldDataSpec &data) override;
};

/** Matrix location centered on observer x, y position (skydomes).

    These objects are child of the root, follow the observer,
    but keep their orientation fixed
*/
class VSGCenteredTransform : public VSGBaseTransform
{
  /** Base transform */
  vsg::dmat4 base_transform;

public:
  /** Constructor */
  VSGCenteredTransform(const WorldDataSpec &data);

  /** Destructor */
  ~VSGCenteredTransform();

  /** Initialise the transform with the VSG scene */
  void init(vsg::ref_ptr<vsg::Group> root, VSGViewer *master) final;

  /** Update on the observer position */
  void iterate(TimeTickType ts, const BaseObjectMotion &base, double late,
               bool freeze = false) override;

  /** Force in active list */
  bool forceActive() final;

  /** Adapt the model */
  void adapt(const WorldDataSpec &data) override;
};

/** Matrix location roughly centered on observer x, y position (floor tiles).

    These objects are by default child of the root location, keep their
    position fixed but jump by a tile size when the observer moves out
    of the tile scope.
*/
class VSGTiledTransform : public VSGBaseTransform
{
  /** Base transform */
  vsg::dmat4 base_transform;

  /** Tile size */
  vsg::dvec3 tile_size;

public:
  /** Constructor */
  VSGTiledTransform(const WorldDataSpec &data);

  /** Destructor */
  ~VSGTiledTransform();

  /** Initialise the transform with the VSG scene */
  void init(vsg::ref_ptr<vsg::Group> root, VSGViewer *master) final;

  /** Update on the observer position */
  void iterate(TimeTickType ts, const BaseObjectMotion &base, double late,
               bool freeze = false) override;

  /** Force in active list */
  bool forceActive() final;

  /** Adapt the model */
  void adapt(const WorldDataSpec &data) override;
};

/** Move and orient a transform according to (world) channel data. */
class VSGMatrixTransform : public VSGBaseTransform
{
protected:
  /** Scale */
  vsg::t_mat4<double> scale;

  /** Channel read token for motion input */
  boost::scoped_ptr<ChannelReadToken> r_motion;

public:
  /** Constructor */
  VSGMatrixTransform(const WorldDataSpec &data);

  /** Destructor */
  ~VSGMatrixTransform();

  /** Connect to a channel entry
      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  virtual void connect(const GlobalId &master_id, const NameSet &cname,
                       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect) override;

  /** Play, update, recalculate, etc. */
  void iterate(TimeTickType ts, const BaseObjectMotion &base, double late,
               bool freeze = false) override;

  /** Initialise the transform with the VSG scene */
  virtual void init(vsg::ref_ptr<vsg::Group> root, VSGViewer *master) override;

  /** Adapt the model */
  void adapt(const WorldDataSpec &data) override;
};

}; // namespace vsgviewer
