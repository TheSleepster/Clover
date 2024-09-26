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
        float NormalizedTextureUVX = vTextureUVs.x / 512;
        float NormalizedTextureUVY = vTextureUVs.y / 512;

        float Distance = texture(FontAtlas, vec2(NormalizedTextureUVX, NormalizedTextureUVY)).r;
        float SmoothEdge = 0.05;
        float Alpha = smoothstep(0.5 - SmoothEdge, 0.5 + SmoothEdge, Distance);
        
        FragColor = vec4(vMatColor.rgb, vMatColor.a * Alpha);
    }
}
