#version 430 core
#extension GL_ARB_shading_language_include : require

#line 4
#include "/../code/shader/CommonShader.glh"
#line 6

uniform float uBrightness;

in vec2 vTextureUVs;
in vec3 vFragPos;
in vec3 vNormals;
in vec4 vMatColor;

layout(location = 0) out vec4 gColor;
layout(location = 1) out vec3 gNormals;

layout(binding = 0) uniform sampler2D GameAtlas;
layout(binding = 1) uniform sampler2D NormalAtlas;

void main()
{
    vec4 TexelColor = texelFetch(GameAtlas, ivec2(vTextureUVs), 0);
    if(TexelColor.a == 0.0)
    {
        discard;    
    }

    gColor.rgb  = TexelColor.rgb * vMatColor.rgb;
    gColor.rgb *= uBrightness;
    gColor.a    = 1.0;

    vec3 NormalizedNormals = normalize(vNormals);
    gNormals = NormalizedNormals * 0.5 + 0.5;
}
