//-----------------------------------------------------------------------------
// File : asdxGfxMisc.cpp
// Desc : Graphics Utility.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxGfxMisc.h>


#define CASE_STRING(x) case x : { return #x; }

namespace asdx {

//-----------------------------------------------------------------------------
//      ミップ数を数え上げます.
//-----------------------------------------------------------------------------
uint32_t CountMips(uint32_t w, uint32_t h)
{
    if (w == 0 || h == 0)
        return 0;

    auto count = 1u;
    while (w > 1 || h > 1)
    {
        w >>= 1;
        h >>= 1;
        count++;
    }
    return count;
}

//-----------------------------------------------------------------------------
//      DXGIフォーマットから1ピクセルあたりのビット数を取得します.
//-----------------------------------------------------------------------------
uint32_t GetBitsPerPixel(DXGI_FORMAT format)
{
    // DirectXTK LoadHelpers.h
    // https://github.com/microsoft/DirectXTK12/blob/main/Src/LoaderHelpers.h

    switch(format)
    {
    case DXGI_FORMAT_R32G32B32A32_TYPELESS:
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
        return 128;

    case DXGI_FORMAT_R32G32B32_TYPELESS:
    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
        return 96;

    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_SINT:
    case DXGI_FORMAT_R32G32_TYPELESS:
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
    case DXGI_FORMAT_Y416:
    case DXGI_FORMAT_Y210:
    case DXGI_FORMAT_Y216:
        return 64;

    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
    case DXGI_FORMAT_R11G11B10_FLOAT:
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
    case DXGI_FORMAT_R16G16_TYPELESS:
    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SNORM:
    case DXGI_FORMAT_R16G16_SINT:
    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT:
    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8X8_TYPELESS:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
    case DXGI_FORMAT_AYUV:
    case DXGI_FORMAT_Y410:
    case DXGI_FORMAT_YUY2:
    #if (defined(_XBOX_ONE) && defined(_TITLE)) || defined(_GAMING_XBOX)
    case DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
    case DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
    case DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM:
    #endif
        return 32;

    case DXGI_FORMAT_P010:
    case DXGI_FORMAT_P016:
    #if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)
    case DXGI_FORMAT_V408:
    #endif
    #if (defined(_XBOX_ONE) && defined(_TITLE)) || defined(_GAMING_XBOX)
    case DXGI_FORMAT_D16_UNORM_S8_UINT:
    case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
    #endif
        return 24;

    case DXGI_FORMAT_R8G8_TYPELESS:
    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R8G8_UINT:
    case DXGI_FORMAT_R8G8_SNORM:
    case DXGI_FORMAT_R8G8_SINT:
    case DXGI_FORMAT_R16_TYPELESS:
    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R16_SNORM:
    case DXGI_FORMAT_R16_SINT:
    case DXGI_FORMAT_B5G6R5_UNORM:
    case DXGI_FORMAT_B5G5R5A1_UNORM:
    case DXGI_FORMAT_A8P8:
    case DXGI_FORMAT_B4G4R4A4_UNORM:
    #if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)
    case DXGI_FORMAT_P208:
    case DXGI_FORMAT_V208:
    #endif
        return 16;

    case DXGI_FORMAT_NV12:
    case DXGI_FORMAT_420_OPAQUE:
    case DXGI_FORMAT_NV11:
        return 12;

    case DXGI_FORMAT_R8_TYPELESS:
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SNORM:
    case DXGI_FORMAT_R8_SINT:
    case DXGI_FORMAT_A8_UNORM:
    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
    case DXGI_FORMAT_AI44:
    case DXGI_FORMAT_IA44:
    case DXGI_FORMAT_P8:
    #if (defined(_XBOX_ONE) && defined(_TITLE)) || defined(_GAMING_XBOX)
    case DXGI_FORMAT_R4G4_UNORM:
    #endif
        return 8;

    case DXGI_FORMAT_R1_UNORM:
        return 1;

    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
        return 4;

    case DXGI_FORMAT_UNKNOWN:
    case DXGI_FORMAT_FORCE_UINT:
    default:
        return 0;
    }
}

//-----------------------------------------------------------------------------
//      DXGIフォーマットから1ピクセルあたりのバイト数を取得します.
//-----------------------------------------------------------------------------
uint32_t GetBytePerPixel(DXGI_FORMAT format)
{
    return GetBitsPerPixel(format) / 8u;
}

//-----------------------------------------------------------------------------
//      深度フォーマットからリソースフォーマットに変換します.
//-----------------------------------------------------------------------------
DXGI_FORMAT GetResourceFormat(DXGI_FORMAT value, bool isStencil)
{
    DXGI_FORMAT result = value;

    switch(value)
    {
    case DXGI_FORMAT_D16_UNORM:
        { result = DXGI_FORMAT_R16_UNORM; }
        break;

    case DXGI_FORMAT_D24_UNORM_S8_UINT:
        {
            if (!isStencil)
                result = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            else
                result = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
        }
        break;

    case DXGI_FORMAT_D32_FLOAT:
        { result = DXGI_FORMAT_R32_FLOAT; }
        break;

    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        {
            if (!isStencil)
                result = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
            else
                result = DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
        }
        break;
    }

    return result;
}

//-----------------------------------------------------------------------------
//      sRGBフォーマットに変換します.
//-----------------------------------------------------------------------------
DXGI_FORMAT GetSRGBFormat(DXGI_FORMAT value)
{
    DXGI_FORMAT result = value;

    switch( value )
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        { result = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; }
        break;

    case DXGI_FORMAT_BC1_UNORM:
        { result = DXGI_FORMAT_BC1_UNORM_SRGB; }
        break;

    case DXGI_FORMAT_BC2_UNORM:
        { result = DXGI_FORMAT_BC2_UNORM_SRGB; }
        break;

    case DXGI_FORMAT_BC3_UNORM:
        { result = DXGI_FORMAT_BC3_UNORM_SRGB; }
        break;

    case DXGI_FORMAT_B8G8R8A8_UNORM:
        { result = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; }
        break;

    case DXGI_FORMAT_B8G8R8X8_UNORM:
        { result = DXGI_FORMAT_B8G8R8X8_UNORM_SRGB; }
        break;

    case DXGI_FORMAT_BC7_UNORM:
        { result = DXGI_FORMAT_BC7_UNORM_SRGB; }
        break;
    }

