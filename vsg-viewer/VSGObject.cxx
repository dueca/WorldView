/* ------------------------------------------------------------------   */
/*      item            : VsgObject.cxx
        made by         : Rene' van Paassen
        date            : 090617
        category        : body file
        description     :
        changes         : 090617 first version
        language        : C++
*/

#include "VSGObject.hxx"
#include <dueca/debug.h>
#include <vsgXchange/all.h>

namespace vsgviewer {

std::map<std::string,vsg::ref_ptr<vsg::Node>> VSGObject::name_node;

VSGObject::VSGObject(const WorldDataSpec &spec) :
  WorldObjectBase(),
  spec(spec)
{
  name = spec.name;
}

VSGObject::~VSGObject()
{
  //
}

bool VSGObject::forceActive() { return false; }

VSGCullGroup::VSGCullGroup(const WorldDataSpec &data) :
  VSGObject(data)
{
  D_MOD("Created cull group, name=" << name);
}

VSGCullGroup::~VSGCullGroup()
{
  D_MOD("Destroying cull group, name=" << spec.name);
}

static vsg::ref_ptr<vsg::Group> _findParent(vsg::ref_ptr<vsg::Group> node,
                                            const std::string &name)
{
  std::string iname;
  vsg::ref_ptr<vsg::Group> res;

  if (node->getValue("name", iname) && iname == name) {
    return node;
  }
  for (auto const &i : node->children) {
    vsg::ref_ptr<vsg::Group> g = i.cast<vsg::Group>();
    if (g) {
      res = _findParent(g, name);
      if (res)
        return res;
    }
  }
  return res;
}

vsg::ref_ptr<vsg::Node> VSGObject::findNode(const std::string &name) const
{
  auto elt = name_node.find(name);
  if (elt == name_node.end()) {
    W_MOD("Cannot find vsg object '" << name << "'");
    return vsg::ref_ptr<vsg::Node>();
  }
  return elt->second;
}

void VSGObject::insertNode(vsg::ref_ptr<vsg::Node> node, vsg::ref_ptr<vsg::Group> root) const
{
  if (name_node.count(getName())) {
    W_MOD("VSG object with name '" << getName() << "' already exits");
  }
  else {
    name_node.emplace(getName(), node);
  }
  if (spec.rootchild) {
    root->addChild(node);
  }
}

void VSGObject::removeNode(vsg::ref_ptr<vsg::Node> node, vsg::ref_ptr<vsg::Group> root) const
{
  auto n = name_node.find(getName());
  if (n == name_node.end()) {
    W_MOD("VSG object with name '" << getName() << "' not found for removal");
  }
  else if (n->second != node) {
     W_MOD("VSG object with name '" << getName() << "' removal does not match node");
  }
  else {
    name_node.erase(n);
  }

  // remove from root list if there
  if (spec.rootchild) {
    auto rchild = find(root->children.begin(), root->children.end(), node);
    if (rchild != root->children.end()) {
      root->children.erase(rchild);
    }
  }
}

void VSGObject::visible(bool vis)
{
  // nothing
}

void VSGObject::adapt(const WorldDataSpec &spec) {}

}; // namespace vsgviewer
