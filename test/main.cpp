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
#include <asdxLogger.h>

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
        ILOGA("input  T(%f, %f, %f), N(%f, %f, %f)", v2.x, v2.y, v2.z, v1.x, v1.y, v1.z);
        ILOGA("decode T(%f, %f, %f), N(%f, %f, %f)", d0.x, d0.y, d0.z, d2.x, d2.y, d2.z);

        asdx::Vector3 v4(-1.0f, 0.0f, 0.0f);
        asdx::Vector3 v5(0.0f, -1.0f, 0.0f);
        asdx::Vector3 v6(0.0f, 0.0f, -1.0f);
        auto tbn1 = asdx::EncodeTBN(v4, v5, 0);
        asdx::Vector3 d3, d4, d5;
        asdx::DecodeTBN(tbn1, d3, d4, d5);
        ILOGA("input  T(%f, %f, %f), N(%f, %f, %f)", v5.x, v5.y, v5.z, v4.x, v4.y, v4.z);
        ILOGA("decode T(%f, %f, %f), N(%f, %f, %f)", d3.x, d3.y, d3.z, d5.x, d5.y, d5.z);


        asdx::Vector2 t(1.0f, 0.0f);
        auto packedT = asdx::EncodeHalf2(t);
        auto decodeT = asdx::DecodeHalf2(packedT);
        ILOGA("decodedT(%f, %f)", decodeT.x, decodeT.y);
    }
#endif

    TestApp().Run();

    return 0;
}