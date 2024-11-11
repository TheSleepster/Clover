Clover Engine
=============
This is an entire game and GameEngine Written using minimal external libraries.   
It operates **ONLY ON THE WIN32** as that is the only made platform layer at the moment.  

Capabitilies (Brief)
-------------------
The Engine is currently capable of
- Rendering Text using it's own font solution (Freetype)
- Rendering Images and parsing them into their own subsprites
- Performing batch rendering operation and deferred rendering using OpenGL
- Multithreaded job handling
- Conroller Support using XInput

Examples
-------
### General & Transparency Testing:
![Demo](https://github.com/user-attachments/assets/398e1c9b-a85f-461e-b0b7-f6d5187beea5)

### Lighting:
![example2](https://github.com/user-attachments/assets/6e365c61-f297-488a-8bc0-de5d62ae7e06)

### Batch Rendering:
![image](https://github.com/user-attachments/assets/7f5e6b08-291f-4a80-ae21-ed446a1b7a14)

### Deferred Rendering:
#### In the below image we render all of the quads onto a series of textures  
- Albedo (color) Texture
- Normal Texture (Vertex Normals)
- World Position Texture
- Depth Buffer

![example3](https://github.com/user-attachments/assets/cce76cd1-c6cd-47c4-b082-3637a836b4d2)
#### Then in this image we use the lighting shader to mix the two together:
```GLSL
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
```
#### Leading to This
![example4](https://github.com/user-attachments/assets/85a15899-964c-47af-ba66-69000a1fa9e7)
This allows us to manage complex lighting and GPU particles.

TODO
------
> [!NOTE]
> This is just a small list of what still needs to be added to the engine
> There's still more

Asset System
---------------
- [ ] Create a system for tagging assets. 
- [ ] Make Tags will include things like what KIND of asset it is 
- [ ] Create a struct to control what assets get streamed in and out the game
- [ ] Asset file format that will store things like what assets are needed on a per area basis for asset streaming

> [!NOTE]
> This is an example of what that could look like
```C++
#include <stdio.h> 
typedef int int32;

#define internal static

enum 
{
    TAG_Music,
    TAG_Texture,
    TAG_SFX,
};

struct asset_tag
{
    int32 ID; 
    int32 TagType;
};

struct game_asset
{
    asset_tag TagData;
};

internal game_asset
LoadAsset(string Filepath, int32 TagType)
{
    game_asset Result = ;
    // stuff
    return(Result);
}
```

Sound Mixer
--------------
- [ ] WASAPI??? XAudio2???
- [ ] Control how audio is mixed
- [ ] Control and determine how a sound is playing
    - [ ] Is the sound looped?
    - [ ] Is the sound multi-instancing? (play more than one of the same sound at a time from the same source)
    - [ ] Is the second of code calling to play the sound already playing it?
    - [ ] Is there a cap on the maximum number of sounds we can mix at a given time?
- [ ] Allow for audio streaming on large files (music)
- [ ] Allow for control on things like pitch 
- [ ] Create an API for allowing the game to modify sounds in response to certain events

> [!NOTE]
> This is an example of what that could look like
``` C++
if(ThingHappens)
{
    ShiftPitch(-0.4f);
}
```

Sound API
----------------
- [ ] Provide a way for the engine to play a sound, probably by creating a pair between a string and a SoundID that's loaded
- [ ] With the Asset System, handle the sounds

Enhance Arenas 
-------------
- [x] Transient State
- [x] Better method of storing the current Base position for the next arena
- [x] Storage for Transient Memory Arenas

Multithreading (if needed)
-------------------
- [x] Job system
- [ ] Multithreaded asset loading
- [ ] Multithreaded rendering
- [ ] Multithreaded game code execution

Preprocessor for introspection of assets and other such runtime information 
---------------------------------------------------------------------------
- [ ] Define a macro with which to designate items to parse
- [ ] Parser for structs that spits out the data they contain and the data type
- [ ] Have it generate the offset of the type and it's name to a file
- [ ] Generate MetaType information about what the object is and spit it into an enum (automatically)
- [ ] Perhaps spit it into the asset file?

Change how our hotreloading works
---------------------------------
- [ ] Looped Live code editing
- [ ] Don't reset EVERYTHING in the game on reload. (Ex: Don't regenerate the world and set the player to 0)
- [ ] Store a persistant world state that doesn't change on reloading
- [ ] Make testing easier

GPU Particles 
-------------
- [ ] Render the particles
- [ ] Create a particle system
- [ ] Allow the system to control the kind of particles and the rate with which they are output
- [ ] Allow images to be used on particles
