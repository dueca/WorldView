/* ------------------------------------------------------------------   */
/*      item            : SpecificationBase.cxx
        made by         : Rene' van Paassen
        date            : 190430
	category        : body file 
        description     : 
	changes         : 190430 first version
        language        : C++
        copyright       : (c) 19 TUDelft-AE-C&S
*/

#define SpecificationBase_cxx

#include "SpecificationBase.hxx"

#include <boost/lexical_cast.hpp>
#include <dueca/DataClassRegistry.hxx>
#include <debug.h>

SpecificationBase::SpecificationBase()
{
  //
}


SpecificationBase::~SpecificationBase()
{
  //
}

void SpecificationBase::addFactorySpec(const std::string& match,
                                     const WorldDataSpec& spec)
{
  latest_classdata = match;
  auto fs = factoryspecs.find(match);
  if (fs != factoryspecs.end()) {
    W_MOD("Replacing spec for " << match);
  }
  I_MOD("Adding specification for \"" << match << "\"");
  factoryspecs[match] = spec;
}

bool SpecificationBase::addCoordinates(const std::vector<double>& coords)
{
  if (latest_classdata.size()) {
    factoryspecs[latest_classdata].coordinates = coords;
    return true;
  }
  E_CNF("First define data for factory");
  return false;
}

MapSpecificationError::MapSpecificationError
(const std::string& match) :
  std::exception()
{
  reason = "MapSpecificationError: failed to find \"" + match + "\"";
}

const char* MapSpecificationError::what() const noexcept
{ return reason.c_str(); }




WorldDataSpec SpecificationBase::retrieveFactorySpec
(const std::string& dclass, const std::string& label,
 unsigned cid, bool strict)
{
  // result
  WorldDataSpec obj;

  // exact match only, match dclass, typically for static objects created
  // from the start script
  if (strict) {
    auto fs = factoryspecs.find(dclass);
    if (fs != factoryspecs.end()) {
      I_MOD("Found strict specification matching \"" << dclass << "\"")
      obj = fs->second; return obj;
      }
    throw (MapSpecificationError(dclass));
  }

  // check whether the label needs a split, and find match
  auto split = label.find('|');
  std::string tomatch = (split == std::string::npos) ?
    label : label.substr(0, split);
  std::string name = (split == std::string::npos) ?
    label : label.substr(split+1);
  std::string dataclass = dclass;

  // first try to find a specific configuration, with dataclass and match
  auto fs = tomatch.size() ?
    factoryspecs.find(dataclass + std::string(":") + tomatch) :
    factoryspecs.end();
    
  // check whether a parent class works?
  try {
    while (tomatch.size() && fs == factoryspecs.end() && dataclass.size()) {
      I_MOD("Could not find specification for \"" <<
            dataclass + std::string(":") + tomatch <<
            "\", trying parent class \"" <<
            dueca::DataClassRegistry::single().getParent(dataclass) << "\"");
      dataclass = dueca::DataClassRegistry::single().getParent(dataclass);
      fs = factoryspecs.find(dataclass + std::string(":") + tomatch);
    }
  }
  catch (const dueca::DataObjectClassNotFound& e) { }
  
  if (fs != factoryspecs.end()) {
    
    // specific configuration found for this class and label
    obj = fs->second;
    I_MOD("Found specification matching \"" << tomatch <<
          "\", with dataclass \"" << dataclass << "\"");
    
    // override name if applicable
    if (split != std::string::npos) {
      I_MOD("Adjusting specification name to \"" << name << "\"");
      obj.name = name;
    }
  }
  else {
    
    // set name to label or name part of label
    obj.name = name;
    
    // try to find a generic configuration, based on data class alone
    dataclass = dclass;
    fs = factoryspecs.find(dataclass);
    try {
      while (fs == factoryspecs.end() && dataclass.size()) {
        I_MOD("Could not find specification for \"" <<
              dataclass <<
              "\", trying parent class \"" <<
              dueca::DataClassRegistry::single().getParent(dataclass) << "\"");
        dataclass = dueca::DataClassRegistry::single().getParent(dataclass);
        fs = factoryspecs.find(dataclass + std::string(":") + tomatch);
      }
    }
    catch (const dueca::DataObjectClassNotFound& e) { }
    
    if (fs != factoryspecs.end()) {
      
      I_MOD("Found generic dataclass match to \"" << dataclass << "\"");
      
      // found a generic configuration for this class, the label is now
      // the name
      obj = fs->second;
    }
    else {
      throw (MapSpecificationError(dclass));
    }
  }

  // add the creation ID as number, to distinguish the names (needed
  // for OGRE, userful for others)
  if (obj.name.size() != 0 && obj.name.back() == '#') {
    obj.name = obj.name + boost::lexical_cast<std::string>(cid);
    I_MOD("Modifying specification name to \"" << obj.name << "\"");
  }
  return obj;
}

bool SpecificationBase::hasFactorySpec(const std::string& match)
{
  return factoryspecs.find(match) != factoryspecs.end();
}

