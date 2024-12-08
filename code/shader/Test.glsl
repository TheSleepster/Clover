#shader VERTEX
#version 450 core

layout(location = 0) in vec4 Position;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec2 TexCoords;
layout(location = 3) in int  TextureIndex;
layout(location = 4) in unsigned int RenderingOptions; 

	 out vec4 vFragColor;
	 out vec2 vTexCoords;
flat out int  vTextureIndex;
flat out int  vRenderingOptions;

void main()
{
    vFragColor  = Color;
    vTexCoords = TexCoords;
    vTextureIndex = TextureIndex;
    
	gl_Position = Position;
}

#shader FRAGMENT
#version 450 core

#define MAX_TEXTURES 32

	 in vec4 vFragColor;
	 in vec2 vTexCoords;
flat in int  vTextureIndex;
flat in int  vRenderingOptions;

uniform sampler2D Textures[MAX_TEXTURES];

out vec4 FragColor;
void main()
{
    if(int(vTextureIndex) > -1)
    {
		vec4 TextureColor = texelFetch(Textures[vTextureIndex], ivec2(vTexCoords), 0);
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
