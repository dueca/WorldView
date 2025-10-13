/* ------------------------------------------------------------------   */
/*      item            : VSGTransform.cxx
        made by         : Rene' van Paassen
        date            : 230126
        category        : body file
        description     :
        changes         : 230126 first version
        language        : C++
        copyright       : (c) 23 TUDelft-AE-C&S
*/

#include "AxisTransform.hxx"
#include "VSGObjectFactory.hxx"
#include "VSGTransform.hxx"
#include <dueca/ChannelReadToken.hxx>
#include <dueca/debug.h>

namespace vsgviewer {

// ----------------- static transformation, non-moving objects ----------
VSGStaticMatrixTransform::VSGStaticMatrixTransform(const WorldDataSpec &data) :
  transform(vsg::MatrixTransform::create())
{
  name = data.name;
  parent = data.parent;
  if (data.coordinates.size() >= 9) {
    transform->matrix = vsg::scale(vsgScale(vrange(data.coordinates, 6, 3)));
  }
  if (data.coordinates.size() >= 6) {
    transform->matrix = vsgRotation(vsg::radians(data.coordinates[3]),
                                    vsg::radians(data.coordinates[4]),
                                    vsg::radians(data.coordinates[5])) *
                        transform->matrix;
  }
  if (data.coordinates.size() >= 3) {
    transform->matrix = vsg::translate(vsgPos(vrange(data.coordinates, 0, 3))) *
                        transform->matrix;
  }
  D_MOD("Created static matrix transform, name=" << name);
}

VSGStaticMatrixTransform::~VSGStaticMatrixTransform()
{
  D_MOD("Destroying static matrix transform, name=" << name);
}

void VSGStaticMatrixTransform::init(const vsg::ref_ptr<vsg::Group> &root,
                                    VSGViewer *master)
{
  transform->setValue("name", name);
  auto par = findParent(root, parent);
  if (!par) {
    W_MOD("Cannot find parent='" << parent << "', for name=" << name
                                 << ", attaching to root");
    par = root;
  }
  par->addChild(transform);
  D_MOD("VSG create static matrix transform, name=" << name);
}

// should be? https://github.com/vsg-dev/VulkanSceneGraph/discussions/1050
void VSGStaticMatrixTransform::unInit(const vsg::ref_ptr<vsg::Group> &root)
{
  auto par = findParent(root, parent);
  if (!par) {
    W_MOD("Cannot find parent='" << parent << "', for name=" << name
                                 << ", detaching from root");
    par = root;
  }

  auto it = std::find(par->children.begin(), par->children.end(), transform);
  if (it != par->children.end()) {
    par->children.erase(it);
  }
}

static auto VSGStaticMatrixTransform_maker =
  new SubContractor<VSGObjectTypeKey, VSGStaticMatrixTransform>(
    "static-transform", "Constant matrix transform");

// ---------------------- centered on observer ----------------------
VSGCenteredTransform::VSGCenteredTransform(const WorldDataSpec &data) :
  transform(vsg::MatrixTransform::create())
{
  name = data.name;
  parent = data.parent;
  if (data.coordinates.size() >= 9) {
    base_transform =
      vsg::scale(data.coordinates[6], data.coordinates[7], data.coordinates[8]);
  }
  else {
    base_transform = vsg::scale(1.0, 1.0, 1.0);
  }
  if (data.coordinates.size() >= 6) {
    base_transform = vsgRotation(vsg::radians(data.coordinates[3]),
                                 vsg::radians(data.coordinates[4]),
                                 vsg::radians(data.coordinates[5])) *
                     base_transform;
  }
  if (data.coordinates.size() >= 3) {
    base_transform =
      vsg::translate(vsgPos(vrange(data.coordinates, 0, 3))) * base_transform;
  }
  D_MOD("Created centered matrix transform, name=" << name);
}

VSGCenteredTransform::~VSGCenteredTransform()
{
  D_MOD("Destroying centered matrix transform, name=" << name);
}

void VSGCenteredTransform::init(const vsg::ref_ptr<vsg::Group> &root,
                                VSGViewer *master)
{
  transform->setValue("name", name);
  auto par = findParent(root, parent);
  if (!par) {
    W_MOD("Cannot find parent='" << parent << "', for name=" << name
                                 << ", attaching to root");
    par = root;
  }
  par->addChild(transform);
  D_MOD("VSG create centered transform, name=" << name);
}

void VSGCenteredTransform::unInit(const vsg::ref_ptr<vsg::Group> &root)
{
  auto par = findParent(root, parent);
  if (!par) {
    W_MOD("Cannot find parent='" << parent << "', for name=" << name
                                 << ", detaching from root");
    par = root;
  }

  auto it = std::find(par->children.begin(), par->children.end(), transform);
  if (it != par->children.end()) {
    par->children.erase(it);
  }
}


void VSGCenteredTransform::iterate(TimeTickType ts,
                                   const BaseObjectMotion &base, double late,
                                   bool freeze)
{
  transform->matrix = vsg::translate(vsgPos(base.xyz)) * base_transform;
}

bool VSGCenteredTransform::forceActive() { return true; }

static auto VSGCenteredTransform_maker =
  new SubContractor<VSGObjectTypeKey, VSGCenteredTransform>(
    "centered-transform", "matrix transform centered on observer");

// ---------------------- tiled, following observer ---------------------
VSGTiledTransform::VSGTiledTransform(const WorldDataSpec &data) :
  transform(vsg::MatrixTransform::create())
{
  name = data.name;
  parent = data.parent;

  // coordinates 4-12 are prientation, scale and offset position
  if (data.coordinates.size() >= 12) {
    base_transform = vsg::scale(data.coordinates[9], data.coordinates[10],
                                data.coordinates[11]);
  }
  else {
    base_transform = vsg::scale(1.0, 1.0, 1.0);
  }

  if (data.coordinates.size() >= 9) {
    base_transform = vsgRotation(vsg::radians(data.coordinates[6]),
                                 vsg::radians(data.coordinates[7]),
                                 vsg::radians(data.coordinates[8])) *
                     base_transform;
  }
  if (data.coordinates.size() >= 6) {
    base_transform =
      vsg::translate(
        vsgPos(data.coordinates[3], data.coordinates[4], data.coordinates[5])) *
      base_transform;
  }
  if (data.coordinates.size() >= 3) {
    tile_size =
      vsg::dvec3(data.coordinates[0], data.coordinates[1], data.coordinates[2]);
  }
  else {
    tile_size = vsg::dvec3(1.0, 1.0, 1.0);
  }
  D_MOD("Created Tiled matrix transform, name=" << name);
}

VSGTiledTransform::~VSGTiledTransform()
{
  D_MOD("Destroying Tiled matrix transform, name=" << name);
}

void VSGTiledTransform::init(const vsg::ref_ptr<vsg::Group> &root,
                             VSGViewer *master)
{
  transform->setValue("name", name);
  auto par = findParent(root, parent);
  if (!par) {
    W_MOD("Cannot find parent='" << parent << "', for name=" << name
                                 << ", attaching to root");
    par = root;
  }
  par->addChild(transform);
  D_MOD("VSG create Tiled transform, name=" << name);
}

void VSGTiledTransform::unInit(const vsg::ref_ptr<vsg::Group> &root)
{
  auto par = findParent(root, parent);
  if (!par) {
    W_MOD("Cannot find parent='" << parent << "', for name=" << name
                                 << ", detaching from root");
    par = root;
  }

  auto it = std::find(par->children.begin(), par->children.end(), transform);
  if (it != par->children.end()) {
    par->children.erase(it);
  }
}


void VSGTiledTransform::iterate(TimeTickType ts, const BaseObjectMotion &base,
                                double late, bool freeze)
{
  dueca::fixvector<3, double> delta = {
    tile_size[0] ? std::floor(base.xyz[0] / tile_size[0]) * tile_size[0] : 0.0,
    tile_size[1] ? std::floor(base.xyz[1] / tile_size[1]) * tile_size[1] : 0.0,
    tile_size[2] ? std::floor(base.xyz[2] / tile_size[2]) * tile_size[2] : 0.0 };

  // apply the base transform to the observer position
  transform->matrix = vsg::translate(vsgPos(delta)) * base_transform;
}

bool VSGTiledTransform::forceActive() { return true; }

static auto VSGTiledTransform_maker =
  new SubContractor<VSGObjectTypeKey, VSGTiledTransform>(
    "tiled-transform", "matrix transform tiled on observer");

// -------- dynamic, entity-controlled transformation -------------------
VSGMatrixTransform::VSGMatrixTransform(const WorldDataSpec &data) :
  transform(vsg::MatrixTransform::create()),
  scale()
{
  if (data.coordinates.size() >= 3) {
    scale = vsg::scale(vsgScale(vrange(data.coordinates, 0, 3)));
  }
  name = data.name;
  D_MOD("Created matrix transform, name=" << name);
}

VSGMatrixTransform::~VSGMatrixTransform()
{
  D_MOD("Destroying matrix transform, name=" << name);
}

void VSGMatrixTransform::init(const vsg::ref_ptr<vsg::Group> &root,
                              VSGViewer *master)
{
  transform->setValue("name", name);
  auto par = findParent(root, parent);
  if (!par) {
    W_MOD("Cannot find parent='" << parent << "', for name=" << name
                                 << ", attaching to root");
    par = root;
  }
  par->addChild(transform);
  D_MOD("VSG create matrix transform, name=" << name);
}

void VSGMatrixTransform::connect(const GlobalId &master_id,
                                 const NameSet &cname, entryid_type entry_id,
                                 Channel::EntryTimeAspect time_aspect)
{
  r_motion.reset(new ChannelReadToken(
    master_id, cname, BaseObjectMotion::classname, entry_id, time_aspect,
    Channel::OneOrMoreEntries, Channel::JumpToMatchTime));
}

void VSGMatrixTransform::iterate(TimeTickType ts, const BaseObjectMotion &base,
                                 double late, bool freeze)
{
  if (r_motion->isValid()) {
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
        transform->matrix = vsg::translate(vsgPos(o2.xyz)) *
                            vsg::rotate(vsgQuat(o2.attitude_q)) * scale;
      }
      else {
        transform->matrix = vsg::translate(vsgPos(r.data().xyz)) *
                            vsg::rotate(vsgQuat(r.data().attitude_q)) * scale;
      }
    }
    catch (const std::exception &e) {
      W_MOD("Cannot read BaseObjectMotion data for object " << name << " error "
                                                            << e.what());
    }
  }
}

static auto VSGMatrixTransform_maker =
  new SubContractor<VSGObjectTypeKey, VSGMatrixTransform>(
    "transform", "Controllable matrix transform");

}; // namespace vsgviewer
