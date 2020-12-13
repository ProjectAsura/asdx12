//-----------------------------------------------------------------------------
// File : asdxColorSpace.h
// Desc : Color Space
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// COLOR_SPACE enum
///////////////////////////////////////////////////////////////////////////////
enum COLOR_SPACE
{
    COLOR_SPACE_NONE,           // �f�t�H���g.
    COLOR_SPACE_SRGB,           // SRGB (�K���}2.2)
    COLOR_SPACE_BT709,          // ITU-R BT.709 (�K���}2.4)
    COLOR_SPACE_BT2100_PQ,      // ITU-R BT.2100 Perceptual Quantizer
    COLOR_SPACE_BT2100_HLG,     // ITU-R BT.2100 Hybrid Log Gamma
};

} // namespace asdx