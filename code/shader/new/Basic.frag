#version 450 core
#extension GL_ARB_shading_language_include : require

#line 4
#include "/../code/shader/CommonShader.glh"
#line 6

#define MAX_TEXTURES 32

     in vec4    vFragColor;
     in vec2    vTexCoords;
flat in int     vTextureIndex;
flat in unsigned int  vRenderingOptions;

uniform sampler2D Textures[MAX_TEXTURES];
layout(binding = 16) uniform sampler2D FontAtlas;

out vec4 FragColor;

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
    }
    else
    {
        real32 NormalizedTextureUVX = vTexCoords.x / 600;
        real32 NormalizedTextureUVY = vTexCoords.y / 600;
        real32 Distance = texture(FontAtlas, vec2(NormalizedTextureUVX, NormalizedTextureUVY)).r;

        real32 SmoothEdge = 0.02;
        real32 Alpha = smoothstep(0.5 - SmoothEdge, 0.5 + SmoothEdge, Distance);
        
        FragColor = vec4(vFragColor.rgb, vFragColor.a * Alpha);
    }
}
