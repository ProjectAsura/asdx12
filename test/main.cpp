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
#include <asdxResModel.h>

int main(int argc, char** argv)
{
    #if defined(DEBUG) || defined(_DEBUG)
        _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
        //_CrtSetBreakAlloc(277);
    #endif

#if 0
    {
        asdx::Vector3 v1(1.0f, 0.0f, 0.0f);
        asdx::Vector3 v2(0.0f, 1.0f, 0.0f);
        asdx::Vector3 v3(0.0f, 0.0f, 1.0f);

        auto tbn0 = asdx::EncodeTBN(v1, v2, 0);
        asdx::Vector3 d0, d1, d2;
        asdx::DecodeTBN(tbn0, d0, d1, d2);

        asdx::Vector3 v4(-1.0f, 0.0f, 0.0f);
        asdx::Vector3 v5(0.0f, -1.0f, 0.0f);
        asdx::Vector3 v6(0.0f, 0.0f, -1.0f);
        auto tbn1 = asdx::EncodeTBN(v4, v5, 0);
        asdx::Vector3 d3, d4, d5;
        asdx::DecodeTBN(tbn1, d3, d4, d5);

        asdx::Vector2 t(1.0f, 0.0f);
        auto packedT = asdx::EncodeTexCoord(t);
        auto decodeT = asdx::DecodeTexCoord(packedT);
        printf("decodedT(%f, %f)", decodeT.x, decodeT.y);
    }
#endif

    TestApp().Run();

    return 0;
}