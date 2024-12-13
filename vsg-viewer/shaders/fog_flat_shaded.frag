#version 450
#extension GL_ARB_separate_shader_objects : enable
#pragma import_defines (VSG_POINT_SPRITE, VSG_DIFFUSE_MAP, VSG_GREYSCALE_DIFFUSE_MAP, WORLDVIEW_SIMPLEFOG)

#define VIEW_DESCRIPTOR_SET 0
#define MATERIAL_DESCRIPTOR_SET 1

#ifdef VSG_DIFFUSE_MAP
layout(set = MATERIAL_DESCRIPTOR_SET, binding = 0) uniform sampler2D diffuseMap;
#endif

layout(set = MATERIAL_DESCRIPTOR_SET, binding = 10) uniform MaterialData
{
    vec4 ambientColor;
    vec4 diffuseColor;
    vec4 specularColor;
    vec4 emissiveColor;
    float shininess;
    float alphaMask;
    float alphaMaskCutoff;
} material;

layout(location = 2) in vec4 vertexColor;
#ifndef VSG_POINT_SPRITE
layout(location = 3) in vec2 texCoord0;
#endif

#ifdef WORLDVIEW_SIMPLEFOG
// simple fog code
// https://stackoverflow.com/questions/22554631/accessing-the-depth-buffer-from-a-fragment-shader
layout(set=VIEW_DESCRIPTOR_SET, binding = 11) uniform FogData
{
   vec3 color;
   float density;
} wv_Fog;
in vec4 gl_FragCoord;
#endif

#endif

layout(location = 0) out vec4 outColor;

void main()
{
#ifdef VSG_POINT_SPRITE
    vec2 texCoord0 = gl_PointCoord.xy;
#endif

    vec4 diffuseColor = vertexColor * material.diffuseColor;

#ifdef VSG_DIFFUSE_MAP
    #ifdef VSG_GREYSCALE_DIFFUSE_MAP
        float v = texture(diffuseMap, texCoord0.st).s;
        diffuseColor *= vec4(v, v, v, 1.0);
    #else
        diffuseColor *= texture(diffuseMap, texCoord0.st);
    #endif
#endif

    if (material.alphaMask == 1.0f)
    {
        if (diffuseColor.a < material.alphaMaskCutoff)
            discard;
    }

#ifdef WORLDVIEW_SIMPLEFOG
    // simple fog code
    const float LOG2 = 1.442695;
    float zd = wv_Fog.density * gl_FragCoord.z / gl_FragCoord.w;
    float fogFactor = exp2( -zd * zd * LOG2 );
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    outColor.rgb = mix(wv_Fog.color, diffuseColor.rgb, fogFactor);
    outColor.a = diffuseColor.a;
#else
    outColor = diffuseColor;
#endif

//    outColor = diffuseColor;
}
