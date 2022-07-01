/* ------------------------------------------------------------------   */
/*      item            : PlibStaticObject.cxx
        made by         : Rene' van Paassen
        date            : 100127
	category        : body file 
        description     : 
	changes         : 100127 first version
        language        : C++
*/

#define PlibStaticObject_cxx
#include "PlibStaticObject.hxx"
#include "RvPQuat.hxx"
#include "PlibObjectFactory.hxx"
#include <cstring>
#include <algorithm>

PlibStaticObject::PlibStaticObject(const WorldDataSpec &specification) :
  PlibObject(specification)
{
  memset(position, 0, sizeof(position));
  memset(orientation, 0, sizeof(orientation));
  for (int ii = 3; ii--; ) scale[ii] = 1.0;
  orientation[0] = 1.0;
  if (specification.coordinates.size() >= 3) {
    for (int ii = 3; ii--; ) {
      position[ii] = specification.coordinates[ii];
    }
  }
  if (specification.coordinates.size() >= 6) {
    phithtpsi2Q(orientation, 
		specification.coordinates[3]*SG_DEGREES_TO_RADIANS, 
		specification.coordinates[4]*SG_DEGREES_TO_RADIANS, 
		specification.coordinates[5]*SG_DEGREES_TO_RADIANS);
  }
  if (specification.coordinates.size() >= 9) {
    std::copy(&specification.coordinates[6], &specification.coordinates[9], 
	      scale);
  }
  if (specification.filename.size() > 0) modelfile = specification.filename[0];
  this->name = specification.name;
}

PlibStaticObject::~PlibStaticObject()
{

}

void PlibStaticObject::init(ssgRoot* scene)
{
  // do default init (model loading etc.)
  PlibObject::init(scene);

  // set with the default position
  this->update(position, orientation, scale);
}

static SubContractor<PlibObjectTypeKey, PlibStaticObject> 
*PlibStaticObject_maker = 
  new SubContractor<PlibObjectTypeKey, PlibStaticObject>("static");


PlibCenteredObject::PlibCenteredObject(const WorldDataSpec &specification) :
  PlibStaticObject(specification)
{
  //
}
  
PlibCenteredObject::~PlibCenteredObject()
{
  //
}

void PlibCenteredObject::iterate(TimeTickType ts,
                                 const BaseObjectMotion& base,
                                 double late)
{
  double cpos[3] = { position[0] == 0.0 ? base.xyz[0] : position[0], 
		     position[1] == 0.0 ? base.xyz[1] : position[1], 
		     position[2] == 0.0 ? base.xyz[2] : position[2]} ;
  this->update(cpos, orientation, scale);
}

static SubContractor<PlibObjectTypeKey, PlibCenteredObject> 
*PlibCenteredObject_maker =
  new SubContractor<PlibObjectTypeKey, PlibCenteredObject>("centered");


PlibTiledObject::PlibTiledObject(const WorldDataSpec &specification) :
  PlibStaticObject(specification)
{
  //
}


PlibTiledObject::~PlibTiledObject()
{
  //
}

void PlibTiledObject::iterate(TimeTickType ts,
                              const BaseObjectMotion& base,
                              double late)
{
  double cpos[3] = 
    { position[0] > 0.0 ? roundf(base.xyz[0] / position[0]) * position[0] : 
      ( position[0] < 0.0 ? base.xyz[0] : 0.0 ), 
      position[1] > 0.0 ? roundf(base.xyz[1] / position[1]) * position[1] :
      ( position[1] < 0.0 ? base.xyz[1] : 0.0 ), 
      position[2] > 0.0 ? roundf(base.xyz[2] / position[2]) * position[2] : 
      ( position[2] < 0.0 ? base.xyz[2] : 0.0 ) };
  this->update(cpos, orientation, scale);
}

// this has to be done with new, since the subcontractors are counted
// with smart pointer ref counting
static SubContractor<PlibObjectTypeKey, PlibTiledObject> 
*PlibTiledObject_maker = new 
  SubContractor<PlibObjectTypeKey, PlibTiledObject>("tiled");


PlibGround::PlibGround(const WorldDataSpec &specification) :
  PlibStaticObject(specification)
{
  //
}
  
PlibGround::~PlibGround()
{
  //
}
  
static int switchOffDepth( ssgState *entity)
{
  glDisable(GL_DEPTH_TEST);
  return 0;
}

static int switchOnDepth( ssgState *entity)
{
  glEnable(GL_DEPTH_TEST);
  return 0;
}

static void recursiveState(ssgEntity* ent, ssgState* state)
{
  if (ent->getNumKids()) {
    ssgBranch *b = dynamic_cast<ssgBranch*>(ent);
    assert(b != NULL);
    cout << "following branch with " << b->getNumKids() << " kids" << endl;
    for (ssgEntity *k = b->getKid(0); k != NULL; k = b->getNextKid()) {
      recursiveState(k, state);
    }
  }
  else {
    ssgLeaf *l = dynamic_cast<ssgLeaf*>(ent);
    assert (l != NULL);
    cout << "found leaf" << endl;
    ssgState* s = l->getState();
    if (s) {
      s->setStateCallback(SSG_CALLBACK_PREDRAW, switchOffDepth);
      s->setStateCallback(SSG_CALLBACK_POSTDRAW, switchOnDepth);
    }
    else {
      cout << "leaf without state" << endl;
    }
    l->setState(state);

    // update if this object used a display list
    if (l->getDListIndex()) l->makeDList();
  }
}

void PlibGround::init(ssgRoot* scene)
{
  PlibStaticObject::init(scene);
  ssgSimpleState *state = new ssgSimpleState();
  state->enable(GL_TEXTURE_2D);
  state->enable(GL_CULL_FACE);
  state->enable(GL_LIGHTING);
  state->enable(GL_BLEND);
  state->setStateCallback(SSG_CALLBACK_PREDRAW, switchOffDepth);
  state->setStateCallback(SSG_CALLBACK_POSTDRAW, switchOnDepth);
  recursiveState(model, state);
}

//static SubContractor<PlibObjectTypeKey, PlibGround> 
//*PlibGround_maker = new 
//  SubContractor<PlibObjectTypeKey, PlibGround>("ground");
