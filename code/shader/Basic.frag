#version 430 core
#extension GL_ARB_shading_language_include : require

#line 4
#include "/../code/shader/CommonShader.glh"
#line 6

layout(std430, binding = 0) buffer PointLightSBO
{
    point_light PointLights[];  
};

uniform int   PointLightCount;
uniform float uBrightness;

// IN FROM VERTEX SHADER
in vec3   vFragPos;
in vec2   vTextureUVs;
in vec3   vNormals;
in vec4   vMatColor;
in real32 vTextureIndex;

// TEXTURES
layout(binding = 0) uniform sampler2D GameAtlas;
layout(binding = 1) uniform sampler2D FontAtlas;

// OUTPUT COLOR
out vec4 FragColor;

void main()
{
    // GAME TEXTURES
    if(int(vTextureIndex) == 0)
    {
        float AmbientStrength = 1.0;
        vec3  GlobalAmbientColor = vec3(0.8);
        vec3  AmbientLight = AmbientStrength * GlobalAmbientColor;
        vec3  Diffuse = vec3(0.0);
        vec3  Norm = normalize(vNormals);

        vec3  TotalLighting = vec3(0.0);
        for(int LightIndex = 0; LightIndex < PointLightCount; ++LightIndex)
        {
            point_light PointLight = PointLights[LightIndex];

            vec3 LightDir = normalize(PointLight.Position - vFragPos);
            float LightDist = length(PointLight.Position - vFragPos);

            float Attenuation = 1.0 / (PointLight.Attenuation.Constant 
                                 + PointLight.Attenuation.Linear * LightDist 
                                 + PointLight.Attenuation.Quadratic * (LightDist * LightDist));
        // NOTE(Sleepster): For SpotLights, have the Dif be (max(dot(LightDir, Norm), 0.0)) 
            float Diff    = 1.0;
                  Diffuse = Diff * PointLight.LightColor.rgb * Attenuation * PointLight.Strength;

            TotalLighting += AmbientLight + Diffuse;
        }
        vec4 TextureColor = texelFetch(GameAtlas, ivec2(vTextureUVs), 0);
        if (TextureColor.a <= 0.1)
        {
            discard;
        }
        
        vec4 ObjectColor = vec4(TextureColor.rgb * vMatColor.rgb, vMatColor.a);
        if(TotalLighting == vec3(0.0))
        {
            FragColor = vec4((AmbientLight * ObjectColor.rgb) * uBrightness, vMatColor.a);
        }
        else
        {
            FragColor = vec4((TotalLighting), 1.0) * ObjectColor;
        }
    }

    // FONT RENDERERING
    if(int(vTextureIndex) == 1)
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