    return result;
}

//-----------------------------------------------------------------------------
//      非sRGBフォーマットに変換します.
//-----------------------------------------------------------------------------
DXGI_FORMAT GetNoSRGBFormat(DXGI_FORMAT value)
{
    DXGI_FORMAT result = value;

    switch( value )
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        { result = DXGI_FORMAT_R8G8B8A8_UNORM; }
        break;

    case DXGI_FORMAT_BC1_UNORM_SRGB:
        { result = DXGI_FORMAT_BC1_UNORM; }
        break;

    case DXGI_FORMAT_BC2_UNORM_SRGB:
        { result = DXGI_FORMAT_BC2_UNORM; }
        break;

    case DXGI_FORMAT_BC3_UNORM_SRGB:
        { result = DXGI_FORMAT_BC3_UNORM; }
        break;

    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        { result = DXGI_FORMAT_B8G8R8A8_UNORM; }
        break;

    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        { result = DXGI_FORMAT_B8G8R8X8_UNORM; }
        break;

    case DXGI_FORMAT_BC7_UNORM_SRGB:
        { result = DXGI_FORMAT_BC7_UNORM; }
        break;
    }

    return result;
}

//-----------------------------------------------------------------------------
//      sRGBフォーマットかどうかチェックします.
//-----------------------------------------------------------------------------
bool IsSRGBFormat(DXGI_FORMAT value)
{
    bool result = false;
    switch(value)
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        { result = true; }
        break;

    case DXGI_FORMAT_BC1_UNORM_SRGB:
        { result = true; }
        break;

    case DXGI_FORMAT_BC2_UNORM_SRGB:
        { result = true; }
        break;

    case DXGI_FORMAT_BC3_UNORM_SRGB:
        { result = true; }
        break;

    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        { result = true; }
        break;

    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        { result = true; }
        break;

    case DXGI_FORMAT_BC7_UNORM_SRGB:
        { result = true; }
        break;
    }

    return result;
}

//-----------------------------------------------------------------------------
//      圧縮フォーマットかどうかチェックします.
//-----------------------------------------------------------------------------
bool IsCompressed(DXGI_FORMAT format)
{
    switch(format)
    {
    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        return true;

    default:
        return false;
    }
}

