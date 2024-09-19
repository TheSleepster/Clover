// IN FROM VERTEX SHADER
in vec2  vTextureUVs;
in vec4  vMatColor;
in float vTextureIndex;

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
        vec4 FontColor = texelFetch(FontAtlas, ivec2(vTextureUVs), 0);
        if(FontColor.r == 0)
        {
            discard;
        }
        FragColor = FontColor.r * vMatColor;
    }
}
