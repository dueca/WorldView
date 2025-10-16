/* ------------------------------------------------------------------   */
/*      item            : VSGModel.hxx
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
#include "VSGObjectFactory.hxx"
#include "VSGTransform.hxx"

namespace vsgviewer {

class VSGViewer;

class VSGStaticModel : virtual public VSGObject
{
protected:
  /** Single 3D model */
  vsg::ref_ptr<vsg::Node> model;

  /** Filename */
  std::string modelfile;

public:
  /** Constructor

      @param data:
      * data.name        - model name
      * data.parent      - if filled, used to look up parent node
      * data.filename[0] - name for the visual model
  */
  VSGStaticModel(const WorldDataSpec &data);

    /** Destructor */
  ~VSGStaticModel();

  /** Initialise the model with the VSG scene */
  void init(vsg::ref_ptr<vsg::Group> root, VSGViewer *master) override;

  /** Undo the initialisation */
  void unInit(vsg::ref_ptr<vsg::Group> root) override;
};

class VSGModel : public VSGMatrixTransform
{
  /** Single 3D model */
  vsg::ref_ptr<vsg::Node> model;

  /** Filename */
  std::string modelfile;

public:
  /** Constructor */
  VSGModel(const WorldDataSpec &data);

  /** Destructor */
  ~VSGModel();

  /** Initialise the model with the VSG scene */
  void init(vsg::ref_ptr<vsg::Group> root, VSGViewer *master) final;

  /** Undo the initialisation */
  void unInit(vsg::ref_ptr<vsg::Group> root) override;
};

}; // namespace vsgviewer
