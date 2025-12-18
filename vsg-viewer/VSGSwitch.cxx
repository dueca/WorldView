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

#include "VSGObjectFactory.hxx"
#include "VSGSwitch.hxx"
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
  if (node && data.children.size() > spec.children.size()) {
    auto ch = data.children.begin();
    for (const auto &dum: spec.children)
      ch++;
    for (; ch != data.children.end(); ch++) {
      auto child = findNode(ch->name);
    if (child)
      node->addChild(1UL, child);
    }
  }
  if (node && spec.coordinates.size()) {
    node->setSingleChildOn(size_t(spec.coordinates[0]));
  }
  this->spec = data;
}

void VSGSwitch::init(const vsg::ref_ptr<vsg::Group> root, VSGViewer *master)
{
  if (node)
    return;

  node = vsg::Switch::create();
  insertNode(node, root);

  bool visible = true;
  for (const auto &ch : spec.children) {
    auto child = findNode(ch.name);
    if (child)
      node->addChild(visible, child);
    visible = false;
  }
}

void VSGSwitch::unInit(vsg::ref_ptr<vsg::Group> root)
{
  if (node) {
    removeNode(node, root);
    node.reset();
  }
}

static auto VSGSwitch_maker = new SubContractor<VSGObjectTypeKey, VSGSwitch>(
  "switch", "Child selection switch");

} // namespace vsgviewer