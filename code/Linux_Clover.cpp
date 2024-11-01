/* ========================================================================
   $File: Linux_Clover.cpp $
   $Date: October 27 2024 11:26 am $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */
#include <SDL3/sdl.h>

#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/CustomStrings.h"
#include "util/Pairs.h"

#include "Intrinsics.h"

int main()
{
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0)
    {
    }
}
