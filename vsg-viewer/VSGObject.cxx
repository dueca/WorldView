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
  DEB("Reading file \"" << modelfile << "\"" << endl);
  entity = vsgXchange::readNodeFile(modelfile, NULL);
  if (!entity.valid()) {
    cerr << "Failed to read " << modelfile << endl;
    return;
  }  
  entity->setDataVariance(vsg::Object::DYNAMIC);
  entity->setName(this->getName());
  transform = new vsg::Transform();
  transform->addChild(entity);
  root->addChild(transform);
  visible(true);
}

void VSGObject::unInit(const vsg::ref_ptr<vsg::Group>& root)
{
  root->removeChild(transform);
}

void VSGObject::visible(bool vis)
{
  if (vis && transform->getNodeMask() == 0) {
    transform->setNodeMask(nodemask);
  }
  else if (!vis && transform->getNodeMask() != 0) {
    nodemask = transform->getNodeMask();
    transform->setNodeMask(0);
  }
}
      
