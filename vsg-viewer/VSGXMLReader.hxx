/* ------------------------------------------------------------------   */
/*      item            : VSGXMLReader.hxx
        made by         : Rene van Paassen
        date            : 230201
        category        : header file
        description     :
        changes         : 230201 first version
        language        : C++
        copyright       : (c) 23 TUDelft-AE-C&S
*/

#pragma once

#include <string>
#include "VSGViewer.hxx"

namespace vsgviewer {

/** Reads an XML file with type definitions and optionally static
    object instatiations. An xml file may be queried to provide a
    translation of XML tags to standard coordinate input. */
class VSGXMLReader
{
  /** Mapping from a coordinate name to a range of elements in the
      coordinates vector */
  struct CoordinateMapping {

    /** First element in the vector */
    const unsigned offset;
    
    /** Number of elements */
    const unsigned size;

    /** Construct */
    CoordinateMapping(unsigned offset, unsigned size);
  };

  /** Mappings per object for coordinates */
  struct ObjectCoordinateMapping {

    /** The various coordinate mappings */
    std::map<std::string,CoordinateMapping> mappings;

    /** Get the values for a specific mapping */
    bool getMapping(unsigned &offset, unsigned &size, const std::string& cname);
  };

  /** All objects */
  std::map<std::string,ObjectCoordinateMapping> object_mappings;

public:
  /** Construct the reader */
  VSGXMLReader(const std::string& definitions);

  /** Read an XML file with type definitions and optionally static
      object instantiations. */
  bool readWorld(const std::string& file, VSGViewer &viewer);
};

}; // namespace
