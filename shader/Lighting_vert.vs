#version 430 core
#extension GL_ARB_shading_language_include : require

#line 0
#include "/../code/shader/CommonShader.glh"
#line 1

layout(location = 0) in vec3   VertexPos;
layout(location = 1) in vec2   TextureCoords;
layout(location = 2) in vec4   MatColor;
layout(location = 3) in float  TextureIndex;

uniform mat4 ProjectionViewMatrix;

out vec2  vTextureUVs;
out vec4  vMatColor;
out float vTextureIndex;

void main()
{
    vTextureUVs   = TextureCoords;
    vMatColor     = MatColor;
    vTextureIndex = TextureIndex;

    gl_Position = ProjectionViewMatrix * vec4(VertexPos, 1.0f); 
}
