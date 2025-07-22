/* ------------------------------------------------------------------   */
/*      item            : VSGPBRShaderSet.cxx
        made by         : Rene' van Paassen
        date            : 230802
        category        : body file
        description     :
        changes         : 230802 first version
        language        : C++
        copyright       : (c) 23 TUDelft-AE-C&S
*/

#include "VSGPBRShaderSet.hxx"
#include <vsg/core/Array.h>
#include <vsg/core/Data.h>
#include <vsg/io/read.h>
#include <dueca/debug.h>
#include <vsg/state/material.h>


#define VIEW_DESCRIPTOR_SET 1
#define MATERIAL_DESCRIPTOR_SET 2
#define CUSTOM_DESCRIPTOR_SET 0

#if VSG_API_VERSION_LESS(1, 1, 11)
namespace vsg {
typedef PositionAndDisplacementMapArrayState TranslationAndDisplacementMapArrayState;
typedef PositionArrayState TranslationArrayState;
}
#endif

namespace vsgviewer {

  vsg::ref_ptr<vsg::ShaderSet> vsgPBRShaderSet
  (vsg::ref_ptr<const vsg::Options> options, vsg::ref_ptr<FogValue> the_fog)
  {
    auto pbr_vertexShader = vsg::read_cast<vsg::ShaderStage>
      ("shaders/fog_pbr.vert", options);
    auto pbr_fragmentShader = vsg::read_cast<vsg::ShaderStage>
      ("shaders/fog_pbr.frag", options);

    if (!pbr_vertexShader || !pbr_fragmentShader) {
        E_MOD("Could not load shaders.");
        return {};
    }

    auto pbr = vsg::ShaderSet::create(
      vsg::ShaderStages{pbr_vertexShader, pbr_fragmentShader});

    pbr->defaultShaderHints = vsg::ShaderCompileSettings::create();
    pbr->defaultShaderHints->generateDebugInfo = true;

    // bindings?
    pbr->addAttributeBinding
      ("vsg_Vertex", "", 0,
       VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    pbr->addAttributeBinding
      ("vsg_Normal", "", 1,
       VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    pbr->addAttributeBinding
      ("vsg_TexCoord0", "", 2,
       VK_FORMAT_R32G32_SFLOAT, vsg::vec2Array::create(1));
    pbr->addAttributeBinding
      ("vsg_Color", "", 3,
       VK_FORMAT_R32G32B32A32_SFLOAT, vsg::vec4Array::create(1));

    pbr->addAttributeBinding
      ("vsg_position", "VSG_INSTANCE_POSITIONS", 4,
       VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    pbr->addAttributeBinding
      ("vsg_position_scaleDistance", "VSG_BILLBOARD", 4,
       VK_FORMAT_R32G32B32A32_SFLOAT, vsg::vec4Array::create(1));

    pbr->addDescriptorBinding
      ("displacementMap", "VSG_DISPLACEMENT_MAP", MATERIAL_DESCRIPTOR_SET, 6,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_VERTEX_BIT,
       vsg::floatArray2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32_SFLOAT}));
    pbr->addDescriptorBinding
      ("diffuseMap", "VSG_DIFFUSE_MAP", MATERIAL_DESCRIPTOR_SET, 0,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::ubvec4Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R8G8B8A8_UNORM}));
    pbr->addDescriptorBinding
      ("mrMap", "VSG_METALLROUGHNESS_MAP", MATERIAL_DESCRIPTOR_SET, 1,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::vec2Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32G32_SFLOAT}));
    pbr->addDescriptorBinding
      ("normalMap", "VSG_NORMAL_MAP", MATERIAL_DESCRIPTOR_SET, 2,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::vec3Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32G32B32_SFLOAT}));
    pbr->addDescriptorBinding
      ("aoMap", "VSG_LIGHTMAP_MAP", MATERIAL_DESCRIPTOR_SET, 3,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::floatArray2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32_SFLOAT}));
    pbr->addDescriptorBinding
      ("emissiveMap", "VSG_EMISSIVE_MAP", MATERIAL_DESCRIPTOR_SET, 4,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::ubvec4Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R8G8B8A8_UNORM}));
    pbr->addDescriptorBinding
      ("specularMap", "VSG_SPECULAR_MAP", MATERIAL_DESCRIPTOR_SET, 5,
       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT,
       vsg::ubvec4Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R8G8B8A8_UNORM}));
    pbr->addDescriptorBinding
      ("material", "", MATERIAL_DESCRIPTOR_SET, 10,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
      vsg::PbrMaterialValue::create());
    pbr->addDescriptorBinding
      ("lightData", "", VIEW_DESCRIPTOR_SET, 0,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
      VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array::create(64));
    pbr->addDescriptorBinding
      ("viewportData", "", VIEW_DESCRIPTOR_SET,
      1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      vsg::vec4Value::create(0,0, 1280, 1024));
    pbr->addDescriptorBinding
      ("shadowMaps", "", VIEW_DESCRIPTOR_SET, 2,
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
      vsg::floatArray3D::create(1, 1, 1, vsg::Data::Properties{VK_FORMAT_R32_SFLOAT}));

    pbr->addDescriptorBinding
      ("Fog", "", CUSTOM_DESCRIPTOR_SET, 0,
       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
       the_fog);

    // additional defines
    pbr->optionalDefines =
      {"VSG_GREYSACLE_DIFFUSE_MAP", "VSG_TWO_SIDED_LIGHTING", "VSG_WORKFLOW_SPECGLOSS"};

    pbr->addPushConstantRange
      ("pc", "", VK_SHADER_STAGE_VERTEX_BIT, 0, 128);

    pbr->definesArrayStates.push_back
      (vsg::DefinesArrayState{{"VSG_INSTANCE_POSITIONS", "VSG_DISPLACEMENT_MAP"},
                                vsg::TranslationAndDisplacementMapArrayState::create()});
    pbr->definesArrayStates.push_back
      (vsg::DefinesArrayState{{"VSG_INSTANCE_POSITIONS"},
                                vsg::TranslationArrayState::create()});
    pbr->definesArrayStates.push_back
      (vsg::DefinesArrayState{{"VSG_DISPLACEMENT_MAP"},
       vsg::DisplacementMapArrayState::create()});
    pbr->definesArrayStates.push_back
      (vsg::DefinesArrayState{{"VSG_BILLBOARD"},
       vsg::BillboardArrayState::create()});

    pbr->customDescriptorSetBindings.push_back(vsg::ViewDependentStateBinding::create(VIEW_DESCRIPTOR_SET));

    return pbr;
  }


} // namespace vsgviewer
