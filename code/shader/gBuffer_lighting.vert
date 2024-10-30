#version 430 core
#extension GL_ARB_shading_language_include : require

#line 4
#include "/../code/shader/CommonShader.glh"
#line 6

out vec2 vUV;

void main()
{
    vec2 Vertices[6] = 
    {
        vec2(-1,  1), // Top-left
        vec2( 1,  1), // Top-right
        vec2(-1, -1), // Bottom-left
        vec2(-1, -1), // Bottom-left
        vec2( 1, -1), // Bottom-right
        vec2( 1,  1), // Top-right
    };

    vec2 TexCoords[6] = 
    {
        vec2(0.0, 1.0), // Top-left
        vec2(1.0, 1.0), // Top-right
        vec2(0.0, 0.0), // Bottom-left
        vec2(0.0, 0.0), // Bottom-left
        vec2(1.0, 0.0), // Bottom-right
        vec2(1.0, 1.0), // Top-right
    };

    gl_Position = vec4(Vertices[gl_VertexID], 0.0, 1.0);
    vUV = TexCoords[gl_VertexID];
}
