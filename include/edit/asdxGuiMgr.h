﻿//-----------------------------------------------------------------------------
// File : asdxGuiMgr.h
// Desc : GUI Manager.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

#ifdef ASDX_ENABLE_IMGUI
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include <chrono>
#include <gfx/asdxVertexBuffer.h>
#include <gfx/asdxIndexBuffer.h>
#include <gfx/asdxConstantBuffer.h>
#include <gfx/asdxTexture.h>
#include <asdxResourceUploader.h>


//-----------------------------------------------------------------------------
// Forward Declarations.
//-----------------------------------------------------------------------------
struct ImDrawData;


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// GuiMgr class
///////////////////////////////////////////////////////////////////////////////
class GuiMgr
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    friend void RenderImGui(ImDrawData* pDrawData);

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    static constexpr uint32_t   MaxPrimitiveCount = 6 * 1024;   //!< 最大プリミティブ数.

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      シングルトンインスタンスを取得します.
    //!
    //! @return     シングルトンインスタンスを返却します.
    //-------------------------------------------------------------------------
    static GuiMgr& Instance();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      hWnd        ウィンドウハンドルです.
    //! @param[in]      width       ウィンドウの横幅です.
    //! @param[in]      height      ウィンドウの縦幅です.
    //! @param[in]      format      出力ターゲットのフォーマットです.
    //! @param[in]      fontPath    フォントファイルパスです.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(
        HWND                hWnd,
        uint32_t            width,
        uint32_t            height,
        DXGI_FORMAT         format,
        const char*         fontPath);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      更新処理を行います.
    //!
    //! @param[in]      width       ウィンドウの横幅です.
    //! @param[in]      height      ウィンドウの縦幅です.
    //-------------------------------------------------------------------------
    void Update(uint32_t width, uint32_t height);

    //-------------------------------------------------------------------------
    //! @brief      描画処理を行います.
    //-------------------------------------------------------------------------
    void Draw(ID3D12GraphicsCommandList* pCmdList);

    //-------------------------------------------------------------------------
    //! @brief      マウス処理です.
    //-------------------------------------------------------------------------
    void OnMouse(int x, int y, int wheelDelta, bool isDownL, bool isDownM, bool isDownR);

    //-------------------------------------------------------------------------
    //! @brief      キー処理です.
    //-------------------------------------------------------------------------
    void OnKey(bool isDown, bool isAltDown, uint32_t code);

    //-------------------------------------------------------------------------
    //! @brief      タイピング処理です.
    //-------------------------------------------------------------------------
    void OnTyping(uint32_t code);

private:
    //=========================================================================
    // private varaibles.
    //=========================================================================
    static GuiMgr                           s_Instance;
    VertexBuffer                            m_VB;
    IndexBuffer                             m_IB;
    ConstantBuffer                          m_CB;
    RefPtr<ID3D12RootSignature>             m_RootSig;
    RefPtr<ID3D12PipelineState>             m_PSO;
    Texture                                 m_FontTexture;
    ID3D12GraphicsCommandList*              m_pCmdList;
    std::chrono::system_clock::time_point   m_LastTime;
    uint32_t                                m_SizeVB;
    uint32_t                                m_SizeIB;

    //=========================================================================
    // private mehods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    GuiMgr();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~GuiMgr();

    //-------------------------------------------------------------------------
    //! @brief      内部処理用描画関数です.
    //-------------------------------------------------------------------------
    void OnDraw(ImDrawData* pData);
};

} // namespace asdx

#endif//ASDX_ENABLE_IMGUI
