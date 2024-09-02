// IN FROM VERTEX SHADER
in vec2  vTextureUVs;
in vec4  vMatColor;
in float vTextureIndex;

// TEXTURES
layout(binding = 0) uniform sampler2D GameAtlas;
layout(binding = 1) uniform sampler2D FontAtlas;

// OUTPUT COLOR
out vec4 FragColor;

void main()
{
    if(int(vTextureIndex) == 1)
    {
        vec4 FontColor = texelFetch(FontAtlas, ivec2(vTextureUVs), 0);
        if(FontColor.r == 0)
        {
            discard;    
        }
        FragColor = FontColor * vMatColor;
    }
    
    if(int(vTextureIndex) == 0)
    {
        vec4 TextureColor = texelFetch(GameAtlas, ivec2(vTextureUVs), 0);
        if(TextureColor.a == 0.0)
        {
            discard;    
        }
        FragColor = TextureColor * vMatColor; 
    }
}
