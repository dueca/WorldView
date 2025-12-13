/* ------------------------------------------------------------------   */
/*      item            : VsgObject.hxx
        made by         : Rene van Paassen
        date            : 090617
        category        : header file
        description     :
        changes         : 090617 first version
        language        : C++
*/

#pragma once
#include <vsg/all.h>
#include <WorldObjectBase.hxx>
#include <WorldDataSpec.hxx>
#include <string>
#include <map>

namespace vsgviewer {

class VSGViewer;

/** Base class for OpenSceneGraph objects that are controlled from the
    simulation.

    Typically, a derived class has

    - a constructor with (const WorldDataSpec& x) as argument

    - a connect() function that creates a read token for the data from
      the world channel data

    - an iterate() function that reads the channel data and updates
      the object
*/
class VSGObject: public WorldObjectBase
{
  // name - node association
  static std::map<std::string,vsg::ref_ptr<vsg::Node>> name_node;

protected:
  WorldDataSpec spec;

public:
  /** Constructor */
  VSGObject(const WorldDataSpec& spec);

  /** Destructor */
  virtual ~VSGObject();

public:

  /** Initialise the vsg side of the object.

      @param root   Root node of the scene graph
      @param master Pointer to the viewer.
   */
  virtual void init(vsg::ref_ptr<vsg::Group> root,
                    VSGViewer* master) = 0;

  /** Undo the initialisation */
  virtual void unInit(vsg::ref_ptr<vsg::Group> root) = 0;

  /** Returns true if the object needs drawing post-access. */
  virtual bool requirePostDrawAccess() { return false; }

  /** Set the visibility. Overrides the ObjectBase.

      @param vis    Visibility true or false.
  */
  virtual void visible(bool vis) override;

  /** Add to active group; these object will receive updates on the
      Observer/view position. */
  virtual bool forceActive();

  /** modify, new parameters etc */
  virtual void adapt(const WorldDataSpec &data);

  /** return the original specification */
  inline const WorldDataSpec& getSpec() const { return spec; }

  /** Get a node */
  vsg::ref_ptr<vsg::Node> findNode(const std::string& name) const;

  /** Insert a node in the map */
  void insertNode(vsg::ref_ptr<vsg::Node> mynode) const;

  /** Insert a node in the map */
  void removeNode(vsg::ref_ptr<vsg::Node> mynode) const;
};

/** Grouping that offers a culling possibility */
class VSGCullGroup: public VSGObject
{
protected:
  /** a cullgroup */
  vsg::ref_ptr<vsg::CullGroup> cullgroup;

public:
  /** Constructor */
  VSGCullGroup(const WorldDataSpec& data);

  /** Destructor */
  virtual ~VSGCullGroup();
};

};
