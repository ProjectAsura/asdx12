//-----------------------------------------------------------------------------
// File : asdxTaaRenderer.h
// Desc : Temporal Anti-Aliasing Renderer
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMath.h>
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
    //! @brief      ピクセルシェーダ用初期化処理を行います.
    //! 
    //! @param[in]      format      出力レンダーターゲットのフォーマット.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool InitPS(DXGI_FORMAT format);

    //-------------------------------------------------------------------------
    //! @brief      コンピュートシェーダ用初期化処理を行います.
    //-------------------------------------------------------------------------
    bool InitCS();

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      ピクセルシェーダを用いて描画処理を行います.
    //! 
    //! @param[in]      pCmdList            コマンドリストです.
    //! @param[in]      pColorRTV           出力カラーレンダーターゲットビューです.
    //! @param[in]      pHistoryRTV         出力ヒストリーレンダーターゲットビューです.
    //! @param[in]      pCurrentColorMap    カレントカラーマップです.
    //! @param[in]      pHistoryColorMap    ヒストリーカラーマップです.
    //! @param[in]      pVelocityMap        速度マップです.
    //! @param[in]      pDepthMap           深度マップです.
    //! @param[in]      gamma               ガンマ.
    //! @param[in]      alpha               ブレンド値です.
    //! @param[in]      jitter              現在フレーム用のジッタ―値です.
    //-------------------------------------------------------------------------
    void RenderPS(
        ID3D12GraphicsCommandList*  pCmdList,
        const IRenderTargetView*    pColorRTV,
        const IRenderTargetView*    pHistoryRTV,
        const IShaderResourceView*  pCurrentColorMap,
        const IShaderResourceView*  pHistoryColorMap,
        const IShaderResourceView*  pVelocityMap,
        const IShaderResourceView*  pDepthMap,
        float                       gamma,
        float                       alpha,
        const asdx::Vector2&        jitter);

    //-------------------------------------------------------------------------
    //! @brief      コンピュートシェーダを用いて描画処理を行います.
    //! 
    //! @param[in]      pCmdList                コマンドリストです.
    //! @param[in]      pColorUAV               出力カラーアンオーダードアクセスビューです.
    //! @param[in]      pHistoryUAV             出力ヒストリーアンオーダードアクセスビューです.
    //! @param[in]      pCurrentColorMap        カレントカラーマップです.
    //! @param[in]      pHistoryColorMap        ヒストリーカラーマップです.
    //! @param[in]      pVelocityMap            速度マップです.
    //! @param[in]      pDepthMap               深度マップです.
    //! @param[in]      gamma                   ガンマ.
    //! @param[in]      alpha                   ブレンド値です.
    //! @param[in]      jitter                  現在フレーム用のジッタ―値です.
    //-------------------------------------------------------------------------
    void RenderCS(
        ID3D12GraphicsCommandList*  pCmdList,
        const IUnorderedAccessView* pColorUAV,
        const IUnorderedAccessView* pHistoryUVA,
        const IShaderResourceView*  pCurrentColorMap,
        const IShaderResourceView*  pHistoryColorMap,
        const IShaderResourceView*  pVelocityMap,
        const IShaderResourceView*  pDepthMap,
        float                       gamma,
        float                       alpha,
        const asdx::Vector2&        jitter);

    //-------------------------------------------------------------------------
    //! @brief      ジッタ―値を計算します.
    //! 
    //! @param[in]      index           フレームインデックス.
    //! @param[in]      w               レンダーターゲットの横幅.
    //! @param[in]      h               レンダーターゲットの縦幅.
    //-------------------------------------------------------------------------
    static asdx::Vector2 CalcJitter(uint32_t index, uint32_t w, uint32_t h);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    RootSignature   m_RootSigPS;
    RootSignature   m_RootSigCS;
    PipelineState   m_PipelineStatePS;
    PipelineState   m_PipelineStateCS;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx
