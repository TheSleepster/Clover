/* ========================================================================
   $File: Clover_AudioEngine.cpp $
   $Date: October 30 2024 11:05 am $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */
#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/String.h"
#include "util/Pairs.h"

#include "Intrinsics.h"

// NOTE(Sleepster): As of right now SDL is being used ONLY for the audio engine.
#include "../data/deps/SDL3/include/SDL3/SDL.h"
#include "../data/deps/SDL3/include/SDL3/SDL_audio.h"

// NOTE(Sleepster): This function is a mess, sorry
internal void
InitAudio(audio_engine_info *InputEngine)
{
    InputEngine->InputSpec     = {.format = SDL_AUDIO_S16LE, .channels = 2, .freq = SampleRate};
    InputEngine->PrimaryDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, 0);
    if(InputEngine->PrimaryDevice != 0)
    {
        if(SDL_GetAudioDeviceFormat(InputEngine->PrimaryDevice, &InputEngine->OutputSpec, 0))
        {
            InputEngine->OutputSpec     = {.format = SDL_AUDIO_S16LE, .channels = 2, .freq = SampleRate};
            InputEngine->SoundSampleBuffer = SDL_CreateAudioStream(&InputEngine->InputSpec, &InputEngine->OutputSpec);
            Check(InputEngine->SoundSampleBuffer != 0, 
                  "[ERROR]: Failed to create an SDL_AudioStream. Error code: %s\n", SDL_GetError());

            Check(SDL_BindAudioStream(InputEngine->PrimaryDevice, InputEngine->SoundSampleBuffer), 
                  "[ERROR]: Failure to attach the sound buffer to the output deviec. Error code: %s\n", SDL_GetError());
        }
        else 
        {Check(1 == 0, "[ERROR]: Failure to open the default output device. Error code: %s\n", SDL_GetError());}
    }
    else 
    {Check(1 == 0, "[ERROR]: Failure to open the default output device. Error code: %s\n", SDL_GetError());}
}

internal inline riff_iterator
ParseChunkAt(void *At, void *End)
{
    riff_iterator Iter;
    Iter.At           = (uint8 *)At;
    Iter.End          = (uint8 *)End;

    return(Iter);
}

internal inline riff_iterator
NextChunk(riff_iterator Iter)
{
    WAVE_chunk *Chunk = (WAVE_chunk *)Iter.At;
    
    uint32 Size = (Chunk->Size + 1) & ~1;
    Iter.At += sizeof(WAVE_chunk) + Size;
    return(Iter);
}

internal inline bool
IsChunkValid(riff_iterator Iter)
{
    bool Result = (Iter.At < Iter.End);
    return(Result);
}

internal inline void *
GetChunkData(riff_iterator Iter)
{
    void *Result = (Iter.At + sizeof(WAVE_chunk));
    return(Result);
}

internal inline int32
GetChunkDataSize(riff_iterator Iter)
{
    WAVE_chunk *Chunk = (WAVE_chunk *)Iter.At;
    return(Chunk->Size);
}    

internal inline uint32
GetType(riff_iterator Iter)
{
    WAVE_chunk *Chunk = (WAVE_chunk *)Iter.At;
    uint32 Result = Chunk->ID;
    return(Result);
}

internal loaded_sound
CloverLoadWAVFile(memory_arena *Memory, audio_engine_info *InputEngine, string Filepath)
{
    loaded_sound Result = {};

    uint32 FileSize = {};
    string FileContents = ReadEntireFileMA(Memory, Filepath, &FileSize);

    if(FileContents.Data != 0)
    {
        WAVE_header *Header = (WAVE_header *)FileContents.Data;

        Assert(Header->RIFFID == WAVE_ChunkID_RIFF);
        Assert(Header->WAVEID == WAVE_ChunkID_WAVE);

        uint32 ChannelCount   = 0;
        uint32 SampleDataSize = 0;
        uint8 *SampleData     = 0;
        for(riff_iterator Iter = ParseChunkAt(Header + 1, (uint8 *)(Header + 1) + Header->Size - 4);
            IsChunkValid(Iter);
            Iter = NextChunk(Iter))
        {
            switch(GetType(Iter))
            {
                case WAVE_ChunkID_fmt:
                {
                    WAVE_fmt *fmt = (WAVE_fmt *)GetChunkData(Iter);
                    Assert(fmt->wFormatTag == 1);         // NOTE(Sleepster): Strictly PCM 
                    Assert(fmt->nSamplesPerSec == 48000);
                    Assert(fmt->nBlockAlign == (2*fmt->nChannels));

                    ChannelCount = fmt->nChannels;
                }break;
                case WAVE_ChunkID_data:
                {
                    SampleData = (uint8 *)GetChunkData(Iter);
                    SampleDataSize = GetChunkDataSize(Iter);
                }break;
            }
        }
        Assert(ChannelCount && SampleData);
        Result.ChannelCount = ChannelCount;
        Result.SampleCount  = SampleDataSize / (sizeof(uint8));
        Result.SampleCount  = (Result.SampleCount + 1) & ~1;
        // NOTE(Sleepster): Mono/Stereo 
        if(ChannelCount == 1||ChannelCount == 2)
        {
            Result.Samples = SampleData;
        }
        // NOTE(Sleepster): IDK like 5.1 or something  
        else
        {
            Check(0 == 1, "Unsupported Channel Count!\n");
        }
    }

    return(Result);
}

internal inline void
CloverPlayWAVFile_IO(memory_arena *Memory, audio_engine_info *InputEngine, string Filepath)
{
    loaded_sound Sound = CloverLoadWAVFile(Memory, InputEngine, Filepath);
    if(!SDL_PutAudioStreamData(InputEngine->SoundSampleBuffer, (const void *)Sound.Samples, Sound.SampleCount))
    {
        printm("[ERROR]: SDL has failed to add audio stream data to our buffer. Error code: %s\n", SDL_GetError());
        Assert(1 == 0);
    }
}
