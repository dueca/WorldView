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

#include "VSGTransform.hxx"
#include "AxisTransform.hxx"
#include <dueca/ChannelReadToken.hxx>
#include <dueca/debug.h>
#include "VSGObjectFactory.hxx"

namespace vsgviewer {

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

static auto VSGStaticMatrixTransform_maker =
  new SubContractor<VSGObjectTypeKey, VSGStaticMatrixTransform>(
    "static-transform", "Constant matrix transform");

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
