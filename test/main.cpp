//-----------------------------------------------------------------------------
// File : main.cpp
// Desc : Application Main Entry Point.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

#if defined(DEBUG) || defined(_DEBUG)
    #define _CRTDBG_MAP_ALLOC
    #include <cstdlib>
    #include <crtdbg.h>
#endif

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "TestApp.h"

int main(int argc, char** argv)
{
    #if defined(DEBUG) || defined(_DEBUG)
        _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
        //_CrtSetBreakAlloc(219);
    #endif

    TestApp().Run();

    return 0;
}