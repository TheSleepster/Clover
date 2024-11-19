> [!NOTE]
> This is just a small list of what still needs to be added to the engine
> There's still more


- So funny little HOO HA. In the game, when an entity is picked up, and then dropped again,
  It increments the entity counter TWICE


- WASAPI??? XAudio2???
- # Sound Mixer
    - [x] Control how audio is mixed
    - [ ] Control and determine how a sound is playing
        - [ ] Is the sound looped?
        - [ ] Is the sound multi-instancing? (play more than one of the same sound at a time from the same source)
        - [ ] Is the second of code calling to play the sound already playing it?
        - [ ] Is there a cap on the maximum number of sounds we can mix at a given time?
    - [ ] Allow for audio streaming on large files (music)
    - [ ] Allow for control on things like pitch 
    - [ ] Create an API for allowing the game to modify sounds in response to certain events

    > [!NOTE]
    > THIS IS VERY IMPORTANT
    - [ ] Add a "trigger" object that can begin a sound and prevents multiple instances from being played at once from the same source

    > [!NOTE]
    > This is an example of what that could look like
    ``` C++
    if(ThingHappens)
    {
        ShiftPitch(-0.4f);
    }
        ```
---
- ## Asset System
    - [ ] Create a system for tagging assets. 
    - [x] Make Tags will include things like what KIND of asset it is 
    - [x] Create a struct to control what assets get streamed in and out the game
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
---
- # Sound API
    - [x] Provide a way for the engine to play a sound, probably by creating a pair between a string and a SoundID that's loaded
    - [x] With the Asset System, handle the sounds

---
- ### Enhance Arenas 
    - [x] Transient State
    - [x] Better method of storing the current Base position for the next arena
    - [x] Storage for Transient Memory Arenas

---
- #### Multithreading (if needed)
    - [x] Job system
    - [x] Multithreaded asset loading
    - [ ] Multithreaded rendering
    - [ ] Multithreaded game code execution

---
- ##### Preprocessor for introspection of assets and other such runtime information 
    - [ ] Define a macro with which to designate items to parse
    - [ ] Parser for structs that spits out the data they contain and the data type
    - [ ] Have it generate the offset of the type and it's name to a file
    - [ ] Generate MetaType information about what the object is and spit it into an enum (automatically)
    - [ ] Perhaps spit it into the asset file?

---

---
- ###### Change how our hotreloading works
    - [ ] Looped Live code editing
    - [ ] Don't reset EVERYTHING in the game on reload. (Ex: Don't regenerate the world and set the player to 0)
    - [ ] Store a persistant world state that doesn't change on reloading
    - [ ] Make testing easier

---
- ###### GPU Particles 
    - [ ] Render the particles
    - [ ] Create a particle system
    - [ ] Allow the system to control the kind of particles and the rate with which they are output
    - [ ] Allow images to be used on particles
