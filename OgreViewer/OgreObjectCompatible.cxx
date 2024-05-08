/* ------------------------------------------------------------------   */
/*      item            : OgreObjectCompatible.cxx
        made by         : Rene' van Paassen
        date            : 180903
	category        : body file
        description     :
	changes         : 180903 first version
        language        : C++
        copyright       : (c) 18 TUDelft-AE-C&S
*/

#define OgreObjectCompatible_cxx
#include "OgreObjectCompatible.hxx"
#include "AxisTransform.hxx"
#include <exception>
#include <dueca/debug.h>
#include <dueca/Ticker.hxx>
#include "OgreObjectFactory.hxx"

#define DO_INSTANTIATE
#include <dueca/dueca.h>


// static map object
std::map<std::string,OgreObjectCompatible::MeshAndGroup>
OgreObjectCompatible::typemap;



OgreObjectCompatible::OgreObjectCompatible(const WorldDataSpec& spec) :
  OgreObject(),
  manager(NULL),
  failreport(false)
{
  // further initialisation when first data comes in
}


OgreObjectCompatible::~OgreObjectCompatible()
{
  //
}

void OgreObjectCompatible::init(Ogre::SceneManager* manager)
{
  this->manager = manager;
  failreport = false;
}

void OgreObjectCompatible::connect(const GlobalId& master_id,
                                   const NameSet& cname,
                                   entryid_type entry_id,
                                   Channel::EntryTimeAspect time_aspect)
{
  r_motion.reset(new ChannelReadToken
                 (master_id, cname, ObjectMotion::classname, entry_id,
                  time_aspect, Channel::OneOrMoreEntries,
                  Channel::JumpToMatchTime));
}

void OgreObjectCompatible::iterate(TimeTickType ts,
                                   const BaseObjectMotion& base,
                                   double late, bool freeze)
{
  if (r_motion->isValid()) {
    try {
      DataReader<ObjectMotion,MatchIntervalStartOrEarlier>
        r(*r_motion, ts);

      // get the data, this might trigger a nodataavailable
      ObjectMotion o2(r.data());

      if (failreport) {
        ChannelEntryInfo ei = r_motion->getChannelEntryInfo();
        W_MOD("Recovered, first data for object " << name << " channel="
              << r_motion->getName()
              << " entry=" << ei.entry_id << " datatype=" << ei.data_class
              << " origin=" << ei.origin);
        failreport = false;
      }

      // initialize if this is the first read
      if (node == NULL && manager && o2.klass.size()) {

        try {

          this->name = o2.name.c_str();
          this->mesh_name = typemap[std::string(o2.klass.c_str())].mesh;
          this->groupname = typemap[std::string(o2.klass.c_str())].group;

          // call init?
          this->OgreObject::init(manager);
        }
        catch (const std::exception& e) {
          W_MOD("Cannot initialize OgreObjectCompatible, name="
                << this->name << " klass=" << o2.klass
                << " mesh=" << this->mesh_name
                << " group=" << this->groupname);
        }
      }

      if (node != NULL) {
        double textra = DataTimeSpec
          (r.timeSpec().getValidityStart(), ts).getDtInSeconds() + late;
        if (textra > 0.0) {
          o2.extrapolate(textra);
        }
        node->setPosition(AxisTransform::ogrePosition(o2.xyz));
        node->setOrientation(AxisTransform::ogreQuaternion(o2.attitude_q));
      }
    }
    catch (const std::exception& e) {
      ChannelEntryInfo ei = r_motion->getChannelEntryInfo();
      if (!failreport) {
        W_MOD("Cannot read ObjectMotion data for object " <<
              name << " channel=" << r_motion->getName()
              << " entry=" << ei.entry_id << " datatype=" << ei.data_class
              << " origin=" << ei.origin << " error " << e.what());
        failreport = true;
      }
    }
  }
}


OgreObjectCompatible::MeshAndGroup::MeshAndGroup(const std::string& mesh,
                                                 const std::string& group) :
  mesh(mesh), group(group) { }

//static SubContractor<OgreObjectTypeKey, OgreObjectCompatible>
//*OgreObjectCompatible_maker = new
//    SubContractor<OgreObjectTypeKey, OgreObjectCompatible>
//    ("ObjectMotion");
static SubContractor<OgreObjectTypeKey, OgreObjectCompatible>
*OgreObjectCompatible_maker2 = new
    SubContractor<OgreObjectTypeKey, OgreObjectCompatible>
    ("OgreObjectCompatible");
