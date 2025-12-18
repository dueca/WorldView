/* ------------------------------------------------------------------   */
/*      item            : VSGGroup.hxx
        made by         : Rene van Paassen
        date            : 251212
        category        : header file
        description     :
        changes         : 251212 first version
        language        : C++
        copyright       : (c) 25 TUDelft-AE-C&S
*/

#pragma once

#include "VSGObject.hxx"

namespace vsgviewer {

/** Transform base class */
class VSGGroup : public VSGObject
{
protected:
  /** VSG transform */
  vsg::ref_ptr<vsg::Group> node;

public:
  /** Constructor */
  VSGGroup(const WorldDataSpec &data);

  /** Destructor */
  ~VSGGroup();

  /** Initialise the Group with the VSG scene */
  void init(vsg::ref_ptr<vsg::Group> root, VSGViewer *master) final;

  /** Adapt the model */
  void adapt(const WorldDataSpec &data) override;

  /** Undo the initialisation */
  void unInit(vsg::ref_ptr<vsg::Group> root) override;
};

} // vsgviewer