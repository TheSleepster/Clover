#version 450 core

layout(location = 0) in vec4 NDCPosition;
layout(location = 1) in vec4 WorldPosition;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 TexCoords;
layout(location = 4) in vec3 VertexNormals;
layout(location = 5) in int  TextureIndex;
layout(location = 6) in unsigned int RenderingOptions; 
layout(location = 7) in float LitFactor; 

	 out vec4    vFragColor;
	 out vec2    vTexCoords;
     out float   vLitFactor;
flat out int     vTextureIndex;
flat out unsigned int  vRenderingOptions;

void main()
{
    vFragColor        = Color;
    vTexCoords        = TexCoords;
    vLitFactor        = LitFactor;
    vTextureIndex     = TextureIndex;
    vRenderingOptions = RenderingOptions;
    
	gl_Position = NDCPosition;
}
