#version 430 core

in  vec2 vUV;
out vec4 FragColor;

layout(binding = 0) uniform sampler2D Image;

void main()
{
    vec4 ImageColor = texture(Image, vUV); 
    FragColor = vec4(ImageColor.rgb, 1.0);
}
