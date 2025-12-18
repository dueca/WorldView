/* ------------------------------------------------------------------   */
/*      item            : VSGModel.cxx
        made by         : Rene' van Paassen
        date            : 230126
        category        : body file
        description     :
        changes         : 230126 first version
        language        : C++
        copyright       : (c) 23 TUDelft-AE-C&S
*/

#include "VSGModel.hxx"
#include "VSGObjectFactory.hxx"
#include "VSGViewer.hxx"
#include <dueca/debug.h>

namespace vsgviewer {

VSGStaticModel::VSGStaticModel(const WorldDataSpec &data) :
  VSGObject(data),
  model()
{
  if (!data.filename.size()) {
    W_MOD("Static model needs a model filename");
  }
}

VSGStaticModel::~VSGStaticModel()
{
  D_MOD("Destroying visual model, name=" << spec.name);
}

void VSGStaticModel::init(vsg::ref_ptr<vsg::Group> root, VSGViewer *master)
{
  // once and only once
  if (model)
    return;

  if (!spec.filename.size()) {
    W_MOD("No filename supplied for visual model");
    return;
  }

  model = master->loadModel(spec.filename[0]);
  if (!model) {
    W_MOD("Could not create static model, name=" << name << ", file="
                                                 << spec.filename[0]);
    return;
  }
  insertNode(model, root);
  D_MOD("VSG create visual model, name=" << spec.name);
}

void VSGStaticModel::unInit(vsg::ref_ptr<vsg::Group> root)
{
  if (model) {
    removeNode(model, root);
    model.reset();
  }
}

static auto VSGStaticModel_maker =
  new SubContractor<VSGObjectTypeKey, VSGStaticModel>(
    "static-model", "3D model from external modeling application");

VSGModel::VSGModel(const WorldDataSpec &data) :
  VSGMatrixTransform(data)
{
  if (!data.filename.size()) {
    W_MOD("Model needs a model filename");
  }
}

VSGModel::~VSGModel() { D_MOD("Destroying static model, name=" << name); }

void VSGModel::init(vsg::ref_ptr<vsg::Group> root, VSGViewer *master)
{
  // only once
  if (model || !spec.filename.size())
    return;

  model = master->loadModel(spec.filename[0]);
  VSGMatrixTransform::init(root, master);

  if (!model) {
    W_MOD("Could not create model, name=" << spec.name
                                          << ", file=" << spec.filename[0]);
    return;
  }
  transform->addChild(model);
  D_MOD("VSG create visual model, name=" << name);
}

void VSGModel::unInit(vsg::ref_ptr<vsg::Group> root)
{
  VSGMatrixTransform::unInit(root);
  model.reset();
}

static auto VSGModel_maker = new SubContractor<VSGObjectTypeKey, VSGModel>(
  "model", "controlled 3D model from external modeling application");

}; // namespace vsgviewer
