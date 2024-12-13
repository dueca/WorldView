/* ------------------------------------------------------------------   */
/*      item            : VSGModel.cxx
        made by         : Rene' van Paassen
        date            : 230126
        category        : body file
        description     :
        changes         : 230126 first version
        language        : C++
        copyright       : (c) 23 TUDelft-AE-C&S
*/

#include "VSGModel.hxx"
#include "VSGViewer.hxx"
#include "VSGObjectFactory.hxx"
#include <dueca/debug.h>
#include <pugixml.hpp>
#include "VSGXMLReader.hxx"

#if 0

// not yet ready

namespace vsgviewer {

  template <class V>
  VSGIntersect::VSGIntersect(const WorldDataSpec& data) :
    V(WorldDataSpec(data, 2)),
    channelname(data.filename[data.filename.size() - 1U])
  {
    pugi::xml_document doc;
    auto result = doc.load_file(data.filename[data.filename.size() - 2U]);
    if (result) {
      auto _isects = doc.child("intersectors");
      for (auto _isect = _isects.child("intersector"); _isect;
           _isect.next_sibling("intersector")) {
        auto _type = _isect.attribute("type").value();
        auto _start = getValues(_isects.child("start").value());
        auto _end = getValues(_isects.child("end").value());
        if (_type == "line" && _start.size() == 3 && _end.size() == 3) {
          intersectors.push_back
            (vsg::LineSegmentIntersector::create(_start, _end));
        }
        else {
          W_MDO("Reading " << data.filename[data.filename.size() - 2U] <<
                " ignoring invalid intersector type " << _type);
        }
      }
    }
    else {
      W_MDO("Cannot read/parse " << data.filename[data.filename.size() - 2U]);
    }
  }


  VSGIntersect::~VSGIntersect()
  {
    D_MOD("Destroying intersect probe model, name=" << name);
  }

  void VSGIntersect::connect(const GlobalId& master_id, const NameSet& cname,
                             entryid_type entry_id,
                             Channel::EntryTimeAspect time_aspect)
  {
    this->V::connect(master_id, cname, entry_id, time_aspect);

    w_intersects.reset
      (new ChannelWriteToken(master_id, NameSet(channelname),
                             getclassname<Intersections>(), name,
                             Channel::OneOrMoreEntries, time_aspect));
  }

  void VSGIntersect::init(const vsg::ref_ptr<vsg::Group>& root,
                          VSGViewer* master)
  {
    // init parent first
    this->V::init(root, master);
    for (auto &is: intersectors) {
      root->accept(*is);
    }
  }

  void VSGIntersect::iterate(TimeTickType ts,
                             const BaseObjectMotion& base,
                             double late, bool freeze)
  {
    // set the reverse transform from this vehicle's
    //rev_transform->matrix = transform->
    this->V::iterate(ts, base, late);
  }

  static auto VSGIntersectModel_maker = new
    SubContractor<VSGObjectTypeKey,VSGIntersect<VSGModel> >
    ("intersect-model",
     "3D model from external modeling application, with intersect check");

}; // end namespace
#endif