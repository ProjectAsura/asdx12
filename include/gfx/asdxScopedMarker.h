//-----------------------------------------------------------------------------
// File : asdxScopedMarker.h
// Desc : Scoped Marker.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <d3d12.h>

#if ASDX_ENABLE_PIX3
    #include <d3d12video.h>
    #include <pix3.h>
#else
    #include <pix.h>
#endif// ASDX_ENABLE_PIX3


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ScopedMarker class
///////////////////////////////////////////////////////////////////////////////
class ScopedMarker
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //! 
    //! @param[in]      pCmd        グラフィックスコマンドリストです.
    //! @param[in]      text        マーカーに表示するテキストです.
    //-------------------------------------------------------------------------
    ScopedMarker(ID3D12GraphicsCommandList* pCmd, const char* text)
    : m_pCmd(pCmd)
    {
        assert(m_pCmd != nullptr);
        assert(text != nullptr);
        PIXBeginEvent(m_pCmd, PIX_COLOR_DEFAULT, text);
    }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~ScopedMarker()
    {
        assert(m_pCmd != nullptr);
        PIXEndEvent(m_pCmd);
        m_pCmd = nullptr;
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    ID3D12GraphicsCommandList* m_pCmd = nullptr;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx

#if defined(DEBUG) || defined(_DEBUG)
    #define ASDX_SCOPED_MARKER(pCmd, Tag) asdx::ScopedMarker marker_##Tag(pCmd, #Tag)
#else
    #define ASDX_SCOPED_MARKER(pCmd, Tag)
#endif// defined(DEBUG) || defined(_DEBUG)

