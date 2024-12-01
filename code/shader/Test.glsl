#VERTEX
#version 450 core

layout(location = 0) in vec4 Position;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec2 TexCoords;
layout(location = 3) in int  TextureIndex;

	 out vec4 vFragColor;
	 out vec2 vTexCoords;
flat out int  vTextureIndex;

void main()
{
    vFragColor  = Color;
    vTexCoords = TexCoords;
    vTextureIndex = TextureIndex;
    
	gl_Position = Position;
}

#FRAGMENT
#version 450 core

#define MAX_TEXTURES 32

	 in vec4 vFragColor;
	 in vec2 vTexCoords;
flat in int  vTextureIndex;

uniform sampler2D Textures[MAX_TEXTURES];

out vec4 FragColor;
void main()
{
    if(int(vTextureIndex) > -1)
    {
		vec4 TextureColor = texture(Textures[vTextureIndex], vTexCoords);
        if(TextureColor.a == 0)
        {
            discard;
        }
        FragColor = vFragColor * TextureColor;
    }
    else
    {
        FragColor = vFragColor;
    }
}
