/* ------------------------------------------------------------------   */
/*      item            : OSGLight.hxx
        made by         : Rene van Paassen
        date            : 150318
        category        : header file
        description     :
        changes         : 150318 first version
        language        : C++
*/

#ifndef OSGLight_hxx
#define OSGLight_hxx

/* Some refs

// a different project, but interesting description
http://www.opensg.org/projects/opensg/wiki/Tutorial/OpenSG2/Light#LightPositionDirectionandInfluenceArea

If you want a directional light source, use setPosition with the w component
set to 0.0. If you want a positional light, specify the x,y,z and set w = 1.0.
Don't use setDirection().
*/

#include "OSGObject.hxx"
#include "OSGStaticObject.hxx"
#include "OSGCenteredObject.hxx"

class OSGLightBase
{
public:
  /// Light type 1
  osg::Vec4d ambient;

  /// Light type 2
  osg::Vec4d diffuse;

  /// Light type 2
  osg::Vec4d specular;

  /// Light position, or directional light direction with w = 0
  osg::Vec4d position;

  /// Spot light direction
  osg::Vec3d direction;

  /// Spot exponent, sharpness of spot edge
  double spot_exponent;

  /// Spot cut-off (width of beam in deg)
  double spot_cutoff;

  /// Light attenuation -- actually none, constant value 0 (no atten) to 1
  double constant_att;

  /// Light attenuation -- linear with distance
  double linear_att;

  /// Light attenuation -- quadratic with distance
  double quadratic_att;

public:
  /** Constructor */
  template<class IT>
  OSGLightBase(IT begin, const IT& end);

  /** Initialisation */
  virtual void init(const osg::ref_ptr<osg::Group>& root,
                    osg::ref_ptr<osg::Group>& parent);
};

/** OSGLight - a light object that can move with the world

    This creates a light object. Needed parameters:
    - ambient, diffuse, specular, (3 x vec4)
    - position vec4   - both updated from simulation initial given, if 4 == 1,
    -                   positional light, if 4 == 0 directional
    - direction vec3  - optional, initial given. If non-zero, used, light is
                        spotlight
    - constant, linear, quadr attenuation
    - spot exponent
    - spot cutoff = opening angle in rad
*/
class OSGLight: public OSGObject
{
protected:
  /** Base data and interpretation */
  OSGLightBase base;

public:
  /** Constructor */
  OSGLight(const WorldDataSpec& specification);

  /** Destructor */
  ~OSGLight();

  /** Connect to a channel entry

      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  virtual void connect(const GlobalId& master_id, const NameSet& cname,
                       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect);

  /** Play, update, recalculate, etc.

      @param ts        Time specification for update
      @param base      Movement of the base object, extrapolated if applicable
      @param late      Time elapsed since ts start tick
   */
  virtual void iterate(TimeTickType ts,
                       const BaseObjectMotion& base, double late);

  /** Initialise the light */
  virtual void init(const osg::ref_ptr<osg::Group>& root, OSGViewer* master);
};


/** A light that is centered on the observer. It has an offset position, and
    possibly an offset direction.

    Parameters dictate whether it is a point light, directional light
    or a spotlight. Note that it has no use to create a directional
    light that follows the observer!
*/
class OSGCenteredLight: public OSGLight
{
public:
  /** Constructor */
  OSGCenteredLight(const WorldDataSpec& specification);

  /** Destructor */
  ~OSGCenteredLight();

  /** Connect to a channel entry

      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  void connect(const GlobalId& master_id, const NameSet& cname,
                       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect);

  /** Play, update, recalculate, etc.

      @param ts        Time specification for update
      @param base      Movement of the base object, extrapolated if applicable
      @param late      Time elapsed since ts start tick
   */
   void iterate(TimeTickType ts,
                const BaseObjectMotion& base, double late);

  /** Initialise the light */
  void init(osg::ref_ptr<osg::Group>& root);
};


/** A light that is static in the world. It has a position, and
    possibly a direction.

    Parameters dictate whether it is a point light, directional light
    or a spotlight.
*/
class OSGStaticLight: public OSGLight
{
public:
  /** Constructor */
  OSGStaticLight(const WorldDataSpec& specification);

  /** Destructor */
  ~OSGStaticLight();

  /** Connect to a channel entry

      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  void connect(const GlobalId& master_id, const NameSet& cname,
               entryid_type entry_id,
               Channel::EntryTimeAspect time_aspect);

  /** Play, update, recalculate, etc. */
  void iterate(TimeTickType ts, const BaseObjectMotion& base, double late);
};


#endif
