#version 430 core
#extension GL_ARB_shading_language_include : require

#line 4
#include "/../code/shader/CommonShader.glh"
#line 6

layout(location = 0) in vec3   VertexPos;
layout(location = 1) in vec2   TextureCoords;
layout(location = 2) in vec3   VertexNormals;
layout(location = 3) in vec4   MatColor;
layout(location = 4) in float  TextureIndex;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

out vec3  vFragPos;
out vec2  vTextureUVs;
out vec3  vNormals;
out vec4  vMatColor;
out float vTextureIndex;

void main()
{
    vTextureUVs   = TextureCoords;
    vNormals      = VertexNormals; 
    vMatColor     = MatColor;
    vTextureIndex = TextureIndex;

    vFragPos = (ProjectionMatrix * ViewMatrix * vec4(VertexPos, 1.0f)).xyz;
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(VertexPos, 1.0f); 
}
