#if !defined(CLOVER_AUDIOENGINE_H)
/* ========================================================================
   $File: Clover_AudioEngine.h $
   $Date: October 31 2024 12:42 pm $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define CLOVER_AUDIOENGINE_H

#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/String.h"
#include "util/Pairs.h"

#include "Intrinsics.h"

// NOTE(Sleepster): As of right now SDL is being used ONLY for the audio engine.
#include "../data/deps/SDL3/include/SDL3/SDL.h"
#include "../data/deps/SDL3/include/SDL3/SDL_audio.h"

constexpr uint32 SampleRate = 48000;

#define RIFF_CODE(a, b, c, d) (((uint32)(a) << 0) | ((uint32)(b) << 8) | ((uint32)(c) << 16) | ((uint32)(d) << 24)) 
enum 
{
    WAVE_ChunkID_fmt  = RIFF_CODE('f', 'm', 't', ' '),
    WAVE_ChunkID_RIFF = RIFF_CODE('R', 'I', 'F', 'F'),
    WAVE_ChunkID_WAVE = RIFF_CODE('W', 'A', 'V', 'E'),
    WAVE_ChunkID_data = RIFF_CODE('d', 'a', 't', 'a'),
};

#pragma pack(push, 1)
struct WAVE_header
{
    uint32 RIFFID;
    uint32 Size;
    uint32 WAVEID;
};

struct WAVE_chunk
{
    uint32 ID;
    uint32 Size;
};

struct WAVE_fmt
{
    uint16 wFormatTag;
    uint16 nChannels;
    uint32 nSamplesPerSec;
    uint32 nAvgBytesPerSec;
    uint16 nBlockAlign;
    uint16 wBitsPerSample;
    uint16 cbSize;
    uint16 wValidBitsPerSample;
    uint32 dwChannelMask;
    uint8  SubFormat[16];
};
#pragma pack(pop)

struct riff_iterator
{
    uint8      *At;
    uint8      *End;
};

struct loaded_sound
{
    uint32 SoundID;
    uint32 NextID;
    
    uint16 ChannelCount;
    uint32 SampleCount;
    
    uint8 *Samples;
};

struct playing_sound
{
    uint32 SoundID;
    uint32 SamplesConsumed;
    real32 Volume[2];

    playing_sound *Next;
};

struct audio_engine_info
{
    SDL_AudioDeviceID  PrimaryDevice;
    SDL_AudioStream   *SoundSampleBuffer;

    SDL_AudioSpec      InputSpec;
    SDL_AudioSpec      OutputSpec;

    uint32             SampleRate;
    uint32             ChannelCount;
};

struct clover_audio_engine
{
    audio_engine_info *Info;
};

#endif // CLOVER_AUDIOENGINE_H

