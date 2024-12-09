#version 430 core
#extension GL_ARB_shading_language_include : require

#line 4
#include "/../code/shader/CommonShader.glh"
#line 6

layout(location = 0) in vec4 NDCPosition;
layout(location = 1) in vec4 WorldPosition;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 TexCoords;
layout(location = 4) in vec3 Normals;
layout(location = 5) in int  TextureIndex;
layout(location = 6) in unsigned int RenderingOptions; 
layout(location = 7) in float LitFactor;

     out vec4  vFragColor;
     out vec4  vWorldPosition;
     out vec2  vTexCoords;
     out vec3  vNormals;
flat out int   vTextureIndex;
flat out unsigned int vRenderingOptions;
     out float vLitFactor;

void main()
{
    vFragColor = Color;
    vWorldPosition = WorldPosition;
    vTexCoords = TexCoords;
    vNormals   = Normals;
    vTextureIndex = TextureIndex;
    vRenderingOptions = RenderingOptions;
    vLitFactor = LitFactor;

    // NOTE(Sleepster): This assumes that the positions you pass are already in NDC
    gl_Position = NDCPosition;
}


