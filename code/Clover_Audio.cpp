/* ========================================================================
   $File: Clover_Audio.cpp $
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

internal inline bool32
IsChunkValid(riff_iterator Iter)
{
    bool32 Result = (Iter.At < Iter.End);
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


internal void 
CloverLoadWAVFile(memory_arena *Memory, loaded_sound *Sound, string Filepath)
{
    uint32 FileSize = {};
    string FileContents = ReadEntireFileMA(Memory, Filepath, &FileSize);

    if(FileContents.Data != 0)
    {
        WAVE_header *Header = (WAVE_header *)FileContents.Data;

        Assert(Header->RIFFID == WAVE_ChunkID_RIFF);
        Assert(Header->WAVEID == WAVE_ChunkID_WAVE);

        uint16 ChannelCount   = 0;
        uint32 SampleDataSize = 0;
        int16 *SampleData     = 0;
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
                    SampleData = (int16 *)GetChunkData(Iter);
                    SampleDataSize = GetChunkDataSize(Iter);
                }break;
            }
        }
        Assert(ChannelCount && SampleData);
        Sound->ChannelCount = ChannelCount;
        Sound->SampleCount  = (SampleDataSize / (sizeof(uint8)));
        Sound->SampleCount  = (Sound->SampleCount + 1) & ~1;
        // NOTE(Sleepster): Mono/Stereo 
        if(ChannelCount == 1||ChannelCount == 2)
        {
            Sound->Samples = SampleData;
        }
        // NOTE(Sleepster): IDK like 5.1 or something  
        else
        {
            Check(0, "Unsupported Channel Count!\n");
        }
    }
}

internal playing_sound *
PlaySound(game_state *GameState, soundfx_id SoundID)
{
    if(!GameState->FirstFreePlayingSound)
    {
        GameState->FirstFreePlayingSound = PushStruct(&GameState->SoundArena, playing_sound);
        GameState->FirstFreePlayingSound->Next = 0;
    }

    playing_sound *PlayingSound = GameState->FirstFreePlayingSound;
    GameState->FirstFreePlayingSound = PlayingSound->Next;

    PlayingSound->IDToPlay = SoundID;
    PlayingSound->Volume   = vec2{1.0f, 1.0f};

    PlayingSound->Next = GameState->FirstPlayingSound;
    GameState->FirstPlayingSound = PlayingSound;
    
    return(PlayingSound);
}
