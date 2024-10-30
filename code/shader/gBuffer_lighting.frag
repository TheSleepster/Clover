#version 430 core
#extension GL_ARB_shading_language_include : require

#line 4
#include "/../code/shader/CommonShader.glh"
#line 6

layout(std430, binding = 0) buffer gBufferPointLightSBO
{
    point_light PointLights[];    
};

uniform float uBrightnessFactor;
uniform float uWorldBrightness;
uniform int   uPointLightCount;

layout(binding = 0) uniform sampler2D gBufferTexture;
layout(binding = 1) uniform sampler2D gBufferNormals;
layout(binding = 2) uniform sampler2D gBufferPosition;

out vec4 FragColor;

in vec2 vUV;

void main()
{
    vec3 Normal      = texture(gBufferNormals,  vUV).rgb;
    vec3 AlbedoColor = texture(gBufferTexture,  vUV).rgb;
    vec3 FragPos     = texture(gBufferPosition, vUV).rgb;
    
    float AmbientStrength = (uWorldBrightness * uBrightnessFactor);
    vec3  AmbientLighting = vec3(1.0) * AmbientStrength;

    vec3 TotalLighting = vec3(0);
    for(int LightIndex = 0; LightIndex < uPointLightCount; LightIndex++)
    {
        point_light PointLight = PointLights[LightIndex];
        
        vec3  LightDir  = normalize(PointLight.Position - FragPos);
        float LightDist = length(PointLight.Position - FragPos); 
        if(LightDist > PointLight.Radius) continue;

        float Attenuation = 1.0 / (PointLight.Attenuation.Constant + PointLight.Attenuation.Linear * LightDist + PointLight.Attenuation.Quadratic * (LightDist * LightDist));
        float DistanceFactor = smoothstep(PointLight.Radius * 0.5, PointLight.Radius, LightDist);

        vec3  DiffuseLighting = AlbedoColor * PointLight.LightColor.rgb * Attenuation * PointLight.Strength * (1 - DistanceFactor);
        TotalLighting        += DiffuseLighting;
    }
    
    FragColor = vec4(AlbedoColor * AmbientLighting, 1.0) + vec4(TotalLighting, 1.0);
}
