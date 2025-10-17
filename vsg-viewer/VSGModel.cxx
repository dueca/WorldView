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
#include "VSGViewer.hxx"
#include "VSGObjectFactory.hxx"
#include <dueca/debug.h>

namespace vsgviewer {

VSGStaticModel::VSGStaticModel(const WorldDataSpec &data) :
  model(),
  modelfile(data.filename.size() ? data.filename[0] : std::string())
{
  name = data.name;
  parent = data.parent;
  if (!data.filename.size()) {
    W_MOD("Static model needs a model filename");
  }
  D_MOD("Created visual model, name=" << name);
}

VSGStaticModel::~VSGStaticModel()
{
  D_MOD("Destroying visual model, name=" << name);
}

void VSGStaticModel::init(vsg::ref_ptr<vsg::Group> root, VSGViewer *master)
{
  model = vsg::read_cast<vsg::Node>(modelfile, master->options);
  if (!model) {
    W_MOD("Could not create static model, name=" << name
                                                 << ", file=" << modelfile);
    return;
  }
  model->setValue("name", name);
  auto par = findParent(root, parent);
  if (!par) {
    W_MOD("Cannot find parent='" << parent << "', for name=" << name
                                 << ", attaching to root");
    par = root;
  }
  par->addChild(model);
  D_MOD("VSG create visual model, name=" << name);
}

void VSGStaticModel::unInit(vsg::ref_ptr<vsg::Group> root)
{
  auto par = findParent(root, parent);
  if (!par) {
    W_MOD("Cannot find parent='" << parent << "', for name=" << name
                                 << ", detaching from root");
    par = root;
  }

  auto it = std::find(par->children.begin(), par->children.end(), model);
  if (it != par->children.end()) {
    par->children.erase(it);
  }
}

static auto VSGStaticModel_maker =
  new SubContractor<VSGObjectTypeKey, VSGStaticModel>(
    "static-model", "3D model from external modeling application");

VSGModel::VSGModel(const WorldDataSpec &data) :
  VSGMatrixTransform(data)
{
  name = data.name;
  parent = data.parent;
  if (!data.filename.size()) {
    W_MOD("Model needs a model filename");
  }
  D_MOD("Created visual model, name=" << name);
}

VSGModel::~VSGModel() { D_MOD("Destroying static model, name=" << name); }

void VSGModel::init(vsg::ref_ptr<vsg::Group> root, VSGViewer *master)
{
  model = vsg::read_cast<vsg::Node>(modelfile, master->options);
  VSGMatrixTransform::init(root, master);

  if (!model) {
    W_MOD("Could not create model, name=" << name << ", file=" << modelfile);
    return;
  }
  transform->addChild(model);
  D_MOD("VSG create visual model, name=" << name);
}

void VSGModel::unInit(vsg::ref_ptr<vsg::Group> root)
{
  auto it =
    std::find(transform->children.begin(), transform->children.end(), model);
  if (it != transform->children.end()) {
    transform->children.erase(it);
  }
  VSGMatrixTransform::unInit(root);
}

static auto VSGModel_maker = new SubContractor<VSGObjectTypeKey, VSGModel>(
  "model", "controlled 3D model from external modeling application");

}; // namespace vsgviewer
