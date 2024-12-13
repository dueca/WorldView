/* ------------------------------------------------------------------   */
/*      item            : VSGLight.hxx
        made by         : Rene van Paassen
        date            : 230125
        category        : header file
        description     :
        changes         : 230125 first version
        language        : C++
        copyright       : (c) 23 TUDelft-AE-C&S
*/

#pragma once
#include "VSGObject.hxx"

namespace vsgviewer {

/** Ambient lighting definition */
class VSGAmbientLight: public VSGObject
{
  /** Light color */
  vsg::vec3 color;

  /** Overall intensity */
  float     intensity;

  /** VSG node */
  vsg::ref_ptr<vsg::AmbientLight> light;

public:
  /** Constructor */
  VSGAmbientLight(const WorldDataSpec& data);

  /** Destructor */
  ~VSGAmbientLight();

  /** Initialise the light with the VSG scene */
  virtual void init(const vsg::ref_ptr<vsg::Group>& root,
                    VSGViewer* master) override;
};

class VSGDirectionalLight: public VSGObject
{
  /** Light color */
  vsg::vec3 color;

  /** Overall intensity */
  float     intensity;

  /** Light direction */
  vsg::vec3 direction;

  /** VSG nodes */
  vsg::ref_ptr<vsg::DirectionalLight> light;
public:
  /** Constructor */
  VSGDirectionalLight(const WorldDataSpec& data);

  /** Destructor */
  ~VSGDirectionalLight();

  /** Initialise the light with the VSG scene */
  virtual void init(const vsg::ref_ptr<vsg::Group>& root,
                    VSGViewer* master) override;
};

class VSGPointLight: public VSGObject
{
  /** Light color */
  vsg::vec3 color;

  /** Overall intensity */
  float     intensity;

  /** Light position */
  vsg::vec3 position;

  /** Range of the light */
  float     span;

  /** VSG nodes */
  vsg::ref_ptr<vsg::PointLight> light;

  /** Cull group, to limit to span */
  vsg::ref_ptr<vsg::CullGroup>  cull;

public:
  /** Constructor */
  VSGPointLight(const WorldDataSpec& data);

  /** Destructor */
  ~VSGPointLight();

  /** Initialise the light with the VSG scene */
  virtual void init(const vsg::ref_ptr<vsg::Group>& root,
                    VSGViewer* master) override;
};

class VSGSpotLight: public VSGObject
{
  /** Light color */
  vsg::vec3 color;

  /** Overall intensity */
  float     intensity;

  /** Light position */
  vsg::vec3 position;

  /** Light direction */
  vsg::vec3 direction;

  /** Cone angles */
  float     innerangle;

  /** Cone angles */
  float     outerangle;

  /** Range of the light */
  float     span;

  /** VSG nodes */
  vsg::ref_ptr<vsg::SpotLight> light;

  /** Cull group, to limit to span */
  vsg::ref_ptr<vsg::CullGroup>  cull;

public:
  /** Constructor */
  VSGSpotLight(const WorldDataSpec& data);

  /** Destructor */
  ~VSGSpotLight();

  /** Initialise the light with the VSG scene */
  virtual void init(const vsg::ref_ptr<vsg::Group>& root,
                    VSGViewer* master) override;
};

}; // namespace
