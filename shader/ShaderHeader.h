/* date = June 17th 2024 3:14 pm */

#ifndef _CLOVER__SHARED_SHADER_HEADER_H
#define _CLOVER__SHARED_SHADER_HEADER_H

#ifdef ENGINE // ENGINE ONLY
#include "Intrinsics.h"
#include "../code/Clover_Renderer.h"

#else // SHADER ONLY

const unsigned int RENDERING_OPTION_FLIP_X = 0x00000001u;
const unsigned int RENDERING_OPTION_FLIP_Y = 0x00000002u;

#endif
// SHARED

#endif //_CLOVER__SHARED_SHADER_HEADER_H
