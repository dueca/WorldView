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
#include <vector>

namespace vsgviewer {

VSGSwitch::VSGSwitch(const WorldDataSpec &data) :
  VSGObject(data),
  node()
{}

VSGSwitch::~VSGSwitch() {}

// static bool childmatc(vsg::Switch::Child& ch, vsg::ref_ptr<>Node)

void VSGSwitch::adapt(const WorldDataSpec &data)
{
  if (node && data.children.size() > spec.children.size()) {

    std::list<vsg::ref_ptr<vsg::Node>> children_in_spec;

    for (const auto &ch : spec.children) {

      auto child = findNode(ch.name);
      auto idx =
        std::find_if(node->children.begin(), node->children.end(),
                     [child](const auto &ch) { return ch.node == child; });

      if (child) {
        if (idx == node->children.end()) {
          I_MOD("VSGSWitch add child " << ch.name);
          node->addChild(node->children.size() == 0, child);
        }
        else {
          D_MOD("Already have child " << ch.name);
        }
        children_in_spec.push_back(child);
      }
      else {
        W_MOD("VSGSwitch cannot find child " << ch.name);
      }

      // clear any children not in spec list
      for(auto ic = node->children.begin(); ic!= node->children.end(); ic++){
        if (std::find(children_in_spec.begin(), children_in_spec.end(), ic->node) == children_in_spec.end()) {
          ic = node->children.erase(ic);
          I_MOD("VSGSwitch erasing child");
        }
      }
    }
  }
  if (node && data.coordinates.size()) {
    node->setSingleChildOn(size_t(data.coordinates[0]));
  }
  this->spec = data;
}

void VSGSwitch::init(const vsg::ref_ptr<vsg::Group> root, VSGViewer *master)
{
  if (node)
    return;

  I_MOD("VSGSwitch create " << spec.name);
  node = vsg::Switch::create();
  insertNode(node, root);

  bool visible = true;
  for (const auto &ch : spec.children) {
    auto child = findNode(ch.name);
    if (child) {
      node->addChild(visible, child);
      I_MOD("VSGSwitch add child " << ch.name);
    }
    else {
      W_MOD("VSGSwitch cannot find child " << ch.name);
    }
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