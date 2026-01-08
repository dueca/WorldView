/* ------------------------------------------------------------------   */
/*      item            : VSGXMLReader.cxx
        made by         : Rene' van Paassen
        date            : 230201
        category        : body file
        description     :
        changes         : 230201 first version
        language        : C++
        copyright       : (c) 23 TUDelft-AE-C&S
*/

#include "VSGViewer.hxx"
#include "VSGXMLReader.hxx"
#include <dueca/debug.h>
#include <exception>
#include <pugixml.hpp>

namespace vsgviewer {

/* exception to throw when reading fails. */
struct error_reading_vsg_xml : public std::exception
{
  const char *what() { return "Cannot read XML file"; }
};

// trim from left (in place)
inline void ltrim(std::string &s)
{
  s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                  [](int ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
inline void rtrim(std::string &s)
{
  s.erase(
    std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); })
      .base(),
    s.end());
}

// trim, but give me a copy
inline std::string trim_copy(std::string s)
{
  ltrim(s);
  rtrim(s);
  return s;
}

// from a comma-separated list, interpret number values
inline std::vector<double> getValues(const std::string &s)
{
  std::stringstream invals(s);
  std::vector<double> res;
  double tmp;
  while (invals >> tmp) {
    res.push_back(tmp);
    if (invals.peek() == ',') {
      invals.ignore();
    }
  }
  return res;
}

// from a comma-separated list, interpret string values
inline std::vector<std::string> getStrings(const std::string &s, unsigned n)
{
  std::vector<std::string> res;
  if (n == 1) {
    res.push_back(trim_copy(s));
  }
  else {
    std::string::size_type idx0 = 0;
    std::string::size_type idx1 = s.find(',', idx0);
    while (idx1 != std::string::npos) {
      res.push_back(trim_copy(s.substr(idx0, idx1 - idx0 - 1)));
      idx0 = idx1 + 1;
      idx1 = s.find(',', idx0);
    }
    res.push_back(trim_copy(s.substr(idx0)));
  }
  return res;
}

VSGXMLReader::CoordinateMapping::CoordinateMapping(unsigned offset,
                                                   unsigned size,
                                                   bool isnumber) :
  offset(offset),
  size(size),
  isnumber(isnumber)
{}

bool VSGXMLReader::ObjectCoordinateMapping::getMapping(unsigned &offset,
                                                       unsigned &size,
                                                       bool &isnumber,
                                                       const std::string &cname) const
{
    // if no coordinate names given, assume this simply fills the array
  if (!cname.size()) {
    return true;
  }

  const auto idx = mappings.find(cname);
  if (idx == mappings.end()) {
    return false;
  }
  offset = idx->second.offset;
  size = idx->second.size ? idx->second.size : size;
  isnumber = idx->second.isnumber;
  return true;
}

VSGXMLReader::VSGXMLReader(const std::string &definitions)
{
  // shortcut exit
  if (!definitions.size()) {
    W_MOD("Empty definitions file for reader!");
    return;
  }

  // read the coordinate definitions
  pugi::xml_document doc;
  auto result = doc.load_file(definitions.c_str());

  if (result) {
    // basic element is maps
    auto _maps = doc.child("maps");

    // run over all defined types
    for (auto _type = _maps.child("type"); _type;
         _type = _type.next_sibling("type")) {
      auto nm = object_mappings.emplace(_type.attribute("name").value(),
                                        ObjectCoordinateMapping());

      for (auto _coord = _type.child("param"); _coord;
           _coord = _coord.next_sibling("param")) {
        unsigned offset = _coord.attribute("offset").as_uint();
        unsigned nelts = _coord.attribute("size").as_uint(1U);
        bool isnumber = _coord.attribute("isnumber").as_bool(true);
        std::string name = trim_copy(_coord.child_value());
        nm.first->second.mappings.emplace(
          std::piecewise_construct, std::forward_as_tuple(name),
          std::forward_as_tuple(offset, nelts, isnumber));
      }
    }
  }
  else {
    throw error_reading_vsg_xml();
  }
}

