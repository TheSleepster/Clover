#if !defined(CLOVER_PLATFORM_H)
/* ========================================================================
   $File: Clover_Platform.h $
   $Date: November 12 2024 08:21 am $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define CLOVER_PLATFORM_H

#include "Intrinsics.h"

#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/String.h"
#include "util/Pairs.h"

#include "Clover.h"
#include "Clover_Renderer.h"


/* struct win32_master_queue_manager */
/* { */
/*     uint32 OpenThreadCount; */
/*     win32_thread_info *Threads; */
    
/*     uint32 QueueCount; */
/*     work_queue *Queues; */

/*     HANDLE Semaphore; */
/* }; */


struct platform_work_queue;
#define PLATFORM_JOB_ENTRY_CALLBACK(name) void name(platform_work_queue *Queue, void *Data)
typedef PLATFORM_JOB_ENTRY_CALLBACK(platform_job_entry_callback);
typedef void platform_add_entry(platform_work_queue *Queue, platform_job_entry_callback *Callback, void *Data);
typedef void platform_flush_worker_entries(platform_work_queue *Queue);

struct game_memory
{
    bool32                         IsInitialized;

    memory_block                   PermanentStorage;
    memory_block                   TransientStorage;

    platform_work_queue           *HighPriorityQueue;
    platform_work_queue           *LowPriorityQueue;

    platform_add_entry            *AddWorkQueueEntry;
    platform_flush_worker_entries *FlushAllWorkQueueEntries;
};

struct time_data
{
    real32 Delta;
    real32 Current;
    real64 Alpha;
    real64 Next;
    
    int32  FPSCounter;
    real32 MSPerFrame;
    real32 CurrentTimeInSeconds;
};


#define GAME_ON_AWAKE(name) void name(game_memory *Memory, gl_render_data *RenderData, game_state *State, transient_state *TransientState)
typedef GAME_ON_AWAKE(game_on_awake);
GAME_ON_AWAKE(GameOnAwakeStub)
{
}

#define GAME_FIXED_UPDATE(name) void name(game_memory *Memory, gl_render_data *RenderData, game_state *State, transient_state *TransientState, time_data Time)
typedef GAME_FIXED_UPDATE(game_fixed_update);
GAME_FIXED_UPDATE(GameFixedUpdateStub)
{
}

#define GAME_UPDATE_AND_DRAW(name) void name(game_memory *Memory, gl_render_data *RenderData, game_state *State, transient_state *TransientState, time_data Time, ivec4 SizeDataIn)
typedef GAME_UPDATE_AND_DRAW(game_update_and_draw);
GAME_UPDATE_AND_DRAW(GameUpdateAndDrawStub)
{
}

#define GAME_GET_SOUND_SAMPLES(name) void name(game_memory *Memory, sound_buffer *SoundBuffer, game_state *State, transient_state *TransientState)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);
GAME_GET_SOUND_SAMPLES(GameGetSoundSamplesStub)
{
}

#endif // CLOVER_PLATFORM_H

