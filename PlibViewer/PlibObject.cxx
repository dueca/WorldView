/* ------------------------------------------------------------------   */
/*      item            : PlibObject.cxx
        made by         : Rene' van Paassen
        date            : 100126
        category        : body file
        description     :
        changes         : 100126 first version
        language        : C++
*/

#define PlibObject_cxx
#include "PlibObject.hxx"
#include "AxisTransform.hxx"
#include "PlibObjectFactory.hxx"
#include <dueca/debug.h>
#include <iostream>
#include <map>

using namespace std;

// keep a check of loaded models
// convert this to a proper static class, so it can destruct the stuff
// at eol?
typedef map<string, ssgEntity *> ModelRecord;
static ModelRecord loaded_models;

ssgEntity *PlibObject::load(const string &file)
{
  // heightening efficiency at a local scale; previously loaded models
  // are cloned

  ssgEntity *object = NULL;

  // find if this has been loaded previously
  ModelRecord::iterator ii = loaded_models.find(file);
  if (ii != loaded_models.end()) {
    cout << "Using a clone of the model in " << file << endl;
    object = dynamic_cast<ssgEntity *>(ii->second->clone(0));
    if (object)
      return object;
    cerr << "Failed dynamic cast of the clone" << endl;
  }

  if (file.compare(file.size() - 3, 3, string(".ac")) == 0) {
    object = ssgLoadAC(file.c_str());
  }
  else if (file.compare(file.size() - 4, 4, string(".obj")) == 0) {
    object = ssgLoadOBJ(file.c_str());
  }
  else if (file.compare(file.size() - 4, 4, string(".3ds")) == 0) {
    object = ssgLoad3ds(file.c_str());
  }
  else if (file.compare(file.size() - 4, 4, string(".wrl")) == 0) {
    object = ssgLoadVRML1(file.c_str());
  }

  if (!object) {
    cerr << "Could not load object from file " << file << endl;
  }
  else {
    ssgFlatten(object);
    ssgStripify(object);

    if (ii == loaded_models.end()) {
      loaded_models[file] = object;
      object->ref(); // keep object beyond ssg use
    }
  }
  return object;
}

PlibObject::PlibObject(const WorldDataSpec &spec) :
  to_model(NULL),
  model(NULL)
{
  if (spec.filename.size()) {
    modelfile = spec.filename[0];
  }
  this->name = spec.name;
  for (double &s : scale) {
    s = 1.0;
  }
}

PlibObject::~PlibObject() {}

void PlibObject::init(ssgRoot *scene)
{
  to_model = new ssgTransform();
  model = load(modelfile);
  if (!model) {
    cerr << "Unable to load model \"" << modelfile << '"' << endl;
    return;
  }
  cout << "loaded model \"" << modelfile << '"' << endl;
  to_model->addKid(model);
  scene->addKid(to_model);
}

void PlibObject::draw(int viewno)
{
  // nothing needed for plib. Overridden for some
}

void PlibObject::observerUpdate(const double xyz[3], const double quat[4])
{
  // overridden for some
}

void PlibObject::visible(bool vis)
{
  // safety for failed models
  if (model == NULL)
    return;

  // visibility is controlled by adding/removing model as kid from transform
  if (vis) {
    if (to_model->searchForKid(model) < 0) {
      to_model->addKid(model);
    }
  }
  else {
    to_model->removeKid(model);
  }
}

void PlibObject::update(double xyz[3], double quat[4])
{
  sgCoord position = AxisTransform::plibCoordinates(xyz, quat);
  to_model->setTransform(&position);
}

void PlibObject::update(const ObjectMotion &o, double time, double late)
{
  ObjectMotion o2(o);
  if (late > 0.0) {
    o2.extrapolate(late);
  }
  this->update(o2.xyz, o2.attitude_q);
}

void PlibObject::update(double xyz[3], double quat[4], double scale[3])
{
  sgCoord position = AxisTransform::plibCoordinates(xyz, quat);
  to_model->setTransform(&position, scale[1], scale[0], scale[2]);
}

void PlibObject::connect(const GlobalId &master_id, const NameSet &cname,
                         entryid_type entry_id,
                         Channel::EntryTimeAspect time_aspect)
{
  r_motion.reset(new ChannelReadToken(
    master_id, cname, BaseObjectMotion::classname, entry_id, time_aspect,
    Channel::OneOrMoreEntries, Channel::JumpToMatchTime));
}

void PlibObject::iterate(TimeTickType ts, const BaseObjectMotion &base,
                         double late, bool freeze)
{
  if (r_motion->isValid() && to_model != NULL) {
    try {
      DataReader<BaseObjectMotion, MatchIntervalStartOrEarlier> r(*r_motion,
                                                                  ts);
      if (r.data().dt != 0.0) {
        BaseObjectMotion o2(r.data());
        double textra =
          DataTimeSpec(r.timeSpec().getValidityStart(), ts).getDtInSeconds() +
          late;
        if (textra > 0.0) {
          o2.extrapolate(textra);
        }
        sgCoord position =
          AxisTransform::plibCoordinates(o2.xyz, o2.attitude_q);
        to_model->setTransform(&position, scale[1], scale[0], scale[2]);
      }
      else {
        sgCoord position =
          AxisTransform::plibCoordinates(r.data().xyz, r.data().attitude_q);
        to_model->setTransform(&position, scale[1], scale[0], scale[2]);
      }
    }
    catch (const std::exception &e) {
      W_MOD("Cannot read BaseObjectMotion data for object " << name << " error "
                                                            << e.what());
    }
  }
}

static SubContractor<PlibObjectTypeKey, PlibObject> *PlibObjectMoving_maker =
  new SubContractor<PlibObjectTypeKey, PlibObject>("PlibObjectMoving");
