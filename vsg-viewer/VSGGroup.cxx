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
  insertNode(node, root);

  for (auto const &ch: spec.children) {
    auto child = findNode(ch.name);
    if (child) {
      node->addChild(child);
    }
  }
}

void VSGGroup::unInit(vsg::ref_ptr<vsg::Group> root)
{
  if (node) {
    removeNode(node, root);
    node.reset();
  }
}

static auto VSGGroup_maker =
  new SubContractor<VSGObjectTypeKey, VSGGroup>(
    "group", "Node group");

} // namespace vsgviewer