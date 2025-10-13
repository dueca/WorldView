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
#include <vsgXchange/all.h>
#include <dueca/debug.h>

namespace vsgviewer {

VSGObject::VSGObject()
{
  //
}

VSGObject::~VSGObject()
{
  //
}

bool VSGObject::forceActive() { return false; }

VSGCullGroup::VSGCullGroup(const WorldDataSpec &data) :
  VSGObject()
{
  name = data.name;
  parent = data.parent;
  D_MOD("Created cull group, name=" << name);
}

VSGCullGroup::~VSGCullGroup() { D_MOD("Destroying cull group, name=" << name); }

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

vsg::ref_ptr<vsg::Group> findParent(vsg::ref_ptr<vsg::Group> root,
                                    const std::string &name)
{
  std::string iname;
  vsg::ref_ptr<vsg::Group> res;

  if (!name.size()) {
    D_MOD("Searching for parent with empty name, assume root");
    return root;
  }
  if (root->getValue("name", iname) && iname == name) {
    return root;
  }

  for (auto const &i : root->children) {
    vsg::ref_ptr<vsg::Group> g = i.cast<vsg::Group>();
    if (g) {
      res = _findParent(g, name);
      if (res)
        return res;
    }
  }

  D_MOD("Could not find node '" << name << "', attaching to root");
  return root;
}

void VSGObject::visible(bool vis)
{
  // nothing
}

}; // namespace vsgviewer
