#version 430 core
#extension GL_ARB_shading_language_include : require

#line 4
#include "/../code/shader/CommonShader.glh"
#line 6

in vec2 vTextureUVs;
in vec3 vFragPos;
in vec3 vNormals;
in vec4 vMatColor;

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormals;
layout(location = 2) out vec4 gAlbedoSpec;

layout(binding = 0) uniform sampler2D GameAtlas;

void main()
{
    gPosition   = vFragPos;
    gNormals    = normalize(vNormals);

    vec4 TexelColor = texelFetch(GameAtlas, ivec2(TextureUVs), 0);
    if(TexelColor.a == 0.0)
    {
        discard;    
    }
    gAlbedoSpec.rgb = TexelColor.rgb * vMatColor.rgb;
    gAlbedoSpec.a   = 1.0;
}
