#version 430 core
#extension GL_ARB_shading_language_include : require

#line 0
#include "/../code/shader/CommonShader.glh"
#line 1

// IN FROM VERTEX SHADER
in vec2  vTextureUVs;
in vec4  vMatColor;
in real32 vTextureIndex;

// TEXTURES
layout(binding = 1) uniform sampler2D GameAtlas;
layout(binding = 2) uniform sampler2D FontAtlas;

// OUTPUT COLOR
out vec4 FragColor;

void main()
{
    // NULL TEXTURE, TEXTURELESS QUADS
    if(int(vTextureIndex) == 0)
    {
        vec4 TextureColor = vMatColor;    
        if(TextureColor.a <= 0.1)
        {
            discard;
        }
        FragColor = TextureColor;
    }

    // GAME TEXTURES
    if(int(vTextureIndex) == 1)
    {
        vec4 TextureColor = texelFetch(GameAtlas, ivec2(vTextureUVs), 0);
        if(TextureColor.a <= 0.1)
        {
            discard;    
        }
        FragColor = TextureColor * vMatColor; 
    }

    // FONT RENDERERING
    if(int(vTextureIndex) == 2)
    {
        // NOTE(Sleepster): 512 is the size of the game texture, if you change the size of the game of the texture, change this to that size 
        real32 NormalizedTextureUVX = vTextureUVs.x / 512;
        real32 NormalizedTextureUVY = vTextureUVs.y / 512;

        real32 Distance = texture(FontAtlas, vec2(NormalizedTextureUVX, NormalizedTextureUVY)).r;
        real32 SmoothEdge = 0.05;
        real32 Alpha = smoothstep(0.5 - SmoothEdge, 0.5 + SmoothEdge, Distance);
        
        FragColor = vec4(vMatColor.rgb, vMatColor.a * Alpha);
    }
}
