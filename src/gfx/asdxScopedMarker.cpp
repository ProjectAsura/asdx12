//-----------------------------------------------------------------------------
// File : asdxScopedMarker.cpp
// Desc : Scoped Marker.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <d3d12.h>
#include <d3d12video.h>
#include <pix3.h>
#include <gfx/asdxScopedMarker.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ScopedMaker class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ScopedMarker::ScopedMarker(ID3D12GraphicsCommandList* pCmd, [[maybe_unused]] const char* text)
: m_pCmd(pCmd)
{
    assert(m_pCmd != nullptr);
    assert(text != nullptr);
    PIXBeginEvent(m_pCmd, PIX_COLOR_DEFAULT, text);
}

//------------------------------------------------------------------------------
//      デストラクタです.
//------------------------------------------------------------------------------
ScopedMarker::~ScopedMarker()
{
    assert(m_pCmd != nullptr);
    PIXEndEvent(m_pCmd);
    m_pCmd = nullptr;
}

} // namespace asdx
