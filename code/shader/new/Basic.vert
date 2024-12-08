#version 450 core

layout(location = 0) in vec4 Position;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec2 TexCoords;
layout(location = 3) in int  TextureIndex;
layout(location = 4) in unsigned int RenderingOptions; 

	 out vec4    vFragColor;
	 out vec2    vTexCoords;
flat out int     vTextureIndex;
flat out unsigned int  vRenderingOptions;

void main()
{
    vFragColor  = Color;
    vTexCoords = TexCoords;
    vTextureIndex = TextureIndex;
    vRenderingOptions = RenderingOptions;
    
	gl_Position = Position;
}
