//-----------------------------------------------------------------------------
// File : SampleApp.cpp
// Desc : Sample Application.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <SampleApp.h>
#include <fnd/asdxMisc.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxFileIO.h>
#include <edit/asdxGuiMgr.h>

#define TEST (1)
#if TEST
#include <gfx/asdxSprite.h>
#include <res/asdxResTexture.h>
#include <gfx/asdxTexture.h>
#include <gfx/asdxSampler.h>
#include <gfx/asdxFont.h>
#include <gfx/asdxSpriteAnimation.h>
#include <gfx/asdxFade.h>
#include "../external/ImGuiRingMenu/ImGuiRingMenu.h"
#endif

namespace {

#if TEST
asdx::SpriteRenderer g_SpriteRenderer;
asdx::TextureBinary g_TextureBinary;
asdx::Texture       g_Texture;
asdx::Sampler       g_Sampler;
asdx::Font          g_Font;
asdx::TimerSpriteAnimation g_AirShipAnim;
ImGuiRingMenu      g_TestMenu;
#endif

} // namespace

///////////////////////////////////////////////////////////////////////////////
// SampleApp class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
SampleApp::SampleApp()
: asdx::App(L"Sample", 1920, 1080, nullptr, nullptr, nullptr)
{
    m_SwapChainFormat    = DXGI_FORMAT_R8G8B8A8_UNORM;
    m_DepthStencilFormat = DXGI_FORMAT_D32_FLOAT;

    m_DeviceDesc.MaxShaderResourceCount = 8192;
    m_DeviceDesc.MaxSamplerCount        = 128;
    m_DeviceDesc.MaxColorTargetCount    = 256;
    m_DeviceDesc.MaxDepthTargetCount    = 256;

#if ASDX_DEBUG
    m_DeviceDesc.EnableDebug   = true;
    m_DeviceDesc.EnableCapture = true;
#endif
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
SampleApp::~SampleApp()
{
}

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool SampleApp::OnInit()
{
    auto pDevice = asdx::GetD3D12Device();

    #if ASDX_ENABLE_SOUND
    {
        // サウンドマネージャの初期化.
        asdx::InitSoundMgr(reinterpret_cast<uintptr_t>(m_hWnd));
    }
    #endif

    // コマンドリストをリセット.
    m_GfxCmdList.Reset();
    auto pCmd = m_GfxCmdList.GetD3D12CommandList();

    #if ASDX_ENABLE_IMGUI
    // GUI初期化.
    {
        if (!asdx::GuiMgr::Instance().Init(pCmd, m_hWnd, m_Width, m_Height, m_SwapChainFormat))
        {
            ELOGA("Error : GuiMgr::Init() Failed.");
            return false;
        }
    }
    #endif

#if TEST
    {
        std::vector<uint8_t> texBin;
        if (!asdx::LoadA("../res/texture/air_ship.txb", texBin))
        {
            ELOG("Texture Load Failed.");
            return false;
        }

        g_TextureBinary.Load(std::move(texBin));
    }

    auto res = g_TextureBinary.GetResource();
    if (!g_Texture.Init(pCmd, res))
    {
        ELOG("Texture::Init() Failed.");
        return false;
    }

    if (!g_SpriteRenderer.Init(m_Width, m_Height, 512, 16, m_SwapChainFormat, m_DepthStencilFormat))
    {
        ELOG("Error : SpriteRenderer::Init() Failed.");
        return false;
    }

    {
        auto desc = asdx::Sampler::PointClamp;
        if (!g_Sampler.Init(&desc))
        {
            ELOG("Error : Sampler::Init() Failed.");
            return false;
        }
    }

    {
        std::vector<uint8_t> fontBin;
        if (!asdx::LoadA("../res/font/yasashisa_gothic.fnb", fontBin))
        {
            ELOG("Error : Font::Init Failed.");
            return false;
        }

        g_Font.Init(pCmd, std::move(fontBin));
    }

    if (!asdx::FontRenderer::Instance().Init(g_SpriteRenderer))
    {
        ELOG("Error : FontRenderer::Init() Failed.");
        return false;
    }

    {
        asdx::Vector2 oneSize(16.0f / 64.0f, 16.0f / 64.0f);

        std::vector<asdx::SpriteAnimation::Frame> frames{
            { asdx::Vector2(0.0f, 40.0f/64.0f), asdx::Vector2(oneSize.x, 40.0f / 64.0f + oneSize.y) },
            { asdx::Vector2(20.0f / 64.0f, 20.0f / 64.0f), asdx::Vector2(20.0f / 64.0f + oneSize.x, 20.0f / 64.0f + oneSize.y) },
        };

        g_AirShipAnim.Init(128, 128, 0.025f, frames);
    }

    if (!asdx::Fade::Instance().Init(pCmd, m_SwapChainFormat))
    {
        ELOG("Error : Fade::Init() Failed.");
        return false;
    }

    asdx::Fade::Instance().SetColor1(asdx::Vector4(1.0f, 0.0f, 0.0f, 1.0f));
    asdx::Fade::Instance().SetChangeSec(3.0f);
    asdx::Fade::Instance().SetEnablePulse(true);
    asdx::Fade::Instance().SetPulseSpeed(4.0f);

    g_TestMenu.Add({0, "TestA"});
    g_TestMenu.Add({0, "TestB"});
    g_TestMenu.Add({0, "TestC"});
    g_TestMenu.Add({0, "TestD"});
    g_TestMenu.Add({0, "TestE"});
    g_TestMenu.Add({0, "TestF"});
    g_TestMenu.Add({0, "TestG"});
    g_TestMenu.Add({0, "TestH"});
    g_TestMenu.Add({0, "TestI"});
    g_TestMenu.Add({0, "TestJ"});
    g_TestMenu.Add({0, "TestK"});
    g_TestMenu.Add({0, "TestL"});
    g_TestMenu.Add({0, "TestM"});
    g_TestMenu.Add({0, "TestN"});
    g_TestMenu.Add({0, "TestO"});
    g_TestMenu.Add({0, "IIIIHISTHEIATestP"});

    //g_TestMenu.StartEnter();
#endif

    // コマンドの記録を終了.
    pCmd->Close();

    // セットアップコマンド実行.
    {
        ID3D12CommandList* pCmds[] = { pCmd };

        // グラフィックスキューを取得.
        auto pGraphicsQueue = asdx::GetGraphicsQueue();

        // コマンドを実行.
        pGraphicsQueue->Execute(_countof(pCmds), pCmds);

        // 待機点を発行.
        m_FrameWaitPoint = pGraphicsQueue->Signal();

        // 完了を待機.
        pGraphicsQueue->Sync(m_FrameWaitPoint);
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了時の処理です.
//-----------------------------------------------------------------------------
void SampleApp::OnTerm()
{
    // TODO : Implementation.
    {
    }

    #if ASDX_ENABLE_IMGUI
    {
        // GUI終了処理.
        asdx::GuiMgr::Instance().Term();
    }
    #endif
}

//-----------------------------------------------------------------------------
//      フレーム遷移時の処理です.
//-----------------------------------------------------------------------------
void SampleApp::OnFrameMove(const asdx::App::FrameEventArgs& args)
{

    #if ASDX_ENABLE_IMGUI
    {
        // ImGuiフレーム開始処理.
        asdx::GuiMgr::Instance().Update(m_Width, m_Height);
    }
    #endif

    #if ASDX_ENABLE_SOUND
    {
        // サウンドマネージャの終了処理.
        asdx::TermSoundMgr();
    }
    #endif

}

//-----------------------------------------------------------------------------
//      フレーム描画時の処理です.
//-----------------------------------------------------------------------------
void SampleApp::OnFrameRender(const asdx::App::FrameEventArgs& args)
{
    auto idx = GetCurrentBackBufferIndex();

#if TEST
    g_SpriteRenderer.Reset();
    g_TestMenu.Update(float(args.ElapsedTimeSec));
    int selectedId = 0;
    g_TestMenu.Draw(selectedId);
#endif

    // コマンド記録開始.
    m_GfxCmdList.Reset();
    auto pCmd = m_GfxCmdList.GetD3D12CommandList();

    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource    = m_ColorTarget[idx].GetResource();
        barrier.Transition.StateBefore  = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter   = D3D12_RESOURCE_STATE_RENDER_TARGET;
        pCmd->ResourceBarrier(1, &barrier);
    }

    auto handleRTV = m_ColorTarget[idx].GetCpuHandleRTV();
    auto handleDSV = m_DepthTarget.GetCpuHandleDSV();
    pCmd->OMSetRenderTargets(1, &handleRTV, FALSE, &handleDSV);
    pCmd->ClearRenderTargetView(handleRTV, m_ClearColor, 0, nullptr);
    pCmd->ClearDepthStencilView(handleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    pCmd->RSSetViewports(1, &m_Viewport);
    pCmd->RSSetScissorRects(1, &m_ScissorRect);

    // TODO : 描画処理.
    {

#if TEST
        g_AirShipAnim.Update(float(args.ElapsedTimeSec));

        g_SpriteRenderer.SetPipelineState(pCmd);
        g_SpriteRenderer.SetTexture(g_Texture.GetGpuHandleSRV(), g_Sampler.GetGpuHandle());
        //g_AirShipAnim.Add(g_SpriteRenderer, 100, 256);
        ////g_SpriteRenderer.Add( 10, 10, 64, 64 );
        //g_SpriteRenderer.Draw(pCmd);

        asdx::FontRenderer::Instance().SetEnableOuter(true);
        asdx::FontRenderer::Instance().SetEnableOffset(true);
        asdx::FontRenderer::Instance().SetOuterColor(1.0f, 0.0f, 0.0f, 1.0f);
        asdx::FontRenderer::Instance().SetOuterOffset(-1.0f, -1.0f);
        asdx::FontRenderer::Instance().SetState(pCmd, g_SpriteRenderer, g_Font);
        asdx::FontRenderer::Instance().SetScale(2.0f);
        asdx::FontRenderer::Instance().Add(g_SpriteRenderer, g_Font, 10, 74, u8"てすとですよ!テスト!");
        asdx::FontRenderer::Instance().AddFormat(g_SpriteRenderer, g_Font, 10, 142, "FPS : %f", args.FPS);

        g_SpriteRenderer.Draw(pCmd);

        //asdx::Fade::Instance().Update(float(args.ElapsedTimeSec));
        //asdx::Fade::Instance().Draw(pCmd);
#endif
    }


    #if ASDX_ENABLE_IMGUI
    {
        // ImGui描画処理.
        asdx::GuiMgr::Instance().Draw(pCmd);
    }
    #endif

    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource    = m_ColorTarget[idx].GetResource();
        barrier.Transition.StateBefore  = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter   = D3D12_RESOURCE_STATE_PRESENT;
        pCmd->ResourceBarrier(1, &barrier);
    }

    // コマンド記録終了.
    pCmd->Close();

    // コマンドを実行.
    {
        ID3D12CommandList* pCmds[] = { pCmd };

        auto pGraphicsQueue = asdx::GetGraphicsQueue();

        // 前フレームの描画の完了を待機.
        if (m_FrameWaitPoint.IsValid())
        { pGraphicsQueue->Sync(m_FrameWaitPoint); }

        // コマンドを実行.
        pGraphicsQueue->Execute(_countof(pCmds), pCmds);

        // 待機点を発行.
        m_FrameWaitPoint = pGraphicsQueue->Signal();
    }

    // 画面に表示.
    Present(1);

    // フレーム同期.
    asdx::FrameSync();
}

//-----------------------------------------------------------------------------
//      リサイズ時の処理です.
//-----------------------------------------------------------------------------
void SampleApp::OnResize(const asdx::App::ResizeEventArgs& args)
{
}

//-----------------------------------------------------------------------------
//      キー処理です.
//-----------------------------------------------------------------------------
void SampleApp::OnKey(const asdx::App::KeyEventArgs& args)
{
    m_Camera.OnKey(args.KeyCode, args.IsKeyDown, args.IsAltDown);
    #if ASDX_ENABLE_IMGUI
    {
        // ImGuiのキー処理.
        asdx::GuiMgr::Instance().OnKey(args.KeyCode, args.IsKeyDown, args.IsAltDown);
    }
    #endif

#if 1
    if (args.IsKeyDown)
    {
        if (args.KeyCode == 'R')
        {
            asdx::Fade::Instance().ResetState();
        }
    }

#endif
}

//-----------------------------------------------------------------------------
//      マウス処理です.
//-----------------------------------------------------------------------------
void SampleApp::OnMouse(const asdx::App::MouseEventArgs& args)
{
    if(args.IsAltDown)
    {
        // カメラのマウス処理.
        m_Camera.OnMouse(
            args.X,
            args.Y,
            args.WheelDelta,
            args.IsDownL,
            args.IsDownR,
            args.IsDownM,
            args.IsDownX1,
            args.IsDownX2);
    }
    else
    {
        #if ASDX_ENABLE_IMGUI
        {
            // ImGuiのマウス処理.
            asdx::GuiMgr::Instance().OnMouse(
                args.X,
                args.Y,
                args.WheelDelta,
                args.IsDownL,
                args.IsDownM,
                args.IsDownR);
        }
        #endif
    }
}

//-----------------------------------------------------------------------------
//      タイピング処理です.
//-----------------------------------------------------------------------------
void SampleApp::OnTyping(uint32_t keyCode)
{
    #if ASDX_ENABLE_IMGUI
    {
        // タイピング時の処理です.
        asdx::GuiMgr::Instance().OnTyping(keyCode);
    }
    #endif
}

//-----------------------------------------------------------------------------
//      メッセージプロシージャ時の処理です.
//-----------------------------------------------------------------------------
void SampleApp::OnMsgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    ASDX_UNUSED(hWnd);
    ASDX_UNUSED(wp);
    ASDX_UNUSED(lp);

    switch(msg)
    {
    case MM_MCINOTIFY:
        {
        #if ASDX_ENABLE_SOUND
            // サウンドマネージャのコールバック.
            asdx::OnSoundMsg(uint32_t(lp), uint32_t(wp));
        #endif//ASDX_ENABLE_SOUND
        }
        break;

    default:
        break;
    }
}