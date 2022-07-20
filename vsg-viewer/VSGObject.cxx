/* ------------------------------------------------------------------   */
/*      item            : VsgObject.cxx
        made by         : Rene' van Paassen
        date            : 090617
	category        : body file 
        description     : 
	changes         : 090617 first version
        language        : C++
*/

#define VsgObject_cxx
#include "VSGObject.hxx"
#include <vsgXchange/all.h>
#include "AxisTransform.hxx"
#include <map>
#include <iostream>

#define DEB(A) cout << A << endl;

using namespace std;
using namespace vsg;

VSGObject::VSGObject() :
  nodemask(0xffffffff)
{
  //
}


VSGObject::~VSGObject()
{
  
}

void VSGObject::init(const vsg::ref_ptr<vsg::Group>& root, VSGViewer* master)
{
  // one options pointer is enough
  static vsg::ref_ptr<vsg::Options> options;
  if (options.get() == NULL) {
    options = vsg::Options::create();
    options->add(vsgXchange::all::create());
  }
  
  DEB("Reading file \"" << modelfile << "\"" << endl);
  entity = vsg::read_cast<vsg::Node>(modelfile, options);
  if (!entity.valid()) {
    cerr << "Failed to read " << modelfile << endl;
    return;
  }  
  //entity->setDataVariance(vsg::Object::DYNAMIC);
  // entity->setName(this->getName());
  transform = vsg::MatrixTransform::create();
  transform->addChild(entity);
  root->addChild(transform);
  visible(true);
}

void VSGObject::unInit(const vsg::ref_ptr<vsg::Group>& root)
{
#if 0
  // apparently not possible
  root->removeChild(transform);
#endif
}

void VSGObject::visible(bool vis)
{
#if 0
  if (vis && transform->getNodeMask() == 0) {
    transform->setNodeMask(nodemask);
  }
  else if (!vis && transform->getNodeMask() != 0) {
    nodemask = transform->getNodeMask();
    transform->setNodeMask(0);
  }
#endif
}
      