//-----------------------------------------------------------------------------
//      DXGIフォーマットを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(DXGI_FORMAT format)
{
    switch(format)
    {
        CASE_STRING(DXGI_FORMAT_UNKNOWN	                                ) // 0,
        CASE_STRING(DXGI_FORMAT_R32G32B32A32_TYPELESS                   ) // 1,
        CASE_STRING(DXGI_FORMAT_R32G32B32A32_FLOAT                      ) // 2,
        CASE_STRING(DXGI_FORMAT_R32G32B32A32_UINT                       ) // 3,
        CASE_STRING(DXGI_FORMAT_R32G32B32A32_SINT                       ) // 4,
        CASE_STRING(DXGI_FORMAT_R32G32B32_TYPELESS                      ) // 5,
        CASE_STRING(DXGI_FORMAT_R32G32B32_FLOAT                         ) // 6,
        CASE_STRING(DXGI_FORMAT_R32G32B32_UINT                          ) // 7,
        CASE_STRING(DXGI_FORMAT_R32G32B32_SINT                          ) // 8,
        CASE_STRING(DXGI_FORMAT_R16G16B16A16_TYPELESS                   ) // 9,
        CASE_STRING(DXGI_FORMAT_R16G16B16A16_FLOAT                      ) // 10,
        CASE_STRING(DXGI_FORMAT_R16G16B16A16_UNORM                      ) // 11,
        CASE_STRING(DXGI_FORMAT_R16G16B16A16_UINT                       ) // 12,
        CASE_STRING(DXGI_FORMAT_R16G16B16A16_SNORM                      ) // 13,
        CASE_STRING(DXGI_FORMAT_R16G16B16A16_SINT                       ) // 14,
        CASE_STRING(DXGI_FORMAT_R32G32_TYPELESS                         ) // 15,
        CASE_STRING(DXGI_FORMAT_R32G32_FLOAT                            ) // 16,
        CASE_STRING(DXGI_FORMAT_R32G32_UINT                             ) // 17,
        CASE_STRING(DXGI_FORMAT_R32G32_SINT                             ) // 18,
        CASE_STRING(DXGI_FORMAT_R32G8X24_TYPELESS                       ) // 19,
        CASE_STRING(DXGI_FORMAT_D32_FLOAT_S8X24_UINT                    ) // 20,
        CASE_STRING(DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS                ) // 21,
        CASE_STRING(DXGI_FORMAT_X32_TYPELESS_G8X24_UINT                 ) // 22,
        CASE_STRING(DXGI_FORMAT_R10G10B10A2_TYPELESS                    ) // 23,
        CASE_STRING(DXGI_FORMAT_R10G10B10A2_UNORM                       ) // 24,
        CASE_STRING(DXGI_FORMAT_R10G10B10A2_UINT                        ) // 25,
        CASE_STRING(DXGI_FORMAT_R11G11B10_FLOAT                         ) // 26,
        CASE_STRING(DXGI_FORMAT_R8G8B8A8_TYPELESS                       ) // 27,
        CASE_STRING(DXGI_FORMAT_R8G8B8A8_UNORM                          ) // 28,
        CASE_STRING(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB                     ) // 29,
        CASE_STRING(DXGI_FORMAT_R8G8B8A8_UINT                           ) // 30,
        CASE_STRING(DXGI_FORMAT_R8G8B8A8_SNORM                          ) // 31,
        CASE_STRING(DXGI_FORMAT_R8G8B8A8_SINT                           ) // 32,
        CASE_STRING(DXGI_FORMAT_R16G16_TYPELESS                         ) // 33,
        CASE_STRING(DXGI_FORMAT_R16G16_FLOAT                            ) // 34,
        CASE_STRING(DXGI_FORMAT_R16G16_UNORM                            ) // 35,
        CASE_STRING(DXGI_FORMAT_R16G16_UINT                             ) // 36,
        CASE_STRING(DXGI_FORMAT_R16G16_SNORM                            ) // 37,
        CASE_STRING(DXGI_FORMAT_R16G16_SINT                             ) // 38,
        CASE_STRING(DXGI_FORMAT_R32_TYPELESS                            ) // 39,
        CASE_STRING(DXGI_FORMAT_D32_FLOAT                               ) // 40,
        CASE_STRING(DXGI_FORMAT_R32_FLOAT                               ) // 41,
        CASE_STRING(DXGI_FORMAT_R32_UINT                                ) // 42,
        CASE_STRING(DXGI_FORMAT_R32_SINT                                ) // 43,
        CASE_STRING(DXGI_FORMAT_R24G8_TYPELESS                          ) // 44,
        CASE_STRING(DXGI_FORMAT_D24_UNORM_S8_UINT                       ) // 45,
        CASE_STRING(DXGI_FORMAT_R24_UNORM_X8_TYPELESS                   ) // 46,
        CASE_STRING(DXGI_FORMAT_X24_TYPELESS_G8_UINT                    ) // 47,
        CASE_STRING(DXGI_FORMAT_R8G8_TYPELESS                           ) // 48,
        CASE_STRING(DXGI_FORMAT_R8G8_UNORM                              ) // 49,
        CASE_STRING(DXGI_FORMAT_R8G8_UINT                               ) // 50,
        CASE_STRING(DXGI_FORMAT_R8G8_SNORM                              ) // 51,
        CASE_STRING(DXGI_FORMAT_R8G8_SINT                               ) // 52,
        CASE_STRING(DXGI_FORMAT_R16_TYPELESS                            ) // 53,
        CASE_STRING(DXGI_FORMAT_R16_FLOAT                               ) // 54,
        CASE_STRING(DXGI_FORMAT_D16_UNORM                               ) // 55,
        CASE_STRING(DXGI_FORMAT_R16_UNORM                               ) // 56,
        CASE_STRING(DXGI_FORMAT_R16_UINT                                ) // 57,
        CASE_STRING(DXGI_FORMAT_R16_SNORM                               ) // 58,
        CASE_STRING(DXGI_FORMAT_R16_SINT                                ) // 59,
        CASE_STRING(DXGI_FORMAT_R8_TYPELESS                             ) // 60,
        CASE_STRING(DXGI_FORMAT_R8_UNORM                                ) // 61,
        CASE_STRING(DXGI_FORMAT_R8_UINT                                 ) // 62,
        CASE_STRING(DXGI_FORMAT_R8_SNORM                                ) // 63,
        CASE_STRING(DXGI_FORMAT_R8_SINT                                 ) // 64,
        CASE_STRING(DXGI_FORMAT_A8_UNORM                                ) // 65,
        CASE_STRING(DXGI_FORMAT_R1_UNORM                                ) // 66,
        CASE_STRING(DXGI_FORMAT_R9G9B9E5_SHAREDEXP                      ) // 67,
        CASE_STRING(DXGI_FORMAT_R8G8_B8G8_UNORM                         ) // 68,
        CASE_STRING(DXGI_FORMAT_G8R8_G8B8_UNORM                         ) // 69,
        CASE_STRING(DXGI_FORMAT_BC1_TYPELESS                            ) // 70,
        CASE_STRING(DXGI_FORMAT_BC1_UNORM                               ) // 71,
        CASE_STRING(DXGI_FORMAT_BC1_UNORM_SRGB                          ) // 72,
        CASE_STRING(DXGI_FORMAT_BC2_TYPELESS                            ) // 73,
        CASE_STRING(DXGI_FORMAT_BC2_UNORM                               ) // 74,
        CASE_STRING(DXGI_FORMAT_BC2_UNORM_SRGB                          ) // 75,
        CASE_STRING(DXGI_FORMAT_BC3_TYPELESS                            ) // 76,
        CASE_STRING(DXGI_FORMAT_BC3_UNORM                               ) // 77,
        CASE_STRING(DXGI_FORMAT_BC3_UNORM_SRGB                          ) // 78,
        CASE_STRING(DXGI_FORMAT_BC4_TYPELESS                            ) // 79,
        CASE_STRING(DXGI_FORMAT_BC4_UNORM                               ) // 80,
        CASE_STRING(DXGI_FORMAT_BC4_SNORM                               ) // 81,
        CASE_STRING(DXGI_FORMAT_BC5_TYPELESS                            ) // 82,
        CASE_STRING(DXGI_FORMAT_BC5_UNORM                               ) // 83,
        CASE_STRING(DXGI_FORMAT_BC5_SNORM                               ) // 84,
        CASE_STRING(DXGI_FORMAT_B5G6R5_UNORM                            ) // 85,
        CASE_STRING(DXGI_FORMAT_B5G5R5A1_UNORM                          ) // 86,
        CASE_STRING(DXGI_FORMAT_B8G8R8A8_UNORM                          ) // 87,
        CASE_STRING(DXGI_FORMAT_B8G8R8X8_UNORM                          ) // 88,
        CASE_STRING(DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM              ) // 89,
        CASE_STRING(DXGI_FORMAT_B8G8R8A8_TYPELESS                       ) // 90,
        CASE_STRING(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB                     ) // 91,
        CASE_STRING(DXGI_FORMAT_B8G8R8X8_TYPELESS                       ) // 92,
        CASE_STRING(DXGI_FORMAT_B8G8R8X8_UNORM_SRGB                     ) // 93,
        CASE_STRING(DXGI_FORMAT_BC6H_TYPELESS                           ) // 94,
        CASE_STRING(DXGI_FORMAT_BC6H_UF16                               ) // 95,
        CASE_STRING(DXGI_FORMAT_BC6H_SF16                               ) // 96,
        CASE_STRING(DXGI_FORMAT_BC7_TYPELESS                            ) // 97,
        CASE_STRING(DXGI_FORMAT_BC7_UNORM                               ) // 98,
        CASE_STRING(DXGI_FORMAT_BC7_UNORM_SRGB                          ) // 99,
        CASE_STRING(DXGI_FORMAT_AYUV                                    ) // 100,
        CASE_STRING(DXGI_FORMAT_Y410                                    ) // 101,
        CASE_STRING(DXGI_FORMAT_Y416                                    ) // 102,
        CASE_STRING(DXGI_FORMAT_NV12                                    ) // 103,
        CASE_STRING(DXGI_FORMAT_P010                                    ) // 104,
        CASE_STRING(DXGI_FORMAT_P016                                    ) // 105,
        CASE_STRING(DXGI_FORMAT_420_OPAQUE                              ) // 106,
        CASE_STRING(DXGI_FORMAT_YUY2                                    ) // 107,
        CASE_STRING(DXGI_FORMAT_Y210                                    ) // 108,
        CASE_STRING(DXGI_FORMAT_Y216                                    ) // 109,
        CASE_STRING(DXGI_FORMAT_NV11                                    ) // 110,
        CASE_STRING(DXGI_FORMAT_AI44                                    ) // 111,
        CASE_STRING(DXGI_FORMAT_IA44                                    ) // 112,
        CASE_STRING(DXGI_FORMAT_P8                                      ) // 113,
        CASE_STRING(DXGI_FORMAT_A8P8                                    ) // 114,
        CASE_STRING(DXGI_FORMAT_B4G4R4A4_UNORM                          ) // 115,

        CASE_STRING(DXGI_FORMAT_P208                                    ) // 130,
        CASE_STRING(DXGI_FORMAT_V208                                    ) // 131,
        CASE_STRING(DXGI_FORMAT_V408                                    ) // 132,


        CASE_STRING(DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE         ) // 189,
        CASE_STRING(DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE ) // 190,

        CASE_STRING(DXGI_FORMAT_A4B4G4R4_UNORM                          ) // 191,


        CASE_STRING(DXGI_FORMAT_FORCE_UINT                  ) // 0xffffffff
        default: return "UNKNOWN";
    }
}

