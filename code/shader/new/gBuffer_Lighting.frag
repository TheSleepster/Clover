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
layout(binding = 1) uniform sampler2D gBufferPosition;
layout(binding = 2) uniform sampler2D gBufferNormals;

in vec2 vUV;

out vec4 FragColor;

void main()
{
    vec3 Normal      = texture(gBufferNormals,  vUV).rgb;
    vec3 AlbedoColor = texture(gBufferTexture,  vUV).rgb;
    vec3 FragPos     = texture(gBufferPosition, vUV).rgb;

    float LitFactor  = Normal.b;
    
    float AmbientStrength = (uWorldBrightness * uBrightnessFactor);
    vec3  AmbientLighting = vec3(1.0) * AmbientStrength;

    vec3 TotalLighting = vec3(0);
    for(int LightIndex = 0; LightIndex < uPointLightCount; LightIndex++)
    {
        point_light PointLight = PointLights[LightIndex];
        
        vec3  LightDir  = normalize(PointLight.WorldPosition - FragPos);
        float LightDist = length(PointLight.WorldPosition - FragPos); 
        if(LightDist > PointLight.Radius) continue;

        float Attenuation = 1.0 / (PointLight.Attenuation.Constant + PointLight.Attenuation.Linear * LightDist + PointLight.Attenuation.Quadratic * (LightDist * LightDist));
        float DistanceFactor = smoothstep(PointLight.Radius * 0.75, PointLight.Radius, LightDist);

        vec3  DiffuseLighting = AlbedoColor * PointLight.LightColor.rgb * Attenuation * PointLight.Strength * (1 - DistanceFactor);
        TotalLighting        += DiffuseLighting;
    }
    
    if(LitFactor == 0)
    {
        FragColor = vec4(AlbedoColor, 1.0);
    }
    else
    {
        FragColor = vec4(AlbedoColor * (AmbientLighting * LitFactor), 1.0) + vec4((TotalLighting * LitFactor), 1.0);
    }
}
