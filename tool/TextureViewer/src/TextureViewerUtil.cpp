//-----------------------------------------------------------------------------
// File : TextureViewerUtil.h
// Desc : Texture Viewer Utility.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <TextureViewerUtil.h>


namespace {

///////////////////////////////////////////////////////////////////////////////
// DXGIFormatTable structure
///////////////////////////////////////////////////////////////////////////////
struct DXGIFormatTable
{
    DXGI_FORMAT     Format;
    const char*     Text;
};

// タイプレスフォーマットとDXGI_FORMAT_UNKNONWは含みません.
static const DXGIFormatTable g_FormatTable[] = {
    { DXGI_FORMAT_R32G32B32A32_FLOAT    , "128 bit - R32G32B32A32_FLOAT" }, // [0] : 2
    { DXGI_FORMAT_R32G32B32A32_UINT     , "128 bit - R32G32B32A32_UINT" },  // [1] : 3
    { DXGI_FORMAT_R32G32B32A32_SINT     , "128 bit - R32G32B32A32_SINT" },  // [2] : 4
    { DXGI_FORMAT_R32G32B32_FLOAT       , "96 bit - R32G32B32_FLOAT" },     // [3] : 6
    { DXGI_FORMAT_R32G32B32_UINT        , "96 bit - R32G32B32_UINT" },      // [4] : 7
    { DXGI_FORMAT_R32G32B32_SINT        , "96 bit - R32G32B32_SINT" },      // [5] : 8
    { DXGI_FORMAT_R16G16B16A16_FLOAT    , "64 bit - R16G16B16A16_FLOAT" },  // [6] : 10
    { DXGI_FORMAT_R16G16B16A16_UNORM    , "64 bit - R16G16B16A16_UNORM" },  // [7] : 11
    { DXGI_FORMAT_R16G16B16A16_UINT     , "64 bit - R16G16B16A16_UINT" },   // [8] : 12
    { DXGI_FORMAT_R16G16B16A16_SNORM    , "64 bit - R16G16B16A16_SNORM" },  // [9] : 13
    { DXGI_FORMAT_R16G16B16A16_SINT     , "64 bit - R16G16B16A16_SINT" },   // [10] : 14
    { DXGI_FORMAT_R32G32_FLOAT          , "64 bit - R32G32_FLOAT" },        // [11] : 16
    { DXGI_FORMAT_R32G32_UINT           , "64 bit - R32G32_UINT" },         // [12] : 17
    { DXGI_FORMAT_R32G32_SINT           , "64 bit - R32G32_SINT" },         // [13] : 18
    { DXGI_FORMAT_R10G10B10A2_UNORM     , "32 bit - R10G10B10A2_UNORM" },   // [14] : 23,
    { DXGI_FORMAT_R10G10B10A2_UINT      , "32 bit - R10G10B10A2_UINT" },    // [15] : 25
    { DXGI_FORMAT_R11G11B10_FLOAT       , "32 bit - R11G11B10_FLOAT" },     // [16] : 26
    { DXGI_FORMAT_R8G8B8A8_UNORM        , "32 bit - R8G8B8A8_UNORM" },      // [17] : 28
    { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB   , "32 bit - R8G8B8A8_UNORM_SRGB" }, // [18] : 29
    { DXGI_FORMAT_R8G8B8A8_UINT         , "32 bit - R8G8B8A8_UINT" },       // [19] : 30
    { DXGI_FORMAT_R8G8B8A8_SNORM        , "32 bit - R8G8B8A8_SNORM" },      // [20] : 31
    { DXGI_FORMAT_R8G8B8A8_SINT         , "32 bit - R8G8B8A8_SINT" },       // [21] : 32
    { DXGI_FORMAT_R16G16_FLOAT          , "32 bit - R16G16_FLOAT" },        // [22] : 34
    { DXGI_FORMAT_R16G16_UNORM          , "32 bit - R16G16_UNORM" },        // [23] : 35
    { DXGI_FORMAT_R16G16_UINT           , "32 bit - R16G16_UINT" },         // [24] : 36
    { DXGI_FORMAT_R16G16_SNORM          , "32 bit - R16G16_SNORM" },        // [25] : 37
    { DXGI_FORMAT_R16G16_SINT           , "32 bit - R16G16_SINT" },         // [26] : 38
    { DXGI_FORMAT_R32_FLOAT             , "32 bit - R32_FLOAT" },           // [27] : 41
    { DXGI_FORMAT_R32_UINT              , "32 bit - R32_UINT" },            // [28] : 42
    { DXGI_FORMAT_R32_SINT              , "32 bit - R32_SINT" },            // [29] : 43
    { DXGI_FORMAT_R8G8_UNORM            , "16 bit - R8G8_UNORM" },          // [30] : 49
    { DXGI_FORMAT_R8G8_UINT             , "16 bit - R8G8_UINT" },           // [31] : 50
    { DXGI_FORMAT_R8G8_SNORM            , "16 bit - R8G8_SNORM" },          // [32] : 51
    { DXGI_FORMAT_R8G8_SINT             , "16 bit - R8G8_SINT" },           // [33] : 52
    { DXGI_FORMAT_R16_FLOAT             , "16 bit - R16_FLOAT" },           // [34] : 54
    { DXGI_FORMAT_R16_UNORM             , "16 bit - R16_UNORM" },           // [35] : 56
    { DXGI_FORMAT_R16_UINT              , "16 bit - R16_UINT" },            // [36] : 57
    { DXGI_FORMAT_R16_SNORM             , "16 bit - R16_SNORM" },           // [37] : 58
    { DXGI_FORMAT_R16_SINT              , "16 bit - R16_SINT" },            // [38] : 59
    { DXGI_FORMAT_R8_UNORM              , "8 bit - R8_UNORM" },             // [39] : 61
    { DXGI_FORMAT_R8_UINT               , "8 bit - R8_UINT" },              // [40] : 62
    { DXGI_FORMAT_R8_SNORM              , "8 bit - R8_SNORM" },             // [41] : 63
    { DXGI_FORMAT_R8_SINT               , "8 bit - R8_SINT" },              // [42] : 64
    { DXGI_FORMAT_A8_UNORM              , "8 bit - A8_UNORM" },             // [43] : 65
    { DXGI_FORMAT_R1_UNORM              , "1 bit - R1_UNORM" },             // [44] : 66
    { DXGI_FORMAT_R9G9B9E5_SHAREDEXP    , "32 bit - R9G9B9E5_SHADEREXP" },  // [45] : 67
    { DXGI_FORMAT_BC1_UNORM             , "BC - BC1_UNORM" },               // [46] : 71
    { DXGI_FORMAT_BC1_UNORM_SRGB        , "BC - BC1_UNORM_SRGB" },          // [47] : 72
    { DXGI_FORMAT_BC2_UNORM             , "BC - BC2_UNORM" },               // [48] : 74
    { DXGI_FORMAT_BC2_UNORM_SRGB        , "BC - BC2_UNORM_SRGB" },          // [49] : 75
    { DXGI_FORMAT_BC3_UNORM             , "BC - BC3_UNORM" },               // [50] : 77
    { DXGI_FORMAT_BC3_UNORM_SRGB        , "BC - BC3_UNORM_SRGB" },          // [51] : 78
    { DXGI_FORMAT_BC4_UNORM             , "BC - BC4_UNORM" },               // [52] : 80
    { DXGI_FORMAT_BC4_SNORM             , "BC - BC4_SNORM" },               // [53] : 81
    { DXGI_FORMAT_BC5_UNORM             , "BC - BC5_UNORM" },               // [54] : 83
    { DXGI_FORMAT_BC5_SNORM             , "BC - BC5_SNORM" },               // [55] : 84
    { DXGI_FORMAT_BC6H_UF16             , "BC - BC6H_UF16" },               // [56] : 95
    { DXGI_FORMAT_BC6H_SF16             , "BC - BC6H_SF16" },               // [57] : 96
    { DXGI_FORMAT_BC7_UNORM             , "BC - BC7_UNORM" },               // [58] : 98
    { DXGI_FORMAT_BC7_UNORM_SRGB        , "BC - BC7_UNORM_SRGB" },          // [59] : 99
    { DXGI_FORMAT_B5G6R5_UNORM          , "16 bit - B5G6R5_UNORM" },        // [60] : 85
    { DXGI_FORMAT_B5G5R5A1_UNORM        , "16 bit - B5G5R5A1_UNORM" },      // [61] : 86
    { DXGI_FORMAT_B8G8R8A8_UNORM        , "32 bit - B8G8R8A8_UNORM" },      // [62] : 87
    { DXGI_FORMAT_B8G8R8A8_UNORM_SRGB   , "32 bit - B8G8R8A8_UNORM_SRGB" }, // [63] : 91
    { DXGI_FORMAT_B8G8R8X8_UNORM        , "32 bit - B8G8R8X8_UNORM" },      // [64] : 88
    { DXGI_FORMAT_B8G8R8X8_UNORM_SRGB   , "32 bit - B8G8R8X8_UNORM_SRGB" }, // [65] : 93
    { DXGI_FORMAT_B4G4R4A4_UNORM        , "16 bit - B4G4R4A4_UNORM" },      // [66] : 115
    { DXGI_FORMAT_A4B4G4R4_UNORM        , "16 bit - A4B4G4R4_UNORM" },      // [67] : 191
};

} // namespace

