/* ------------------------------------------------------------------   */
/*      item            : OsgObject.cxx
        made by         : Rene' van Paassen
        date            : 090617
	category        : body file 
        description     : 
	changes         : 090617 first version
        language        : C++
*/

#define OsgObject_cxx
#include "OSGObject.hxx"
#include "AxisTransform.hxx"
#include <osg/PositionAttitudeTransform>
#include <osgDB/ReadFile>
#include <map>
#include <iostream>

#define DEB(A) cout << A << endl;

using namespace std;
using namespace osg;

OSGObject::OSGObject() :
  nodemask(0xffffffff)
{
  //
}


OSGObject::~OSGObject()
{

}

void OSGObject::init(const osg::ref_ptr<osg::Group>& root, OSGViewer* master)
{
  DEB("Reading file \"" << modelfile << "\"" << endl);
  entity = osgDB::readNodeFile(modelfile, NULL);
  if (!entity.valid()) {
    cerr << "Failed to read " << modelfile << endl;
    return;
  }  
  entity->setDataVariance(osg::Object::DYNAMIC);
  entity->setName(this->getName());
  transform = new PositionAttitudeTransform();
  transform->addChild(entity);
  root->addChild(transform);
  visible(true);
}

void OSGObject::unInit(const osg::ref_ptr<osg::Group>& root)
{
  root->removeChild(transform);
}

void OSGObject::visible(bool vis)
{
  if (vis && transform->getNodeMask() == 0) {
    transform->setNodeMask(nodemask);
  }
  else if (!vis && transform->getNodeMask() != 0) {
    nodemask = transform->getNodeMask();
    transform->setNodeMask(0);
  }
}
      
