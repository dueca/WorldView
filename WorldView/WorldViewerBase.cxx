/* ------------------------------------------------------------------   */
/*      item            : WorldViewerBase.cxx
        made by         : Rene' van Paassen
        date            : 100122
        category        : body file
        description     :
        changes         : 100122 first version
        language        : C++
*/

#define WorldViewerBase_cxx
#include "WorldViewerBase.hxx"

#include <dueca/debug.h>
#include <dueca/DataClassRegistry.hxx>
#include <boost/lexical_cast.hpp>



WorldViewerBase::WorldViewerBase()
{
  // no action
}


WorldViewerBase::~WorldViewerBase()
{
  // no action
}

void WorldViewerBase::makeContextCurrent()
{
  // not implemented
}

void WorldViewerBase::addControllable(const dueca::GlobalId& master_id,
                                      const dueca::NameSet& cname,
                                      dueca::entryid_type entry_id,
                                      uint32_t creation_id,
                                      const std::string& data_class,
                                      const std::string& entry_label,
                                      dueca::Channel::EntryTimeAspect
                                      time_aspect)
{
  // the actual object is created by the descendant
  bool res =
    this->createControllable(master_id, cname, entry_id, creation_id,
                             data_class, entry_label, time_aspect);

  if (!res) {
    uncreatables.insert(data_class);
  }
}

bool WorldViewerBase::setEventMask(const std::string& window,
                                   unsigned long mask)
{
  return false;
}

const WorldViewerEvent& WorldViewerBase::getNextEvent
(const std::string& window)
{
  return current_event;
}

bool WorldViewerBase::adaptSceneGraph(const WorldViewConfig& adapt)
{
  return false;
}