//-----------------------------------------------------------------------------
//      TEX_DIMENSIONを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(DirectX::TEX_DIMENSION dim)
{
    switch(dim)
    {
    case DirectX::TEX_DIMENSION_TEXTURE1D: { return "Texture1D"; }
    case DirectX::TEX_DIMENSION_TEXTURE2D: { return "Texture2D"; }
    case DirectX::TEX_DIMENSION_TEXTURE3D: { return "Texture3D"; }
    default: { return "Unknown"; }
    }
}

//-----------------------------------------------------------------------------
//      フォーマットを列挙します.
//-----------------------------------------------------------------------------
bool EnumrateFormat(void* data, int index, const char** result)
{
    if (index < 0)
        return false;

    if (index >= _countof(g_FormatTable))
        return false;

    *result = g_FormatTable[index].Text;
    return true;
}

//-----------------------------------------------------------------------------
//      フォーマット数を取得します.
//-----------------------------------------------------------------------------
int GetFormatCount()
{ return (int)_countof(g_FormatTable); }

//-----------------------------------------------------------------------------
//      DXGIフォーマットを取得します.
//-----------------------------------------------------------------------------
DXGI_FORMAT GetDXGIFormat(int combBoxIndex)
{ return g_FormatTable[combBoxIndex].Format; }
