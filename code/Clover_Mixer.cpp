/* ========================================================================
   $File: Clover_Mixer.cpp $
   $Date: November 20 2024 09:33 pm $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

internal void
CloverMixerPlayAllSounds(game_memory *GameMemory, sound_buffer *SoundBuffer, time_data Time)
{
    transient_state *TransientState = (transient_state *)GameMemory->TransientStorage.MemoryBlock;
    game_state      *GameState      = (game_state *)GameMemory->PermanentStorage.MemoryBlock;
    real32 *MixerBuffer00 = PushArray(&TransientState->Garbage, real32, SoundBuffer->SampleOutputCount);
    real32 *MixerBuffer01 = PushArray(&TransientState->Garbage, real32, SoundBuffer->SampleOutputCount);

    real32 *Dest00 = MixerBuffer00;
    real32 *Dest01 = MixerBuffer01;

    for(int32 SampleIndex = 0;
            SampleIndex < SoundBuffer->SampleOutputCount;
            ++SampleIndex)
    {
        *Dest00++ = 0;
        *Dest01++ = 0;
    }

    real32 MasterVolume  = 1.0f;
    for(playing_sound **PlayingSoundptr = &GameState->AudioState.FirstPlayingSound;
        *PlayingSoundptr;
       )
    {
        bool32 IsFinished = false;

        playing_sound *PlayingSound = *PlayingSoundptr;
        if(PlayingSound->IsPlaying)
        {
            uint32 TotalSamplesToMix = SoundBuffer->SampleOutputCount;
            Dest00 = MixerBuffer00;
            Dest01 = MixerBuffer01;

            loaded_sound *CurrentSound = GetSoundFromID(TransientState, (soundfx_id)PlayingSound->ID);
            if(CurrentSound)
            {
                if(PlayingSound->ID != PlayingSound->NextIDToPlay)
                {
                    LoadSoundFromID(GameMemory, (soundfx_id)PlayingSound->NextIDToPlay);
                }

                uint32 MixingCount = TotalSamplesToMix;
                real32 RealRemainingSamplesInSound = (CurrentSound->SampleCount - (int32)floorf(PlayingSound->PlayCursor)) / PlayingSound->dPitch;
                uint32 RemainingSamplesInSound = (int32)floorf(RealRemainingSamplesInSound); 
                if(MixingCount > RemainingSamplesInSound)
                {
                    MixingCount = RemainingSamplesInSound;
                }

                real32 SampleIndex = PlayingSound->PlayCursor;
                for(uint32 LoopIndex = 0;
                        LoopIndex < MixingCount;
                        ++LoopIndex)
                {
                    int32  FlooredIndex    = (int32)SampleIndex;

                    int32  SampleOffset           = FlooredIndex % CurrentSound->SampleCount;
                    real32 LeftSampleValue        = real32(CurrentSound->Samples[SampleOffset * 2]);
                    real32 RightSampleValue       = real32(CurrentSound->Samples[(SampleOffset * 2) + 1]);

                    *Dest00++ += LeftSampleValue  * PlayingSound->CurrentVolume[0];
                    *Dest01++ += RightSampleValue * PlayingSound->CurrentVolume[1];

                    SampleIndex += PlayingSound->dPitch;
                }

                // NOTE(Sleepster): Animate the volume 
                v2Approach(&PlayingSound->CurrentVolume, 
                        PlayingSound->TargetVolume, 
                        PlayingSound->dVolumeRate, 
                        Time.Delta);

                PlayingSound->PlayCursor = SampleIndex;
                TotalSamplesToMix -= MixingCount;
                if(PlayingSound->PlayCursor >= CurrentSound->SampleCount)
                {
                    if(IsValid(PlayingSound->NextIDToPlay))
                    {
                        PlayingSound->ID = PlayingSound->NextIDToPlay;
                        PlayingSound->PlayCursor = 0.0f;
                    }
                    else
                    {
                        IsFinished = true;
                    }
                }
            }
            else
            {
                CurrentSound = LoadSoundFromID(GameMemory, (soundfx_id)PlayingSound->ID, PlayingSound->IsStreamed, 
                        PlayingSound->FirstSampleToRead, PlayingSound->SamplesToRead); 
            }

        }
        if(IsFinished)
        {
            *PlayingSoundptr = PlayingSound->Next;
            PlayingSound->Next = GameState->AudioState.FirstFreePlayingSound;
            GameState->AudioState.FirstFreePlayingSound = PlayingSound;
        }
        else
        {
            PlayingSoundptr = &PlayingSound->Next;
        }
    }

    Dest00 = MixerBuffer00;
    Dest01 = MixerBuffer01;
    int16 *SampleOut = SoundBuffer->SampleBuffer;
    for(int32 SampleIndex = 0;
        SampleIndex < SoundBuffer->SampleOutputCount;
        ++SampleIndex)
    {
        *SampleOut++ = int16((*Dest00++ * MasterVolume) + 0.5f);
        *SampleOut++ = int16((*Dest01++ * MasterVolume) + 0.5f);
    }
}
