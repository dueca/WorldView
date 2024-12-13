/* ------------------------------------------------------------------   */
/*      item            : VSGPBRShaderSet.hxx
        made by         : Rene van Paassen
        date            : 230802
        category        : header file
        description     :
        changes         : 230802 first version
        language        : C++
        copyright       : (c) 23 TUDelft-AE-C&S
*/

#pragma once

#include <vsg/utils/ShaderSet.h>
#include <glm/glm.hpp>

namespace vsgviewer {

  /** Object to pass simple fog data to shaders */
  struct Fog {
    /// Color of the fog, RGBA
    vsg::vec3 color = {1.0, 1.0, 1.0};

    /// Density, determines depth.
    float density = 0.05;

    /// or start/end
    float start = 0.0;

    ///
    float end = 1.0;

    /// exponent
    float exponent = 1.0;


    void read(vsg::Input& input)
    {
            input.read("color", color);
            input.read("density", density);
            input.read("start", start);
            input.read("end", end);
            input.read("exponent", exponent);
        }

        void write(vsg::Output& output) const
        {
            output.write("color", color);
            output.write("density", density);
            output.write("start", start);
            output.write("end", end);
            output.write("exponent", exponent);
        }
  };

  using FogValue = vsg::Value<Fog>;

  /** load and generate set of shaders

    @param opt   Generic vsg options
    @param data  Pointer to any shared data between shaders and application
  */
  vsg::ref_ptr<vsg::ShaderSet> vsgPBRShaderSet
    (vsg::ref_ptr<const vsg::Options> opt);
} // namespace vsgviewer
