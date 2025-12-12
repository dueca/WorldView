/* ------------------------------------------------------------------   */
/*      item            : VSGSwitch.hxx
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
class VSGSwitch : public VSGObject
{
protected:
  /** VSG transform */
  vsg::ref_ptr<vsg::Switch> node;

public:
  /** Constructor */
  VSGSwitch(const WorldDataSpec &data);

  /** Destructor */
  ~VSGSwitch();

  /** Initialise the switch with the VSG scene */
  void init(vsg::ref_ptr<vsg::Group> root, VSGViewer *master) final;

  /** Adapt the model */
  void adapt(const WorldDataSpec &data) override;

  /** Undo the initialisation */
  void unInit(vsg::ref_ptr<vsg::Group> root) override;
};

} // vsgviewer