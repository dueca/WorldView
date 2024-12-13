/* ------------------------------------------------------------------   */
/*      item            : SpecificationBase.hxx
        made by         : Rene van Paassen
        date            : 190430
        category        : header file
        description     :
        changes         : 190430 first version
        language        : C++
        copyright       : (c) 2019 TUDelft-AE-C&S
*/

#ifndef SpecificationBase_hxx
#define SpecificationBase_hxx

#include "comm-objects.h"
#include <map>
#include <string>

/** Exception for failing to look up specification */
class MapSpecificationError : public std::exception
{
  /** Message to print */
  std::string reason;

public:
  /** print the problem. */
  const char *what() const noexcept;

  /** constructor, the argument shows what the function tried to match */
  MapSpecificationError(const std::string &match);
};

/** Common base class for WorldViewerBase and ListenerBase, provides a
    mapping between (graphical/audio) object classes and their
    specification.

    Using a string-based key, a WorldDataSpec object can be returned.
    These WorldDataSpec objects contain the data for creating the
    visual or audio representations.

    To enable the efficient creation of graphical and audio objects in
    graphics/audio back-ends in a DUECA/DUSIME simulation, some
    conventions on a search for graphics/audio models based on the
    creation of a channel entry is used.

    - Strict search (with a strict parameter), is done on a given key
      only, and returns data with an exact match. This is normally
      done for static or self-driven objects in the environment (i.e.,
      those not needing data from the simulation).

    - Search on label (normally from a created channel entry) and
      dataclass (likewise) is performed next. This search will be
      repeated with parent classes.

    - The last search, provided for compatibility, is performed on
      data class alone.

   The returned WorldDataSpec objects contain a "type" field, that is
   used in a second stage to locate a mapped factory method, that can
   create the requested graphical or audio object. In this manner
   extensible back-ends can be created.

   One audio back-end (with OpenAL) and several graphics library back-ends
   use these facilities, to varying degrees.

   TLDR; When you create an entry in a channel for WorldView or
   WorldListener, the WorldView or WorldListener back-end looks up a
   WorldDataSpec specification, based on a match with the dataclass of
   the entry you created, and its label. This WorldDataSpec contains a
   type entry, which is used to find the type of object (sound, moving
   or static, skydome, 3D object etc.) that needs to be created. The
   data in the SpecificationBase (sound/visual files, coordinates) is
   then used to create the sound or visual object. All that is linked
   to your channel entry, so you can control the object.
*/
class SpecificationBase
{
  /** type for map with specifications (mesh names etc) for objects created
      through the factory */
  typedef std::map<std::string, WorldDataSpec> factoryspecs_t;

  /** Map with information for factory methods */
  factoryspecs_t factoryspecs;

  /** Remember latest entered */
  std::string latest_classdata;

public:
  /** Add specification for an object

      When implementing a factory pattern for the creation of graphic
      objects, either from a script or to match objects appearing in a
      channel, configuration data for these objects can be added
      through this call.

      Using addFactorySpec, data specifications are added to a map.
      These contain coordinates, (file) names, object name, and a
      type member to associate the graphical object to the factory that
      can create the object.

      @param match   String to match against.
      @param spec    Specification.
  */
  void addFactorySpec(const std::string &match, const WorldDataSpec &spec);

  /** Set coordinates to the latest added factory specifications, e.g., to
      provide data on sound location and attenuation, relative locations of
      graphical models, etc.

      @param coords  Vector with coordinates
      @returns       True if successful
  */
  bool addCoordinates(const std::vector<double> &coords);

  /** Retrieve a specification for a graphical object, previously added by
      addFactorySpec

      This tries to find a specification from the map of
      specifications. There is a specific case for strict:

      - if strict==true, the match is exact, with label as the key. This
        is typically done when creating (static) objects through the
        start script.

      The match is successively tried against the following

      - class with label combination, match against value of dclass:label,
        as an example, "BaseObjectMotion:737KLM", could be against a
        specification with files giving 737 geometry in KLM livery.

      - the same as above, class with label, is repeated for all the base
        classes of the dataclass. Since e.g., BaseObjectMotion is based
        on BaseObjectPosition, that will be tried next if the previous
        match did not succeed.

      - just the class, match against dclass, as an example
        "SpecificVehicleMotion". In this case the name of the specification
        will be set to the label.

      - afther that all parent classes of the dclass are tried.

      - if all fails, a specification is generated with label as name, and
        dclass as type. If, in this case, the label/name does not have a
        # suffix, this will be added (see below for the effect)

      There are a few special processing cases for the name in the spec:

      - if there is a vertical bar in label, the match is against the
        first part of label only. The information after the vertical bar
        is then used to modify the name field in the spec. So with label
        "BaseObjectMotion:737KLM|PH-AXY", the match will be as in the above
        example, the name element of the return specification will be replaced
        with "PH-AXY". Otherwise the name is from the specification.

      - If the name from a found dataclass ends with a '#' character,
        (either because this was given in the specification, or
        because it was given in the label name, or because it was
        added after not finding a match) the creation id from the
        channel is added to the object name (for automatic naming). So
        the name may be "B737 #", that will result in "B737 #1", "B737
        #5", etc. Note that the numbering is based on the entries in
        the channel, and entries in-between might match other
        SpecificationBase specifications.

     @param dclass  Class name of the DCO type to match.
     @param label   Label of the entry.
     @param cid     Channel entry id.
     @param strict  Strict matching, i.e. against label only.
  */
  WorldDataSpec retrieveFactorySpec(const std::string &dclass,
                                    const std::string &label, unsigned cid = 0U,
                                    bool strict = false);

  /** Check that the database has a specific specification */
  bool hasFactorySpec(const std::string &match);

public:
  /** Constructor */
  SpecificationBase();

  /** Destructor */
  ~SpecificationBase();
};

#endif