//-----------------------------------------------------------------------------
//      D3D12_RESOURCE_DIMENSIONを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_RESOURCE_DIMENSION dimension)
{
    switch(dimension)
    {
        CASE_STRING(D3D12_RESOURCE_DIMENSION_UNKNOWN)
        CASE_STRING(D3D12_RESOURCE_DIMENSION_BUFFER)
        CASE_STRING(D3D12_RESOURCE_DIMENSION_TEXTURE1D)
        CASE_STRING(D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        CASE_STRING(D3D12_RESOURCE_DIMENSION_TEXTURE3D)
        default: return "UNKNOWN";
    }
}

//-----------------------------------------------------------------------------
//      D3D12_RTV_DIMENSIONを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_RTV_DIMENSION dimension)
{
    switch(dimension)
    {
        CASE_STRING(D3D12_RTV_DIMENSION_UNKNOWN)
        CASE_STRING(D3D12_RTV_DIMENSION_BUFFER)
        CASE_STRING(D3D12_RTV_DIMENSION_TEXTURE1D)
        CASE_STRING(D3D12_RTV_DIMENSION_TEXTURE1DARRAY)
        CASE_STRING(D3D12_RTV_DIMENSION_TEXTURE2D)
        CASE_STRING(D3D12_RTV_DIMENSION_TEXTURE2DARRAY)
        CASE_STRING(D3D12_RTV_DIMENSION_TEXTURE2DMS)
        CASE_STRING(D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY)
        CASE_STRING(D3D12_RTV_DIMENSION_TEXTURE3D)
        default: return "UNKNOWN";
    }
}

//-----------------------------------------------------------------------------
//      D3D12_DSV_DIMENSIONを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_DSV_DIMENSION dimension)
{
    switch(dimension)
    {
        CASE_STRING(D3D12_DSV_DIMENSION_UNKNOWN)
        CASE_STRING(D3D12_DSV_DIMENSION_TEXTURE1D)
        CASE_STRING(D3D12_DSV_DIMENSION_TEXTURE1DARRAY)
        CASE_STRING(D3D12_DSV_DIMENSION_TEXTURE2D)
        CASE_STRING(D3D12_DSV_DIMENSION_TEXTURE2DARRAY)
        CASE_STRING(D3D12_DSV_DIMENSION_TEXTURE2DMS)
        CASE_STRING(D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY)
        default: return "UNKNOWN";
    }
}

//-----------------------------------------------------------------------------
//      D3D12_SRV_DIMENSIONを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_SRV_DIMENSION dimension)
{
    switch(dimension)
    {
        CASE_STRING(D3D12_SRV_DIMENSION_UNKNOWN)
        CASE_STRING(D3D12_SRV_DIMENSION_BUFFER)
        CASE_STRING(D3D12_SRV_DIMENSION_TEXTURE1D)
        CASE_STRING(D3D12_SRV_DIMENSION_TEXTURE1DARRAY)
        CASE_STRING(D3D12_SRV_DIMENSION_TEXTURE2D)
        CASE_STRING(D3D12_SRV_DIMENSION_TEXTURE2DARRAY)
        CASE_STRING(D3D12_SRV_DIMENSION_TEXTURE2DMS)
        CASE_STRING(D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY)
        CASE_STRING(D3D12_SRV_DIMENSION_TEXTURE3D)
        CASE_STRING(D3D12_SRV_DIMENSION_TEXTURECUBE)
        CASE_STRING(D3D12_SRV_DIMENSION_TEXTURECUBEARRAY)
        CASE_STRING(D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE)
        default: return "UNKNOWN";
    }
}

//-----------------------------------------------------------------------------
//      D3D12_UAV_DIMENSIONを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_UAV_DIMENSION dimension)
{
    switch(dimension)
    {
        CASE_STRING(D3D12_UAV_DIMENSION_UNKNOWN)
        CASE_STRING(D3D12_UAV_DIMENSION_BUFFER)
        CASE_STRING(D3D12_UAV_DIMENSION_TEXTURE1D)
        CASE_STRING(D3D12_UAV_DIMENSION_TEXTURE1DARRAY)
        CASE_STRING(D3D12_UAV_DIMENSION_TEXTURE2D)
        CASE_STRING(D3D12_UAV_DIMENSION_TEXTURE2DARRAY)
        CASE_STRING(D3D12_UAV_DIMENSION_TEXTURE2DMS)
        CASE_STRING(D3D12_UAV_DIMENSION_TEXTURE2DMSARRAY)
        CASE_STRING(D3D12_UAV_DIMENSION_TEXTURE3D)
        default: return "UNKNOWN";
    }
}

//-----------------------------------------------------------------------------
//      D3D12_RESOURCE_STATESを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_RESOURCE_STATES states)
{
    switch(states)
    {
        CASE_STRING(D3D12_RESOURCE_STATE_COMMON) //0,
        CASE_STRING(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER) //0x1,
        CASE_STRING(D3D12_RESOURCE_STATE_INDEX_BUFFER) //0x2,
        CASE_STRING(D3D12_RESOURCE_STATE_RENDER_TARGET) //0x4,
        CASE_STRING(D3D12_RESOURCE_STATE_UNORDERED_ACCESS) //0x8,
        CASE_STRING(D3D12_RESOURCE_STATE_DEPTH_WRITE) //0x10,
        CASE_STRING(D3D12_RESOURCE_STATE_DEPTH_READ) //0x20,
        CASE_STRING(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) //0x40,
        CASE_STRING(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) //0x80,
        CASE_STRING(D3D12_RESOURCE_STATE_STREAM_OUT) //0x100,
        CASE_STRING(D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT) //0x200,
        CASE_STRING(D3D12_RESOURCE_STATE_COPY_DEST) //0x400,
        CASE_STRING(D3D12_RESOURCE_STATE_COPY_SOURCE) //0x800,
        CASE_STRING(D3D12_RESOURCE_STATE_RESOLVE_DEST) //0x1000,
        CASE_STRING(D3D12_RESOURCE_STATE_RESOLVE_SOURCE) //0x2000,
        CASE_STRING(D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE) //0x400000,
        CASE_STRING(D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE) //0x1000000,
        CASE_STRING(D3D12_RESOURCE_STATE_RESERVED_INTERNAL_8000) //0x8000,
        CASE_STRING(D3D12_RESOURCE_STATE_RESERVED_INTERNAL_4000) //0x4000,
        CASE_STRING(D3D12_RESOURCE_STATE_RESERVED_INTERNAL_100000) //0x100000,
        CASE_STRING(D3D12_RESOURCE_STATE_RESERVED_INTERNAL_40000000) //0x40000000,
        CASE_STRING(D3D12_RESOURCE_STATE_RESERVED_INTERNAL_80000000) //0x80000000,
        CASE_STRING(D3D12_RESOURCE_STATE_GENERIC_READ) //( ( ( ( ( 0x1 | 0x2 )  | 0x40 )  | 0x80 )  | 0x200 )  | 0x800 ) ,
        CASE_STRING(D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE) //( 0x40 | 0x80 ) ,
        //CASE_STRING(D3D12_RESOURCE_STATE_PRESENT) //0,
        //CASE_STRING(D3D12_RESOURCE_STATE_PREDICATION) //0x200,
        CASE_STRING(D3D12_RESOURCE_STATE_VIDEO_DECODE_READ) //0x10000,
        CASE_STRING(D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE) //0x20000,
        CASE_STRING(D3D12_RESOURCE_STATE_VIDEO_PROCESS_READ) //0x40000,
        CASE_STRING(D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE) //0x80000,
        CASE_STRING(D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ) //0x200000,
        CASE_STRING(D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE) //0x800000
        default: return "UNKNOWN";
    }
}

//-----------------------------------------------------------------------------
//      DXGIフォーマットを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToShortString(DXGI_FORMAT format)
{
    #define FMT_STRING(x) case DXGI_FORMAT_##x : { return #x; }

    switch(format)
    {
        FMT_STRING(UNKNOWN	                                ) // 0,
        FMT_STRING(R32G32B32A32_TYPELESS                   ) // 1,
        FMT_STRING(R32G32B32A32_FLOAT                      ) // 2,
        FMT_STRING(R32G32B32A32_UINT                       ) // 3,
        FMT_STRING(R32G32B32A32_SINT                       ) // 4,
        FMT_STRING(R32G32B32_TYPELESS                      ) // 5,
        FMT_STRING(R32G32B32_FLOAT                         ) // 6,
        FMT_STRING(R32G32B32_UINT                          ) // 7,
        FMT_STRING(R32G32B32_SINT                          ) // 8,
        FMT_STRING(R16G16B16A16_TYPELESS                   ) // 9,
        FMT_STRING(R16G16B16A16_FLOAT                      ) // 10,
        FMT_STRING(R16G16B16A16_UNORM                      ) // 11,
        FMT_STRING(R16G16B16A16_UINT                       ) // 12,
        FMT_STRING(R16G16B16A16_SNORM                      ) // 13,
        FMT_STRING(R16G16B16A16_SINT                       ) // 14,
        FMT_STRING(R32G32_TYPELESS                         ) // 15,
        FMT_STRING(R32G32_FLOAT                            ) // 16,
        FMT_STRING(R32G32_UINT                             ) // 17,
        FMT_STRING(R32G32_SINT                             ) // 18,
        FMT_STRING(R32G8X24_TYPELESS                       ) // 19,
        FMT_STRING(D32_FLOAT_S8X24_UINT                    ) // 20,
        FMT_STRING(R32_FLOAT_X8X24_TYPELESS                ) // 21,
        FMT_STRING(X32_TYPELESS_G8X24_UINT                 ) // 22,
        FMT_STRING(R10G10B10A2_TYPELESS                    ) // 23,
        FMT_STRING(R10G10B10A2_UNORM                       ) // 24,
        FMT_STRING(R10G10B10A2_UINT                        ) // 25,
        FMT_STRING(R11G11B10_FLOAT                         ) // 26,
        FMT_STRING(R8G8B8A8_TYPELESS                       ) // 27,
        FMT_STRING(R8G8B8A8_UNORM                          ) // 28,
        FMT_STRING(R8G8B8A8_UNORM_SRGB                     ) // 29,
        FMT_STRING(R8G8B8A8_UINT                           ) // 30,
        FMT_STRING(R8G8B8A8_SNORM                          ) // 31,
        FMT_STRING(R8G8B8A8_SINT                           ) // 32,
        FMT_STRING(R16G16_TYPELESS                         ) // 33,
        FMT_STRING(R16G16_FLOAT                            ) // 34,
        FMT_STRING(R16G16_UNORM                            ) // 35,
        FMT_STRING(R16G16_UINT                             ) // 36,
        FMT_STRING(R16G16_SNORM                            ) // 37,
        FMT_STRING(R16G16_SINT                             ) // 38,
        FMT_STRING(R32_TYPELESS                            ) // 39,
        FMT_STRING(D32_FLOAT                               ) // 40,
        FMT_STRING(R32_FLOAT                               ) // 41,
        FMT_STRING(R32_UINT                                ) // 42,
        FMT_STRING(R32_SINT                                ) // 43,
        FMT_STRING(R24G8_TYPELESS                          ) // 44,
        FMT_STRING(D24_UNORM_S8_UINT                       ) // 45,
        FMT_STRING(R24_UNORM_X8_TYPELESS                   ) // 46,
        FMT_STRING(X24_TYPELESS_G8_UINT                    ) // 47,
        FMT_STRING(R8G8_TYPELESS                           ) // 48,
        FMT_STRING(R8G8_UNORM                              ) // 49,
        FMT_STRING(R8G8_UINT                               ) // 50,
        FMT_STRING(R8G8_SNORM                              ) // 51,
        FMT_STRING(R8G8_SINT                               ) // 52,
        FMT_STRING(R16_TYPELESS                            ) // 53,
        FMT_STRING(R16_FLOAT                               ) // 54,
        FMT_STRING(D16_UNORM                               ) // 55,
        FMT_STRING(R16_UNORM                               ) // 56,
        FMT_STRING(R16_UINT                                ) // 57,
        FMT_STRING(R16_SNORM                               ) // 58,
        FMT_STRING(R16_SINT                                ) // 59,
        FMT_STRING(R8_TYPELESS                             ) // 60,
        FMT_STRING(R8_UNORM                                ) // 61,
        FMT_STRING(R8_UINT                                 ) // 62,
        FMT_STRING(R8_SNORM                                ) // 63,
        FMT_STRING(R8_SINT                                 ) // 64,
        FMT_STRING(A8_UNORM                                ) // 65,
        FMT_STRING(R1_UNORM                                ) // 66,
        FMT_STRING(R9G9B9E5_SHAREDEXP                      ) // 67,
        FMT_STRING(R8G8_B8G8_UNORM                         ) // 68,
        FMT_STRING(G8R8_G8B8_UNORM                         ) // 69,
        FMT_STRING(BC1_TYPELESS                            ) // 70,
        FMT_STRING(BC1_UNORM                               ) // 71,
        FMT_STRING(BC1_UNORM_SRGB                          ) // 72,
        FMT_STRING(BC2_TYPELESS                            ) // 73,
        FMT_STRING(BC2_UNORM                               ) // 74,
        FMT_STRING(BC2_UNORM_SRGB                          ) // 75,
        FMT_STRING(BC3_TYPELESS                            ) // 76,
        FMT_STRING(BC3_UNORM                               ) // 77,
        FMT_STRING(BC3_UNORM_SRGB                          ) // 78,
        FMT_STRING(BC4_TYPELESS                            ) // 79,
        FMT_STRING(BC4_UNORM                               ) // 80,
        FMT_STRING(BC4_SNORM                               ) // 81,
        FMT_STRING(BC5_TYPELESS                            ) // 82,
        FMT_STRING(BC5_UNORM                               ) // 83,
        FMT_STRING(BC5_SNORM                               ) // 84,
        FMT_STRING(B5G6R5_UNORM                            ) // 85,
        FMT_STRING(B5G5R5A1_UNORM                          ) // 86,
        FMT_STRING(B8G8R8A8_UNORM                          ) // 87,
        FMT_STRING(B8G8R8X8_UNORM                          ) // 88,
        FMT_STRING(R10G10B10_XR_BIAS_A2_UNORM              ) // 89,
        FMT_STRING(B8G8R8A8_TYPELESS                       ) // 90,
        FMT_STRING(B8G8R8A8_UNORM_SRGB                     ) // 91,
        FMT_STRING(B8G8R8X8_TYPELESS                       ) // 92,
        FMT_STRING(B8G8R8X8_UNORM_SRGB                     ) // 93,
        FMT_STRING(BC6H_TYPELESS                           ) // 94,
        FMT_STRING(BC6H_UF16                               ) // 95,
        FMT_STRING(BC6H_SF16                               ) // 96,
        FMT_STRING(BC7_TYPELESS                            ) // 97,
        FMT_STRING(BC7_UNORM                               ) // 98,
        FMT_STRING(BC7_UNORM_SRGB                          ) // 99,
        FMT_STRING(AYUV                                    ) // 100,
        FMT_STRING(Y410                                    ) // 101,
        FMT_STRING(Y416                                    ) // 102,
        FMT_STRING(NV12                                    ) // 103,
        FMT_STRING(P010                                    ) // 104,
        FMT_STRING(P016                                    ) // 105,
        FMT_STRING(420_OPAQUE                              ) // 106,
        FMT_STRING(YUY2                                    ) // 107,
        FMT_STRING(Y210                                    ) // 108,
        FMT_STRING(Y216                                    ) // 109,
        FMT_STRING(NV11                                    ) // 110,
        FMT_STRING(AI44                                    ) // 111,
        FMT_STRING(IA44                                    ) // 112,
        FMT_STRING(P8                                      ) // 113,
        FMT_STRING(A8P8                                    ) // 114,
        FMT_STRING(B4G4R4A4_UNORM                          ) // 115,

        FMT_STRING(P208                                    ) // 130,
        FMT_STRING(V208                                    ) // 131,
        FMT_STRING(V408                                    ) // 132,


        FMT_STRING(SAMPLER_FEEDBACK_MIN_MIP_OPAQUE         ) // 189,
        FMT_STRING(SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE ) // 190,

        FMT_STRING(A4B4G4R4_UNORM                          ) // 191,


        FMT_STRING(FORCE_UINT                  ) // 0xffffffff
        default: return "UNKNOWN";
    }

    #undef FMT_STRING
}

//-----------------------------------------------------------------------------
//      D3D12_RESOURCE_DIMENSIONを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToShortString(D3D12_RESOURCE_DIMENSION dimension)
{
    #define DIM_STRING(x) case D3D12_RESOURCE_DIMENSION_##x : { return #x; }

    switch(dimension)
    {
        DIM_STRING(UNKNOWN)
        DIM_STRING(BUFFER)
        DIM_STRING(TEXTURE1D)
        DIM_STRING(TEXTURE2D)
        DIM_STRING(TEXTURE3D)
        default: return "UNKNOWN";
    }

    #undef DIM_STRING
}

//-----------------------------------------------------------------------------
//      D3D12_RTV_DIMENSIONを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToShortString(D3D12_RTV_DIMENSION dimension)
{
    #define DIM_STRING(x) case D3D12_RTV_DIMENSION_##x : { return #x; }

    switch(dimension)
    {
        DIM_STRING(UNKNOWN)
        DIM_STRING(BUFFER)
        DIM_STRING(TEXTURE1D)
        DIM_STRING(TEXTURE1DARRAY)
        DIM_STRING(TEXTURE2D)
        DIM_STRING(TEXTURE2DARRAY)
        DIM_STRING(TEXTURE2DMS)
        DIM_STRING(TEXTURE2DMSARRAY)
        DIM_STRING(TEXTURE3D)
        default: return "UNKNOWN";
    }

    #undef DIM_STRING
}

//-----------------------------------------------------------------------------
//      D3D12_DSV_DIMENSIONを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToShortString(D3D12_DSV_DIMENSION dimension)
{
    #define DIM_STRING(x) case D3D12_DSV_DIMENSION_##x : { return #x; }

    switch(dimension)
    {
        DIM_STRING(UNKNOWN)
        DIM_STRING(TEXTURE1D)
        DIM_STRING(TEXTURE1DARRAY)
        DIM_STRING(TEXTURE2D)
        DIM_STRING(TEXTURE2DARRAY)
        DIM_STRING(TEXTURE2DMS)
        DIM_STRING(TEXTURE2DMSARRAY)
        default: return "UNKNOWN";
    }

    #undef DIM_STRING
}

//-----------------------------------------------------------------------------
//      D3D12_SRV_DIMENSIONを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToShortString(D3D12_SRV_DIMENSION dimension)
{
    #define DIM_STRING(x) case D3D12_SRV_DIMENSION_##x : { return #x; }

    switch(dimension)
    {
        DIM_STRING(UNKNOWN)
        DIM_STRING(BUFFER)
        DIM_STRING(TEXTURE1D)
        DIM_STRING(TEXTURE1DARRAY)
        DIM_STRING(TEXTURE2D)
        DIM_STRING(TEXTURE2DARRAY)
        DIM_STRING(TEXTURE2DMS)
        DIM_STRING(TEXTURE2DMSARRAY)
        DIM_STRING(TEXTURE3D)
        DIM_STRING(TEXTURECUBE)
        DIM_STRING(TEXTURECUBEARRAY)
        DIM_STRING(RAYTRACING_ACCELERATION_STRUCTURE)
        default: return "UNKNOWN";
    }

    #undef DIM_STRING
}

//-----------------------------------------------------------------------------
//      D3D12_UAV_DIMENSIONを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToShortString(D3D12_UAV_DIMENSION dimension)
{
    #define DIM_STRING(x) case D3D12_UAV_DIMENSION_##x : { return #x; }

    switch(dimension)
    {
        DIM_STRING(UNKNOWN)
        DIM_STRING(BUFFER)
        DIM_STRING(TEXTURE1D)
        DIM_STRING(TEXTURE1DARRAY)
        DIM_STRING(TEXTURE2D)
        DIM_STRING(TEXTURE2DARRAY)
        DIM_STRING(TEXTURE2DMS)
        DIM_STRING(TEXTURE2DMSARRAY)
        DIM_STRING(TEXTURE3D)
        default: return "UNKNOWN";
    }

    #undef DIM_STRING
}

//-----------------------------------------------------------------------------
//      D3D12_RESOURCE_STATESを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToShortString(D3D12_RESOURCE_STATES states)
{
    #define STATE_STRING(x) case D3D12_RESOURCE_STATE_##x : { return #x; }

    switch(states)
    {
        STATE_STRING(COMMON) //0,
        STATE_STRING(VERTEX_AND_CONSTANT_BUFFER) //0x1,
        STATE_STRING(INDEX_BUFFER) //0x2,
        STATE_STRING(RENDER_TARGET) //0x4,
        STATE_STRING(UNORDERED_ACCESS) //0x8,
        STATE_STRING(DEPTH_WRITE) //0x10,
        STATE_STRING(DEPTH_READ) //0x20,
        STATE_STRING(NON_PIXEL_SHADER_RESOURCE) //0x40,
        STATE_STRING(PIXEL_SHADER_RESOURCE) //0x80,
        STATE_STRING(STREAM_OUT) //0x100,
        STATE_STRING(INDIRECT_ARGUMENT) //0x200,
        STATE_STRING(COPY_DEST) //0x400,
        STATE_STRING(COPY_SOURCE) //0x800,
        STATE_STRING(RESOLVE_DEST) //0x1000,
        STATE_STRING(RESOLVE_SOURCE) //0x2000,
        STATE_STRING(RAYTRACING_ACCELERATION_STRUCTURE) //0x400000,
        STATE_STRING(SHADING_RATE_SOURCE) //0x1000000,
        STATE_STRING(RESERVED_INTERNAL_8000) //0x8000,
        STATE_STRING(RESERVED_INTERNAL_4000) //0x4000,
        STATE_STRING(RESERVED_INTERNAL_100000) //0x100000,
        STATE_STRING(RESERVED_INTERNAL_40000000) //0x40000000,
        STATE_STRING(RESERVED_INTERNAL_80000000) //0x80000000,
        STATE_STRING(GENERIC_READ) //( ( ( ( ( 0x1 | 0x2 )  | 0x40 )  | 0x80 )  | 0x200 )  | 0x800 ) ,
        STATE_STRING(ALL_SHADER_RESOURCE) //( 0x40 | 0x80 ) ,
        //STATE_STRING(PRESENT) //0,
        //STATE_STRING(PREDICATION) //0x200,
        STATE_STRING(VIDEO_DECODE_READ) //0x10000,
        STATE_STRING(VIDEO_DECODE_WRITE) //0x20000,
        STATE_STRING(VIDEO_PROCESS_READ) //0x40000,
        STATE_STRING(VIDEO_PROCESS_WRITE) //0x80000,
        STATE_STRING(VIDEO_ENCODE_READ) //0x200000,
        STATE_STRING(VIDEO_ENCODE_WRITE) //0x800000
        default: return "UNKNOWN";
    }

    #undef STATE_STRING
}


} // namespace asdx

#undef CASE_STRING
