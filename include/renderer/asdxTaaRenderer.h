//-----------------------------------------------------------------------------
// File : asdxTaaRenderer.h
// Desc : Temporal Anti-Aliasing Renderer
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxView.h>
#include <gfx/asdxRootSignature.h>
#include <gfx/asdxPipelineState.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// TaaRenderer class
///////////////////////////////////////////////////////////////////////////////
class TaaRenderer
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
    //! @brief      初期化処理を行います.
    //! 
    //! @param[in]      format      出力レンダーターゲットのフォーマット.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(DXGI_FORMAT format);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      描画処理を行います.
    //! 
    //! @param[in]      pCmdList            コマンドリストです.
    //! @param[in]      pRenderTargetView   レンダーターゲットビューです.
    //! @param[in]      pCurrentColorMap    カレントカラーマップです.
    //! @param[in]      pHistoryColorMap    ヒストリーカラーマップです.
    //! @param[in]      pVelocityMap        速度マップです.
    //! @param[in]      pDepthMap           深度マップです.
    //! @param[in]      gamma               ガンマ.
    //! @param[in]      alpha               ブレンド値です.
    //-------------------------------------------------------------------------
    void Render(
        ID3D12GraphicsCommandList*  pCmdList,
        const IRenderTargetView*    pRenderTargetView,
        const IShaderResourceView*  pCurrentColorMap,
        const IShaderResourceView*  pHistoryColorMap,
        const IShaderResourceView*  pVelocityMap,
        const IShaderResourceView*  pDepthMap,
        float                       gamma,
        float                       alpha);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    RootSignature   m_RootSig;
    PipelineState   m_PipelineState;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx
