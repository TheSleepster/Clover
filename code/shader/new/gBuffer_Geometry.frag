#version 430 core
#extension GL_ARB_shading_language_include : require

#line 4
#include "/../code/shader/CommonShader.glh"
#line 6

     in vec4  vFragColor;
     in vec4  vWorldPosition;
     in vec2  vTexCoords;
     in vec3  vNormals;
     in float vLitFactor;
flat in int   vTextureIndex;
flat in unsigned int vRenderingOptions;

uniform sampler2D Textures[MAX_TEXTURES];
layout(binding = 16) uniform sampler2D FontAtlas;

out vec4 FragColor;
out vec4 WorldPositionColor;
out vec4 VertexNormalsColor;

void main()
{
    ivec2 TexCoords = ivec2(vTexCoords);
    
    bool Textured = vTextureIndex > -1;
    bool Font     = ((vRenderingOptions & RENDERING_OPTION_FONT) != 0);

    vec4 TextureColor;
    if(!Font)
    {
        TextureColor = Textured ? texelFetch(Textures[vTextureIndex], TexCoords, 0) : vec4(1.0);
        if(TextureColor.a == 0) discard;
        FragColor = TextureColor * vFragColor;
        WorldPositionColor = vec4(vWorldPosition.rg, 0.0, 1.0);
        VertexNormalsColor = vec4(vNormals.xy, vLitFactor, 1.0);
    }
    else
    {
        real32 NormalizedTextureUVX = vTexCoords.x / CLOVER_FONT_ATLAS_SIZE;
        real32 NormalizedTextureUVY = vTexCoords.y / CLOVER_FONT_ATLAS_SIZE;
        real32 Distance = texture(FontAtlas, vec2(NormalizedTextureUVX, NormalizedTextureUVY)).r;

        real32 SmoothEdge = 0.02;
        real32 Alpha = smoothstep(0.5 - SmoothEdge, 0.5 + SmoothEdge, Distance);
        if(Alpha == 0.0) discard;
        
        FragColor = vec4(vFragColor.rgb, vFragColor.a * Alpha);
        WorldPositionColor = vec4(vWorldPosition.rg, 0.0, 1.0);
        VertexNormalsColor = vec4(vNormals.xy, vLitFactor, 1.0);
    }
}

