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
#include <string>

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
protected:
  std::string    parent;
public:
  /** Constructor */
  VSGObject();

  /** Destructor */
  virtual ~VSGObject();

public:

  /** Initialise the vsg side of the object.

      @param root   Root node of the scene graph
      @param master Pointer to the viewer.
   */
  virtual void init(const vsg::ref_ptr<vsg::Group>& root,
                    VSGViewer* master) = 0;

  /** Undo the initialisation */
  virtual void unInit(const vsg::ref_ptr<vsg::Group>& root) = 0;

  /** Returns true if the object needs drawing post-access. */
  virtual bool requirePostDrawAccess() { return false; }

  /** Set the visibility. Overrides the ObjectBase.

      @param vis    Visibility true or false.
  */
  virtual void visible(bool vis) override;

  /** Add to active group; these object will receive updates on the
      Observer/view position. */
  virtual bool forceActive();
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

/** Helper, to find a named node in the tree */
vsg::ref_ptr<vsg::Group> findParent(vsg::ref_ptr<vsg::Group> root,
                                    const std::string& name);

};
