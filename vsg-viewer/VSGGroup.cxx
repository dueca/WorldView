/* ------------------------------------------------------------------   */
/*      item            : VSGGroup.cxx
        made by         : Rene' van Paassen
        date            : 251212
        category        : body file
        description     :
        changes         : 251212 first version
        language        : C++
        copyright       : (c) 25 TUDelft-AE-C&S
*/

#include "VSGGroup.hxx"
#include "VSGObjectFactory.hxx"
#include <algorithm>
#include <dueca/debug.h>

namespace vsgviewer {

VSGGroup::VSGGroup(const WorldDataSpec &data) :
  VSGObject(data),
  node()
{}

VSGGroup::~VSGGroup() {}

void VSGGroup::adapt(const WorldDataSpec &data)
{
  this->spec = data;
}

void VSGGroup::init(const vsg::ref_ptr<vsg::Group> root, VSGViewer *master)
{
  if (node)
    return;

  node = vsg::Group::create();
  node->setValue("name", name);
  auto par = findParent(root, spec.parent);
  if (!par) {
    W_MOD("Cannot find parent='" << spec.parent << "', for name=" << spec.name
                                 << ", attaching to root");
    par = root;
  }
  par->addChild(node);
}

void VSGGroup::unInit(vsg::ref_ptr<vsg::Group> root)
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

static auto VSGGroup_maker =
  new SubContractor<VSGObjectTypeKey, VSGGroup>(
    "group", "Node group");

} // namespace vsgviewer