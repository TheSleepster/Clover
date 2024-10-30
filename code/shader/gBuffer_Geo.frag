#version 430 core
#extension GL_ARB_shading_language_include : require

#line 4
#include "/../code/shader/CommonShader.glh"
#line 6

layout(std430, binding = 0) buffer gBufferPointLightSBO
{
    point_light PointLights[];    
};

uniform float uBrightness;
uniform int   uPointLightCount;

layout(location = 0) in vec3  vFragPos;
layout(location = 1) in vec2  vTextureUVs;
layout(location = 2) in vec3  vNormals;
layout(location = 3) in vec4  vMatColor;
layout(location = 4) in float vTextureIndex;


layout(location = 0) out vec4 gColor;
layout(location = 1) out vec4 gNormals;
layout(location = 2) out vec4 gPosition;

layout(binding = 0) uniform sampler2D GameAtlas;
layout(binding = 1) uniform sampler2D FontAtlas;

void main()
{
    gNormals  = vec4(vNormals * 0.5 + 0.5, 1.0);
    gPosition = vec4(vFragPos.rg, 0, 1.0); 

    vec4 TexelColor = texelFetch(GameAtlas, ivec2(vTextureUVs), 0);
    if (TexelColor.a == 0.0)
    {
        discard;
    }

    gColor.rgb = TexelColor.rgb * vMatColor.rgb;
    gColor.a = 1.0;
}
