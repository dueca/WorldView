/* ------------------------------------------------------------------   */
/*      item            : VSGSwitch.cxx
        made by         : Rene' van Paassen
        date            : 251212
        category        : body file
        description     :
        changes         : 251212 first version
        language        : C++
        copyright       : (c) 25 TUDelft-AE-C&S
*/

#include "VSGSwitch.hxx"
#include "VSGObjectFactory.hxx"
#include <algorithm>
#include <dueca/debug.h>

namespace vsgviewer {

VSGSwitch::VSGSwitch(const WorldDataSpec &data) :
  VSGObject(data),
  node()
{}

VSGSwitch::~VSGSwitch() {}

void VSGSwitch::adapt(const WorldDataSpec &data)
{
  this->spec = data;
  if (node && spec.coordinates.size()) {
    node->setSingleChildOn(size_t(spec.coordinates[0]));
  }
}

void VSGSwitch::init(const vsg::ref_ptr<vsg::Group> root, VSGViewer *master)
{
  if (node)
    return;

  node = vsg::Switch::create();
  node->setValue("name", name);
  for (auto const &child: spec.children) {
    auto ch = findNode(root, child.name) {

    }
  }
  auto par = findParent(root, spec.parent);
  if (!par) {
    W_MOD("Cannot find parent='" << spec.parent << "', for name=" << spec.name
                                 << ", attaching to root");
    par = root;
  }
  par->addChild(node);
  if (spec.coordinates.size())
    node->setSingleChildOn(size_t(spec.coordinates[0]));
}

void VSGSwitch::unInit(vsg::ref_ptr<vsg::Group> root)
{
  auto par = findParent(root, spec.parent);
  if (!par) {
    W_MOD("Cannot find parent='" << spec.parent << "', for name=" << spec.name
                                 << ", detaching from root");
    par = root;
  }

  auto it = std::find(par->children.begin(), par->children.end(), node);
  if (it != par->children.end()) {
    par->children.erase(it);
  }
}

static auto VSGSwitch_maker =
  new SubContractor<VSGObjectTypeKey, VSGSwitch>(
    "switch", "Child selection switch");

} // namespace vsgviewer