template <typename MAP>
static void processParameters(WorldDataSpec &spec, const pugi::xml_node &node,
                              const MAP &object_mappings)
{
  // first get all the files/string data (old style)
  if (spec.filename.size() == 0) {
    for (auto fname = node.child("file"); fname;
         fname = fname.next_sibling("file")) {
      spec.filename.push_back(trim_copy(fname.child_value()));
    }
  }
  else {
    for (auto fname = node.child("file"); fname;
         fname = fname.next_sibling("file")) {
      W_MOD("In VSG file, ignoring file argument " << fname.child_value());
    }
  }

  // now get&translate all parameters
  for (auto coord = node.child("param"); coord;
       coord = coord.next_sibling("param")) {
    std::string _label = coord.attribute("name").value();

    // for definition
    unsigned offset, n;
    bool isnumber;
    auto idx = object_mappings.find(spec.type);

    // find the mapping
    if (idx != object_mappings.end()) {
      if (!idx->second.getMapping(offset, n, isnumber, _label)) {
        W_MOD("Param index '" << _label << "' for type '" << spec.type
                              << "' missing");
        continue;
      }
    }
    else if (_label.size()) {
      W_MOD("Param mappings for type '" << spec.type << "' missing");
      continue;
    }
    if (isnumber) {
      auto values = getValues(coord.child_value());
      spec.setCoordinates(offset, n, values);
    }
    else {
      auto values = getStrings(coord.child_value(), n);
      spec.setStrings(offset, n, values);
    }
  }

  // and are there any children
  for (auto child = node.child("child"); child;
       child = child.next_sibling("child")) {
    float ratio = child.attribute("ratio").as_float(0.0f);
    auto name = child.child_value();
    if (name) {
      spec.children.emplace_back(name, ratio);
    }
  }
}

bool VSGXMLReader::readWorld(const std::string &file, VSGViewer &viewer)
{
  pugi::xml_document doc;
  auto result = doc.load_file(file.c_str(), pugi::parse_trim_pcdata);
  if (!result) {
    W_MOD("Cannot read vsg world from " << file);
    return false;
  }

  // get the container
  auto world = doc.child("world");

  // each declaration gets translated in data for a createable object
  // either through direct creation or from a channel entry
  for (auto def = world.child("template"); def;
       def = def.next_sibling("template")) {

    // Prepare the data for the object
    WorldDataSpec spec;

    // required stuff is key and type
    auto _key = def.attribute("key");
    auto _type = def.attribute("type");
    auto _name = def.attribute("name");
    bool rootchild = def.attribute("root").as_bool(false);

    // test required attributes are there
    if (!_key || !_type) {
      W_MOD("Skipping template, missing key and type");
      continue;
    }

    // store the data
    spec.type = _type.value();
    spec.rootchild = rootchild;
    if (_name) {
      spec.name = _name.value();
    }

    processParameters(spec, def, object_mappings);

    // add the object to the viewer templates
    viewer.addFactorySpec(_key.value(), spec);
  }

  // process removals
  for (auto sta = world.child("remove"); sta;
       sta = sta.next_sibling("remove")) {
    auto name = sta.attribute("name");
    viewer.removeStatic(name.as_string());
  }

  // process the static additions
  for (auto sta = world.child("static"); sta;
       sta = sta.next_sibling("static")) {

    auto template_id = sta.attribute("template");
    auto name = sta.attribute("name");
    auto type = sta.attribute("type");
    bool rootchild = sta.attribute("root").as_bool(false);

    WorldDataSpec spec;

    if (!template_id && !type) {
      E_MOD("Need template or type for object '" << name.value() << "'");
      return false;
    }

      // if a template supplied, use that as basis
    if (template_id) {
      try {
        spec = viewer.retrieveFactorySpec(template_id.value(), "", 0U, true);
      }
      catch (const MapSpecificationError &e) {
        E_MOD("Cannot find template '" << template_id.value()
                                       << "' for static '" << name.value()
                                       << "'");
        return false;
      }
    }

    // overwrite with the type if specified
    if (type) {
      spec.type = type.value();
    }

      // name / parent from default (template, none) or overwritten
    if (name) {
      spec.name = name.value();
    }
    spec.rootchild = rootchild;

    // run the files, overwrite all if available
    if (sta.child("file")) {
      spec.filename.clear();
    }

    // param, file and child processing
    processParameters(spec, sta, object_mappings);

    // create the object
    viewer.createStatic(spec);
  }

  // modifications
  for (auto sta = world.child("modify"); sta;
       sta = sta.next_sibling("modify")) {
    auto name = sta.attribute("name");

    // fill in the name
    WorldDataSpec spec;
    spec.name = name.value();

    // get existing config
    if (viewer.findExisting(spec)) {

      processParameters(spec, sta, object_mappings);

      // make the update
      I_MOD("VSG object mod " << spec);
      viewer.modifyStatic(spec);
    }
    else {

      W_MOD("Cannot find existing for modification:" << spec.name);
    }
  }

  return true;
}

}; // namespace vsgviewer
