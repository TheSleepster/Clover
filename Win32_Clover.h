/* date = August 27 2024 06:24 am*/

#ifndef WIN32_CLOVER_H
#define WIN32_CLOVER_H

#include "util/Math.h"
#include "util/Array.h"
#include "util/FileIO.h"
#include "util/CustomStrings.h"

#include "Clover_Renderer.h"

struct wgl_function_pointers
{
    PFNWGLCHOOSEPIXELFORMATARBPROC    wglChoosePixelFormatARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
    PFNWGLSWAPINTERVALEXTPROC         wglSwapIntervalEXT;
};

internal inline FILETIME
Win32MaxFiletime(FILETIME A, FILETIME B)
{
    if(CompareFileTime(&A, &B) != 0)
    {
        return(A);
    }
    return(B);
}

internal FILETIME
Win32GetLastWriteTime(string Filename)
{
    FILETIME LastWriteTime = {};
    
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle = FindFirstFileA((const char *)Filename.Data, &FindData);
    if(FindHandle != INVALID_HANDLE_VALUE)
    {
        LastWriteTime = FindData.ftLastWriteTime;
        FindClose(FindHandle);
    }
    
    return(LastWriteTime);
}

#endif // _WIN32_CLOVER_H

