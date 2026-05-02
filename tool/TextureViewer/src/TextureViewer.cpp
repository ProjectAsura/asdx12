//-----------------------------------------------------------------------------
// File : TextureViewer.cpp
// Desc : Texture Viewer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <TextureViewer.h>
#include <TextureViewerUtil.h>
#include <TextureConverter.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxFileIO.h>
#include <fnd/asdxMisc.h>
#include <fnd/asdxPath.h>
#include <edit/asdxGuiMgr.h>
#include <res/asdxResTexture.h>
#include <gfx/asdxGfxMisc.h>
#include <imgui.h>


namespace {

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
static const uint32_t kMaxSpriteCount = 4096;
static const uint32_t kMaxBatchCount  = 16;

//-----------------------------------------------------------------------------
// Shaders
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/Sprite1dPS.inc"
#include "../res/shaders/Compiled/Sprite1dArrayPS.inc"
#include "../res/shaders/Compiled/Sprite2dPS.inc"
#include "../res/shaders/Compiled/Sprite2dArrayPS.inc"
#include "../res/shaders/Compiled/SpriteCubePS.inc"
#include "../res/shaders/Compiled/SpriteCubeArrayPS.inc"
#include "../res/shaders/Compiled/Sprite3dPS.inc"

const char* kCubeFace[] = {
    ASDX_U8("PositiveX (X+)"),
    ASDX_U8("NegativeX (X-)"),
    ASDX_U8("PositiveY (Y+)"),
    ASDX_U8("NegativeY (Y-)"),
    ASDX_U8("PositionZ (Z+)"),
    ASDX_U8("NegativeZ (Z-)"),
};

const char* kNormalMapChannel[] = {
    ASDX_U8("赤成分"),
    ASDX_U8("緑成分"),
    ASDX_U8("青成分"),
    ASDX_U8("アルファ"),
    ASDX_U8("輝度"),
};

} // namespace


///////////////////////////////////////////////////////////////////////////////
// TextureViewer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
TextureViewer::TextureViewer()
: asdx::App(L"TexxtureViewer", 1920, 1080, nullptr, nullptr, nullptr)
{
    m_SwapChainFormat    = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    m_DepthStencilFormat = DXGI_FORMAT_D32_FLOAT;

    m_ClearColor[0] = 0.1f;
    m_ClearColor[1] = 0.1f;
    m_ClearColor[2] = 0.1f;
    m_ClearColor[3] = 1.0f;

    m_DeviceDesc.MaxShaderResourceCount = 8192;
    m_DeviceDesc.MaxSamplerCount        = 128;
    m_DeviceDesc.MaxColorTargetCount    = 256;
    m_DeviceDesc.MaxDepthTargetCount    = 256;

#if ASDX_DEBUG
    m_DeviceDesc.EnableDebug = true;
#endif
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
TextureViewer::~TextureViewer()
{
}

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool TextureViewer::OnInit()
{
    auto pDevice = asdx::GetD3D12Device();

    if (!asdx::InitAsyncFileIO())
    {
        ELOG("Error : InitFileLoader() Failed.");
        return false;
    }

    m_GfxCmdList.Reset();
    auto pCmd = m_GfxCmdList.GetD3D12CommandList();

    {
        if (!asdx::GuiMgr::Instance().Init(pCmd, m_hWnd, m_Width, m_Height, m_SwapChainFormat, nullptr))
        {
            ELOG("Error : GuiMgr::Init() Failed.");
            return false;
        }
    }

    // スプライトレンダラー初期化.
    {
        if (!m_SpriteRenderer.Init(m_Width, m_Height, kMaxSpriteCount, kMaxBatchCount, m_SwapChainFormat, m_DepthStencilFormat))
        {
            ELOG("Error : SpriteRenderer::Init() Failed.");
            return false;
        }
    }

    // ポイントサンプラー初期化.
    {
        auto desc = asdx::Sampler::PointClamp;
        if (!m_PointClamp.Init(&desc))
        {
            ELOG("Error : PointClamp Init Failed.");
            return false;
        }
    }

    // リニアサンプラー初期化.
    {
        auto desc = asdx::Sampler::LinearClamp;
        if (!m_LinearClamp.Init(&desc))
        {
            ELOG("Error : LinearClamp Init Failed.");
            return false;
        }
    }

    {
        D3D12_SHADER_BYTECODE ps = { Sprite1dPS, sizeof(Sprite1dPS) };
        if (!m_SpriteRenderer.CreatePipelineState(pDevice, ps, true, m_Pso1d.GetAddress()))
        {
            ELOG("Error : SpriteRenderer::CreatePipelineState() Failed.");
            return false;
        }
    }

    {
        D3D12_SHADER_BYTECODE ps = { Sprite1dArrayPS, sizeof(Sprite1dArrayPS) };
        if (!m_SpriteRenderer.CreatePipelineState(pDevice, ps, true, m_Pso1dArray.GetAddress()))
        {
            ELOG("Error : SpriteRenderer::CreatePipelineState() Failed.");
            return false;
        }
    }

    {
        D3D12_SHADER_BYTECODE ps = { Sprite2dPS, sizeof(Sprite2dPS) };
        if (!m_SpriteRenderer.CreatePipelineState(pDevice, ps, true, m_Pso2d.GetAddress()))
        {
            ELOG("Error : SpriteRenderer::CreatePipelineState() Failed.");
            return false;
        }
    }

    {
        D3D12_SHADER_BYTECODE ps = { Sprite2dArrayPS, sizeof(Sprite2dArrayPS) };
        if (!m_SpriteRenderer.CreatePipelineState(pDevice, ps, true, m_Pso2dArray.GetAddress()))
        {
            ELOG("Error : SpriteRenderer::CreatePipelineState() Failed.");
            return false;
        }
    }

    {
        D3D12_SHADER_BYTECODE ps = { SpriteCubePS, sizeof(SpriteCubePS) };
        if (!m_SpriteRenderer.CreatePipelineState(pDevice, ps, true, m_PsoCube.GetAddress()))
        {
            ELOG("Error : SpriteRenderer::CreatePipelineState() Failed.");
            return false;
        }
    }

    {
        D3D12_SHADER_BYTECODE ps = { SpriteCubeArrayPS, sizeof(SpriteCubeArrayPS) };
        if (!m_SpriteRenderer.CreatePipelineState(pDevice, ps, true, m_PsoCubeArray.GetAddress()))
        {
            ELOG("Error : SpriteRenderer::CreatePipelineState() Failed.");
            return false;
        }
    }

    {
        D3D12_SHADER_BYTECODE ps = { Sprite3dPS, sizeof(Sprite3dPS) };
        if (!m_SpriteRenderer.CreatePipelineState(pDevice, ps, true, m_Pso3d.GetAddress()))
        {
            ELOG("Error : SpriteRenderer::CreatePipelineState() Failed.");
            return false;
        }
    }


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
//      終了処理です.
//-----------------------------------------------------------------------------
void TextureViewer::OnTerm()
{
    m_Pso1d         .Reset();
    m_Pso1dArray    .Reset();
    m_Pso2d         .Reset();
    m_Pso2dArray    .Reset();
    m_PsoCube       .Reset();
    m_PsoCubeArray  .Reset();
    m_Pso3d         .Reset();

    m_PointClamp    .Term();
    m_LinearClamp   .Term();
    m_SpriteRenderer.Term();
    if (m_Texture)
    {
        m_Texture->Release();
        m_Texture = nullptr;
    }

    m_ScratchImage.Release();

    asdx::TermAsyncFileIO();
    asdx::GuiMgr::Instance().Term();
}

//-----------------------------------------------------------------------------
//      フレーム遷移処理です.
//-----------------------------------------------------------------------------
void TextureViewer::OnFrameMove(const asdx::App::FrameEventArgs& args)
{
    auto pCmd = m_GfxCmdList.Reset();

    m_SpriteRenderer.Reset();
    m_SpriteRenderer.SetScreenSize(m_Width, m_Height);

    // ファイルドロップされたものを処理.
    if (m_DirtyScratchImage)
    {
        RecreateTexture(pCmd);
        m_DirtyScratchImage = false;
    }

    asdx::GuiMgr::Instance().Update(m_Width, m_Height);

    // 情報表示.
    {
        const auto w = 250.0f;
        const auto h = 165.0f;
        const auto x = 10.0f;
        const auto y = 10.0f;

        ImGui::SetNextWindowPos(ImVec2(m_Width - (w + x), m_Height - (h + y)));
        ImGui::SetNextWindowSize(ImVec2(w, h));

        auto flags = ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoTitleBar;
        if (ImGui::Begin(ASDX_U8("Info"), nullptr, flags))
        {
            ImGui::Text(ASDX_U8("FPS : %.2f"), GetFPS());
            ImGui::Separator();

            const auto& meta = m_ScratchImage.GetMetadata();

            ImGui::BeginTable(ASDX_U8("TextureStatus"), 2);
            ImGui::TableSetupColumn("##row0", ImGuiTableColumnFlags_WidthFixed);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(ASDX_U8("Dimension"));
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(ASDX_U8("%s"), ToString(meta.dimension));

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(ASDX_U8("Width"));
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(ASDX_U8("%zu"), meta.width);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(ASDX_U8("Height"));
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(ASDX_U8("%zu"), meta.height);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(ASDX_U8("Depth"));
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(ASDX_U8("%zu"), meta.depth);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(ASDX_U8("ArraySize"));
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(ASDX_U8("%zu"), meta.arraySize);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(ASDX_U8("MipLevels"));
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(ASDX_U8("%zu"), meta.mipLevels);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(ASDX_U8("Format"));
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(ASDX_U8("%s"), asdx::ToShortString(meta.format));

            ImGui::EndTable();

            ImGui::End();
        }
    }

    // コンテキストメニュー.
    {
        if (ImGui::IsMouseClicked(1))
        { ImGui::OpenPopup(ASDX_U8("ContextMenu")); }

        if (ImGui::BeginPopup(ASDX_U8("ContextMenu")))
        {
            if (ImGui::BeginMenu(ASDX_U8("ファイル")))
            {
                MenuFile(pCmd);
                ImGui::EndMenu();
            }
            // テクスチャが読み込まれているときのみ.
            if (m_ScratchImage.GetImageCount() > 0)
            {
                if (ImGui::BeginMenu(ASDX_U8("フォーマット")))
                {
                    MenuFormat(pCmd);
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu(ASDX_U8("表示")))
                {
                    MenuView();
                    ImGui::EndMenu();
                }
            }
            if (ImGui::BeginMenu(ASDX_U8("ヘルプ")))
            {
                MenuHelp();
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }
    }

    DrawLisence();

    if (m_OpenConvert)
    {
        ImGui::OpenPopup(ASDX_U8("FormatConversion"));
        m_OpenConvert = false;
    }

    if (m_OpenResize)
    { 
        ImGui::OpenPopup(ASDX_U8("Resize"));
        m_OpenResize = false;
    }

    if (m_OpenNormalMap)
    {
        ImGui::OpenPopup(ASDX_U8("NormalMap"));
        m_OpenNormalMap = false;
    }

    if (m_OpenPremultipliedAlpha)
    {
        ImGui::OpenPopup(ASDX_U8("PremultipliedAlpha"));
        m_OpenPremultipliedAlpha = false;
    }

    auto doConvert              = false;
    auto doResize               = false;
    auto doPremultipliedAlpha   = false;
    auto doNormalMap            = false;

    if (ImGui::BeginPopupModal(ASDX_U8("FormatConversion")))
    {
        ImGui::Text(ASDX_U8("変換前フォーマット : %s"), asdx::ToString(m_ScratchImage.GetMetadata().format));
        ImGui::Combo(ASDX_U8("変換後フォーマット"), &m_FormatIndex, EnumrateFormat, nullptr, GetFormatCount());

        auto close = false;
        if (ImGui::Button(ASDX_U8("キャンセル")))
        {
            close = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(ASDX_U8("OK")))
        {
            doConvert = true;
            close = true;
        }

        if (close)
        { ImGui::CloseCurrentPopup(); }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal(ASDX_U8("Resize")))
    {
        int w = int(m_ResizedWidth);
        int h = int(m_ResizedHeight);

        if (ImGui::InputInt(ASDX_U8("横幅"), &w))
        { m_ResizedWidth = size_t(w); }

        if (ImGui::InputInt(ASDX_U8("高さ"), &h))
        { m_ResizedHeight = size_t(h); }

        auto close = false;
        if (ImGui::Button(ASDX_U8("キャンセル")))
        {
            close = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(ASDX_U8("OK")))
        {
            doResize = true;
            close = true;
        }

        if (close)
        { ImGui::CloseCurrentPopup(); }

        ImGui::EndPopup();
    }

    // 法線マップ変換設定ダイアログ.
    if (ImGui::BeginPopupModal(ASDX_U8("NormalMap")))
    {
        int channel = m_NormalMapFlags & 0xF;
        ImGui::Combo(ASDX_U8("チャンネル"), &channel, kNormalMapChannel, int(_countof(kNormalMapChannel)));

        bool mirrorU = !!(m_NormalMapFlags & DirectX::CNMAP_MIRROR_U);
        bool mirrorV = !!(m_NormalMapFlags & DirectX::CNMAP_MIRROR_V);
        bool invertSign = !!(m_NormalMapFlags & DirectX::CNMAP_INVERT_SIGN);

        int format = m_NormalMapFormat;
        ImGui::Checkbox(ASDX_U8("ミラーU"), &mirrorU);
        ImGui::Checkbox(ASDX_U8("ミラーV"), &mirrorV);
        ImGui::Checkbox(ASDX_U8("符号を反転"), &invertSign);
        ImGui::DragFloat(ASDX_U8("強さ"), &m_NormalMapAmplitude, 0.1f, 0.0f, 100.0f);
        ImGui::Combo(ASDX_U8("フォーマット"), &format, EnumrateFormat, nullptr, GetFormatCount());
        m_NormalMapFormat = DXGI_FORMAT(format);

        uint32_t flags = 0;
        flags = uint32_t(channel);
        if (mirrorU)
            flags |= DirectX::CNMAP_MIRROR_U;
        if (mirrorV)
            flags |= DirectX::CNMAP_MIRROR_V;
        if (invertSign)
            flags |= DirectX::CNMAP_INVERT_SIGN;

        m_NormalMapFlags = flags;

        auto close = false;
        if (ImGui::Button(ASDX_U8("キャンセル")))
        {
            close = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(ASDX_U8("OK")))
        {
            close = true;
            doNormalMap = true;
        }
        if (close)
        { ImGui::CloseCurrentPopup(); }

        ImGui::EndPopup();
    }

    // 事前乗算済みアルファ設定ダイアログ.
    if (ImGui::BeginPopupModal(ASDX_U8("PremultipliedAlpha")))
    {
        auto ignoreSRGB = !!(m_PremultipliedAlphaFlags & DirectX::TEX_PMALPHA_IGNORE_SRGB);
        auto reverse    = !!(m_PremultipliedAlphaFlags & DirectX::TEX_PMALPHA_REVERSE);
      
        ImGui::Checkbox(ASDX_U8("sRGB変換を無視"), &ignoreSRGB);
        ImGui::Checkbox(ASDX_U8("逆変換"), &reverse);

        uint32_t flags = 0;
        if (ignoreSRGB)
            flags |= DirectX::TEX_PMALPHA_IGNORE_SRGB;
        if (reverse)
            flags |= DirectX::TEX_PMALPHA_REVERSE;

        m_PremultipliedAlphaFlags = flags;

        auto close = false;
        if (ImGui::Button(ASDX_U8("キャンセル")))
        {
            close = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(ASDX_U8("OK")))
        {
            close = true;
            doPremultipliedAlpha = true;
        }
        if (close)
        { ImGui::CloseCurrentPopup(); }

        ImGui::EndPopup();
    }

    // フォーマット変換処理.
    if (doConvert)
    {
        auto nextFormat = GetDXGIFormat(m_FormatIndex);
        auto prevFormat = m_ScratchImage.GetMetadata().format;

        // 同じフォーマットの場合は変換せずに終了.
        if (prevFormat == nextFormat)
            return;

        DirectX::ScratchImage scratchImage;
        HRESULT hr = S_OK;

        // 非圧縮 ---> 圧縮.
        if (!asdx::IsCompressed(prevFormat) && asdx::IsCompressed(nextFormat))
        {
            hr = DirectX::Compress(
                m_ScratchImage.GetImages(), m_ScratchImage.GetImageCount(), m_ScratchImage.GetMetadata(),
                nextFormat,
                DirectX::TEX_COMPRESS_PARALLEL,
                DirectX::TEX_THRESHOLD_DEFAULT,
                scratchImage);
        }
        // 圧縮 ---> 圧縮.
        else if (asdx::IsCompressed(prevFormat) && asdx::IsCompressed(nextFormat))
        {
            hr = DirectX::Decompress(
                m_ScratchImage.GetImages(), m_ScratchImage.GetImageCount(), m_ScratchImage.GetMetadata(),
                DXGI_FORMAT_UNKNOWN,
                scratchImage);
            if (SUCCEEDED(hr))
            {
                DirectX::ScratchImage compressImage;
                hr = DirectX::Compress(
                    scratchImage.GetImages(), scratchImage.GetImageCount(), scratchImage.GetMetadata(),
                    nextFormat,
                    DirectX::TEX_COMPRESS_PARALLEL,
                    DirectX::TEX_THRESHOLD_DEFAULT,
                    compressImage);
                if (SUCCEEDED(hr))
                {
                    scratchImage = std::move(compressImage);
                }
            }
        }
        // 圧縮 ---> 非圧縮.
        else if (asdx::IsCompressed(prevFormat) && !asdx::IsCompressed(nextFormat))
        {
            hr = DirectX::Decompress(
                m_ScratchImage.GetImages(), m_ScratchImage.GetImageCount(), m_ScratchImage.GetMetadata(),
                DXGI_FORMAT_UNKNOWN,
                scratchImage);
            if (SUCCEEDED(hr))
            {
                DirectX::ScratchImage convImage;
                hr = DirectX::Convert(
                    scratchImage.GetImages(), scratchImage.GetImageCount(), scratchImage.GetMetadata(),
                    nextFormat,
                    DirectX::TEX_FILTER_DEFAULT,
                    DirectX::TEX_THRESHOLD_DEFAULT,
                    convImage);
                if (SUCCEEDED(hr))
                {
                    scratchImage = std::move(convImage);
                }
            }
        }
        else // 非圧縮 ---> 非圧縮
        {
            hr = DirectX::Convert(
                m_ScratchImage.GetImages(), m_ScratchImage.GetImageCount(), m_ScratchImage.GetMetadata(),
                nextFormat,
                DirectX::TEX_FILTER_DEFAULT,
                DirectX::TEX_THRESHOLD_DEFAULT,
                scratchImage);
        }

        if (FAILED(hr))
        {
            char msg[2048];
            sprintf_s(msg, "フォーマット変換に失敗しました.\nエラーコード(0x%x)", hr);
            asdx::ErrorDlg("Format Convert Failed.", msg);
            ELOGA("Error : %s", msg);
        }

        if (SUCCEEDED(hr))
        {
            m_ScratchImage = std::move(scratchImage);
            RecreateTexture(pCmd);
        }
    }

    // リサイズ処理.
    if (doResize)
    {
        // サイズが同じ場合は処理しない.
        if (m_ResizedWidth  == m_ScratchImage.GetMetadata().width
         && m_ResizedHeight == m_ScratchImage.GetMetadata().height)
            return;

        DirectX::ScratchImage scratchImage;
        auto hr = DirectX::Resize(m_ScratchImage.GetImages(), m_ScratchImage.GetImageCount(), m_ScratchImage.GetMetadata(),
            m_ResizedWidth, 
            m_ResizedHeight,
            DirectX::TEX_FILTER_DEFAULT,
            scratchImage);

        // リサイズに成功したら差し替える.
        if (SUCCEEDED(hr))
        {
            m_ScratchImage = std::move(scratchImage);
            RecreateTexture(pCmd);
        }
        else
        {
            ELOGA("Error : DirectX::Resize() Failed. errcode = 0x%x", hr);
        }
    }

    // 事前乗算済みアルファ処理.
    if (doPremultipliedAlpha)
    {
        DirectX::ScratchImage scratchImage;
        auto hr = DirectX::PremultiplyAlpha(
            m_ScratchImage.GetImages(),
            m_ScratchImage.GetImageCount(),
            m_ScratchImage.GetMetadata(),
            DirectX::TEX_PMALPHA_FLAGS(m_PremultipliedAlphaFlags),
            scratchImage);
        if (SUCCEEDED(hr))
        {
            m_ScratchImage = std::move(scratchImage);
            RecreateTexture(pCmd);
        }
        else
        {
            ELOGA("Error : DirectX::PremultiplyAlpha() Failed. errcode = 0x%x", hr);
        }
    }

    // 法線マップ計算処理.
    if (doNormalMap)
    {
        DirectX::ScratchImage scratchImage;
        auto hr = DirectX::ComputeNormalMap(
            m_ScratchImage.GetImages(),
            m_ScratchImage.GetImageCount(),
            m_ScratchImage.GetMetadata(),
            DirectX::CNMAP_FLAGS(m_NormalMapFlags),
            m_NormalMapAmplitude,
            m_NormalMapFormat,
            scratchImage);
        if (SUCCEEDED(hr))
        {
            m_ScratchImage = std::move(scratchImage);
            RecreateTexture(pCmd);
        }
        else
        {
            ELOGA("Error : DirectX::ComputeNormalMap() Failed. errcode = 0x%x", hr);
        }
    }
}

//-----------------------------------------------------------------------------
//      フレーム描画処理です.
//-----------------------------------------------------------------------------
void TextureViewer::OnFrameRender(const asdx::App::FrameEventArgs& args)
{
    auto idx = GetCurrentBackBufferIndex();

    auto pCmd = m_GfxCmdList.GetD3D12CommandList();

    // 書き込み状態に遷移.
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource    = m_ColorTarget[idx].GetResource();
        barrier.Transition.StateBefore  = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter   = D3D12_RESOURCE_STATE_RENDER_TARGET;
        pCmd->ResourceBarrier(1, &barrier);
    }

    // レンダーターゲット設定.
    auto handleRTV = m_ColorTarget[idx].GetCpuHandleRTV();
    auto handleDSV = m_DepthTarget.GetCpuHandleDSV();
    pCmd->OMSetRenderTargets(1, &handleRTV, FALSE, &handleDSV);
    pCmd->ClearRenderTargetView(handleRTV, m_ClearColor, 0, nullptr);
    pCmd->ClearDepthStencilView(handleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    pCmd->RSSetViewports(1, &m_Viewport);
    pCmd->RSSetScissorRects(1, &m_ScissorRect);

    // TODO : CubeMap, 3D Texture 対応.

    // テクスチャを描画.
    if (m_Texture && m_Texture->GetHandleGPU().ptr != 0)
    {
        auto& meta = m_ScratchImage.GetMetadata();

        auto w = (m_Width  < meta.width)  ? m_Width  : meta.width;
        auto h = (m_Height < meta.height) ? m_Height : meta.height;

        pCmd->SetGraphicsRootSignature(m_SpriteRenderer.GetRootSignature());

        ID3D12PipelineState* pPSO = nullptr;

        if (meta.dimension == DirectX::TEX_DIMENSION_TEXTURE1D)
        {
            if (meta.arraySize > 1)
            { pPSO = m_Pso1dArray.GetPtr(); }
            else
            { pPSO = m_Pso1d.GetPtr(); }
        }
        else if (meta.dimension == DirectX::TEX_DIMENSION_TEXTURE2D)
        {
            if (!!(meta.miscFlags & DirectX::TEX_MISC_TEXTURECUBE))
            {
                if (meta.arraySize > 1)
                { pPSO = m_PsoCubeArray.GetPtr(); }
                else 
                { pPSO = m_PsoCube.GetPtr(); }
            }
            else if (meta.arraySize > 1)
            { pPSO = m_Pso2dArray.GetPtr(); }
            else
            { pPSO = m_Pso2d.GetPtr(); }
        }
        else if (meta.dimension == DirectX::TEX_DIMENSION_TEXTURE3D)
        { pPSO = m_Pso3d.GetPtr(); }
        assert(pPSO != nullptr);

        struct Param
        {
            float   MipLevel   = 0.0f;
            float   ArrayIndex = 0.0f;
            float   FaceIndex  = 0.0f;
        };
        Param param = {};
        param.MipLevel   = float(m_MipLevel);
        param.ArrayIndex = float(m_ArrayIndex);
        param.FaceIndex  = float(m_FaceIndex);

        m_SpriteRenderer.SetParam(3, &param, 0);
        m_SpriteRenderer.ChangeBatch(pPSO, m_Texture->GetHandleGPU(), m_PointClamp.GetHandleGPU());
        m_SpriteRenderer.Add(0, 0, int(w), int(h));
    }
    m_SpriteRenderer.Draw(pCmd);

    // GUIを描画.
    asdx::GuiMgr::Instance().Draw(pCmd);

    // 表示状態に遷移.
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

    Present(1);
}

//-----------------------------------------------------------------------------
//      リサイズ処理です.
//-----------------------------------------------------------------------------
void TextureViewer::OnResize(const asdx::App::ResizeEventArgs& args)
{
}

//-----------------------------------------------------------------------------
//      キー処理です.
//-----------------------------------------------------------------------------
void TextureViewer::OnKey(const asdx::App::KeyEventArgs& args)
{
    asdx::GuiMgr::Instance().OnKey(args.KeyCode, args.IsKeyDown, args.IsAltDown);
}

//-----------------------------------------------------------------------------
//      マウス処理です.
//-----------------------------------------------------------------------------
void TextureViewer::OnMouse(const asdx::App::MouseEventArgs& args)
{
    asdx::GuiMgr::Instance().OnMouse(args.X, args.Y, args.WheelDelta, args.IsDownL, args.IsDownM, args.IsDownR);
}

//-----------------------------------------------------------------------------
//      タイピング処理です.
//-----------------------------------------------------------------------------
void TextureViewer::OnTyping(uint32_t keyCode)
{
    asdx::GuiMgr::Instance().OnTyping(keyCode);
}

//-----------------------------------------------------------------------------
//      ファイルドロップ時の処理.
//-----------------------------------------------------------------------------
void TextureViewer::OnDrop(const wchar_t** dropFiles, uint32_t fileCount)
{
    if (fileCount == 0 || dropFiles == nullptr)
        return;

    // 最初の1つだけ処理する.
    m_DirtyScratchImage = LoadScratchImage(dropFiles[0]);
}

//-----------------------------------------------------------------------------
//      ファイルメニューの処理.
//-----------------------------------------------------------------------------
void TextureViewer::MenuFile(ID3D12GraphicsCommandList* pCmd)
{
    if (ImGui::MenuItem(ASDX_U8("ファイルを開く")))
    {
        const char* filter = 
            "テクスチャファイル(*.txb, *.dds, *.tga, *.hdr, *.bmp, *.jpg, *.jpeg, *.png, *.tif, *.tiff, *.gif, *.hdp)\0*.txb;*.dds;*.tga;*.hdr;*.bmp;*.jpg;*.jpeg;*.png;*.tif;*.tiff;*.gif;*.hdp;*.jxr;*.wdp;*.heif;*.heic\0"
            "Project Asura Texture Binary (*.txb)\0*.txb\0"
            "Direct Draw Surface (*.dds)\0*.dds\0"
            "Truevision Graphics Adapter (*.tga)\0*.tga\0"
            "Radiance HDR (*.hdr)\0*.hdr\0"
            "Windows Bitmap (*.bmp)\0*.bmp\0"
            "Joint Photographics Expers Group (*.jpg, *.jpeg)\0*.jpg;*.jpeg\0"
            "Portable Network Graphic (*.png)\0*.png\0"
            "Tagged Image File Format (*.tif, *.tiff)\0*.tif;*.tiff\0"
            "Graphics Interchanged Format (*.gif)\0*.gif\0" 
            "HD Photo (*.hdp)\0*.hdp\0"
            "Window Media Photo (*.wdp)\0*.wdp\0"
            "JPEG XR (*.jxr)\0*.jxr\0"
            "High Efficiency Image File (*.heif, *.heic)\0*.heif;*.heic\0"
            "全てのファイル (*.*)\0*.*\0\0";

        asdx::fs::path path = m_InputPath;
        if (asdx::OpenFileDlg(filter, path))
        {
            auto wpath = path.wstring();
            if (LoadScratchImage(wpath.c_str()))
            { RecreateTexture(pCmd); }
        }
    }
    if (ImGui::MenuItem(ASDX_U8("名前を付けて保存")))
    {
        const char* filter = 
            "Project Asura Texture Binary(*.txb)\0*.txb\0"
            "Direct Draw Surface (*.dds)\0*.dds\0"
            "Truevision Graphics Adapter (*.tga)\0*.tga\0"
            "Radiance HDR (*.hdr)\0*.hdr\0"
            "Window Bitmap (*.bmp)\0*.bmp\0"
            "Portable Network Graphics (*.png)\0*.png\0"
            "Tagged Image File Format (*.tif)\0*.tif\0"
            "HD Photo (*.hdp)\0*.hdp\0"
            "JPEG XR (*.jxr)\0*.jxr\0"
            "Window Media Photo (*.wdp)\0*.wpd\0"
            "High Efficiency Image File (*.heif)\0*.heif\0\0";

        asdx::fs::path path = m_OutputPath;
        if (asdx::SaveFileDlg(filter, path))
        {
            m_OutputPath = path.string();
            auto wpath = path.wstring();
            SaveScratchImage(wpath.c_str());
        }
    }
    if (ImGui::MenuItem(ASDX_U8("上書き保存")))
    {
        auto wpath = asdx::ToStringW(m_OutputPath);
        SaveScratchImage(wpath.c_str());
    }
}

//-----------------------------------------------------------------------------
//      フォーマットメニューの処理.
//-----------------------------------------------------------------------------
void TextureViewer::MenuFormat(ID3D12GraphicsCommandList* pCmd)
{
    assert(m_ScratchImage.GetImageCount() > 0);

    const auto& meta = m_ScratchImage.GetMetadata();

    if (meta.mipLevels <= 1)
    {
        if (ImGui::MenuItem(ASDX_U8("ミップマップ生成")))
        {
            DirectX::ScratchImage scratchImage;

            HRESULT hr = S_OK;
            if (meta.dimension == DirectX::TEX_DIMENSION_TEXTURE3D)
            {
                hr = DirectX::GenerateMipMaps3D(
                    m_ScratchImage.GetImages(), m_ScratchImage.GetImageCount(), meta,
                    DirectX::TEX_FILTER_CUBIC, 0, scratchImage);

            }
            else
            {
                hr = DirectX::GenerateMipMaps(
                    m_ScratchImage.GetImages(), m_ScratchImage.GetImageCount(), meta,
                    DirectX::TEX_FILTER_CUBIC, 0, scratchImage);
            }

            if (SUCCEEDED(hr))
            {
                m_ScratchImage = std::move(scratchImage);
                RecreateTexture(pCmd);
            }
        }
    }
    if (ImGui::MenuItem(ASDX_U8("フォーマット変換")))
    {
        m_OpenConvert = true;
        m_FormatIndex = GetFormatIndex(m_ScratchImage.GetMetadata().format);
    }
    if (ImGui::MenuItem(ASDX_U8("テクスチャをリサイズ")))
    {
        m_OpenResize = true;
    }
    if (ImGui::MenuItem(ASDX_U8("事前乗算済みアルファ生成")))
    {
        m_OpenPremultipliedAlpha = true;
    }
    if (ImGui::MenuItem(ASDX_U8("法線マップ生成")))
    {
        m_OpenNormalMap = true;
    }
    if (ImGui::BeginMenu(ASDX_U8("回転/フリップ")))
    {
        auto flags = DirectX::TEX_FR_ROTATE0;

        if (ImGui::MenuItem(ASDX_U8("90度回転")))
            flags = DirectX::TEX_FR_ROTATE90;
        else if (ImGui::MenuItem(ASDX_U8("180度回転")))
            flags = DirectX::TEX_FR_ROTATE180;
        else if (ImGui::MenuItem(ASDX_U8("270度回転")))
            flags = DirectX::TEX_FR_ROTATE270;
        if (ImGui::MenuItem(ASDX_U8("水平方向に反転")))
            flags = DirectX::TEX_FR_FLIP_HORIZONTAL;
        else if (ImGui::MenuItem(ASDX_U8("垂直方向に反転")))
            flags = DirectX::TEX_FR_FLIP_VERTICAL;

        if (flags != DirectX::TEX_FR_ROTATE0)
        {
            DirectX::ScratchImage scratchImage;

            auto hr = DirectX::FlipRotate(
                m_ScratchImage.GetImages(), m_ScratchImage.GetImageCount(), meta,
                flags, scratchImage);
            if (SUCCEEDED(hr))
            {
                m_ScratchImage = std::move(scratchImage);
                RecreateTexture(pCmd);
            }
        }

        ImGui::EndMenu();
    }
}

//-----------------------------------------------------------------------------
//      表示メニューの処理.
//-----------------------------------------------------------------------------
void TextureViewer::MenuView()
{
    assert(m_ScratchImage.GetImageCount() > 0);
    const auto& meta = m_ScratchImage.GetMetadata();

    if (meta.mipLevels > 1)
    {
        if (ImGui::BeginMenu(ASDX_U8("ミップレベル")))
        {
            ImGui::DragInt(ASDX_U8("表示ミップ番号"), &m_MipLevel, 1, 0, int(meta.mipLevels - 1));
            ImGui::EndMenu();
        }
    }

    if (meta.dimension == DirectX::TEX_DIMENSION_TEXTURE2D 
      && !!(meta.miscFlags & DirectX::TEX_MISC_TEXTURECUBE))
    {
        if (ImGui::BeginMenu(ASDX_U8("キューブマップ")))
        {
            ImGui::Combo(ASDX_U8("表示面"), &m_FaceIndex, kCubeFace, int(_countof(kCubeFace)));
            ImGui::EndMenu();
        }

        auto arraySize = meta.arraySize / 6;
        if (arraySize > 1)
        {
            if (ImGui::BeginMenu(ASDX_U8("テクスチャ配列")))
            {
                ImGui::DragInt(ASDX_U8("表示配列番号"), &m_ArrayIndex, 1, 0, int(arraySize - 1));
                ImGui::EndMenu();
            }
        }
    }
    else
    {
        if (meta.arraySize > 1)
        {
            if (ImGui::BeginMenu(ASDX_U8("テクスチャ配列")))
            {
                ImGui::DragInt(ASDX_U8("表示配列番号"), &m_ArrayIndex, 1, 0, int(meta.arraySize - 1));
                ImGui::EndMenu();
            }
        }
    }
}

//-----------------------------------------------------------------------------
//      ヘルプメニューの処理.
//-----------------------------------------------------------------------------
void TextureViewer::MenuHelp()
{
    if (ImGui::MenuItem(ASDX_U8("バージョン情報")))
    {
        asdx::InfoDlg("Version Info",
            "TextureViewer ver 0.1\n"
            "Build 0.1\n"
            "Copyright(c) Project Asura.");
    }
    if (ImGui::MenuItem(ASDX_U8("ライセンス情報")))
    {
        m_ShowLisence = true;
    }
}

//-----------------------------------------------------------------------------
//      ライセンス情報を描画します.
//-----------------------------------------------------------------------------
void TextureViewer::DrawLisence()
{
    if (m_ShowLisence)
    {
        ImGui::OpenPopup("Lisence");
        m_ShowLisence = false;
    }

    if (ImGui::BeginPopupModal("Lisence"))
    {
        ImGui::Text("Dear ImGui");
        ImGui::TextLinkOpenURL("https://github.com/ocornut/imgui/blob/master/LICENSE.txt", "https://github.com/ocornut/imgui/blob/master/LICENSE.txt");
        ImGui::NewLine();

        ImGui::Text("D3D12MemoryAllocator");
        ImGui::TextLinkOpenURL("https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator/blob/master/LICENSE.txt", "https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator/blob/master/LICENSE.txt");
        ImGui::NewLine();

        ImGui::Text("xxHash");
        ImGui::TextLinkOpenURL("https://github.com/Cyan4973/xxHash/blob/dev/LICENSE", "https://github.com/Cyan4973/xxHash/blob/dev/LICENSE");
        ImGui::NewLine();

        ImGui::Text("flatbuffers");
        ImGui::TextLinkOpenURL("https://github.com/google/flatbuffers/blob/master/LICENSE", "https://github.com/google/flatbuffers/blob/master/LICENSE");
        ImGui::NewLine();

        if (ImGui::Button("Close"))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
}

//-----------------------------------------------------------------------------
//      テクスチャを再生成します.
//-----------------------------------------------------------------------------
void TextureViewer::RecreateTexture(ID3D12GraphicsCommandList* pCmd)
{
    std::vector<uint8_t> blob;

    // asdx形式に変換.
    if (!TextureConverter::Convert(m_ScratchImage, blob))
    {
        ELOG("Error : TexutreConverter::Convret() Failed.");
        return;
    }

    // テクスチャを遅延解放.
    if (m_Texture != nullptr)
    {
        m_Texture->Release();
        m_Texture = nullptr;
    }

    // テクスチャバイナリをロード.
    asdx::TextureBinary bin;
    bin.Load(std::move(blob));

    // リソースを取得.
    asdx::ResTexture res = bin.GetResource();

    // テクスチャ初期化.
    if (!asdx::Texture::Create(res, &m_Texture))
    {
        ELOG("Error : Texture::Init() Failed.");
    }
}

//-----------------------------------------------------------------------------
//      テクスチャバイナリを保存します.
//-----------------------------------------------------------------------------
void TextureViewer::SaveTextureBinary(const char* path)
{
    if (m_ScratchImage.GetImageCount() == 0 || path == nullptr)
        return;

    std::vector<uint8_t> blob;
    if (!TextureConverter::Convert(m_ScratchImage, blob))
    {
        ELOG("Error : TextureConverter::Convert() Failed.");
        return;
    }

    if (!asdx::SaveA(path, blob))
    {
        ELOG("Error : SaveA() Failed. path = %s", path);
        return;
    }

    ILOG("Info : TextureBinary Output success. path = %s", path);
}

//-----------------------------------------------------------------------------
//      スクラッチイメージをロードします.
//-----------------------------------------------------------------------------
bool TextureViewer::LoadScratchImage(const wchar_t* path)
{
    // テクスチャを読み込む.
    auto ext = asdx::fs::path(path).extension().wstring();

    HRESULT hr = S_OK;
    DirectX::ScratchImage scratchImage;
    if (ext == L".txb")
    {
        std::vector<uint8_t> blob;
        if (!asdx::LoadW(path, blob))
        {
            ELOG("Error : LoadW() File Failed. path = %ls", path);
            return false;
        }

        if (!TextureConverter::ReverseConvert(blob, scratchImage))
        {
            ELOG("Error : TextureConverter::ReverseConvert() Failed. path = %ls", path);
            return false;
        }

        hr = S_OK;
    }
    else if (ext == L".dds")
    {
        hr = DirectX::LoadFromDDSFile(path, DirectX::DDS_FLAGS_NONE, nullptr, scratchImage);
    }
    else if (ext == L".tga")
    {
        hr = DirectX::LoadFromTGAFile(path, DirectX::TGA_FLAGS_NONE, nullptr, scratchImage);
    }
    else if (ext == L".hdr")
    {
        hr = DirectX::LoadFromHDRFile(path, nullptr, scratchImage);
    }
    else if (ext == L".bmp" || ext == L".jpg" || ext == L".jpeg" || ext == L".png" || ext == L".tif" 
          || ext == L".tiff" || ext == L".gif" || ext == L".hdp" || ext == L".wdp" || ext == L".jxr"
          || ext == L".heif" || ext == L".heic")
    {
        hr = DirectX::LoadFromWICFile(path, DirectX::WIC_FLAGS_NONE, nullptr, scratchImage);
    }
    else
    {
        hr = E_FAIL;
        ELOG("Error : Unsupported File Extension (%s)", ext.c_str());
    }

    if (SUCCEEDED(hr))
    {
        m_ScratchImage  = std::move(scratchImage);
        m_ResizedWidth  = m_ScratchImage.GetMetadata().width;
        m_ResizedHeight = m_ScratchImage.GetMetadata().height;

        auto fullPath = asdx::ToFullPath(path);
        m_InputPath     = fullPath.string();
        m_OutputPath    = fullPath.replace_extension(".txb").string();

        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
//      スクラッチイメージをロードします.
//-----------------------------------------------------------------------------
bool TextureViewer::SaveScratchImage(const wchar_t* path)
{
    // テクスチャを読み込む.
    auto ext = asdx::fs::path(path).extension().wstring();

    HRESULT hr = S_OK;
    DirectX::ScratchImage scratchImage;
    if (ext == L".txb")
    {
        std::vector<uint8_t> blob;
        if (!TextureConverter::Convert(m_ScratchImage, blob))
        {
            ELOG("Error : TextureConverter::Convert() Failed.");
            return false;
        }

        if (!asdx::SaveW(path, blob))
        {
            ELOG("Error : SaveW() failed. path = %ls", path);
            return false;
        }

        hr = S_OK;
    }
    else if (ext == L".dds")
    {
        hr = DirectX::SaveToDDSFile(
            m_ScratchImage.GetImages(),
            m_ScratchImage.GetImageCount(),
            m_ScratchImage.GetMetadata(),
            DirectX::DDS_FLAGS_NONE,
            path);
    }
    else if (ext == L".tga")
    {
        assert(m_ScratchImage.GetImageCount() > 0);
        auto images = m_ScratchImage.GetImages();
        auto meta = m_ScratchImage.GetMetadata();
        hr = DirectX::SaveToTGAFile(
            images[0],
            DirectX::TGA_FLAGS_NONE,
            path,
            &meta);
    }
    else if (ext == L".hdr")
    {
        assert(m_ScratchImage.GetImageCount() > 0);
        auto images = m_ScratchImage.GetImages();
        hr = DirectX::SaveToHDRFile(
            images[0],
            path);
    }
    else if (ext == L".bmp")
    {
        hr = DirectX::SaveToWICFile(
            m_ScratchImage.GetImages(),
            m_ScratchImage.GetImageCount(),
            DirectX::WIC_FLAGS_NONE,
            DirectX::GetWICCodec(DirectX::WIC_CODEC_BMP),
            path);
    }
    else if (ext == L".jpg")
    {
        hr = DirectX::SaveToWICFile(
            m_ScratchImage.GetImages(),
            m_ScratchImage.GetImageCount(),
            DirectX::WIC_FLAGS_NONE,
            DirectX::GetWICCodec(DirectX::WIC_CODEC_JPEG),
            path);
    }
    else if (ext == L".png")
    {
        hr = DirectX::SaveToWICFile(
            m_ScratchImage.GetImages(),
            m_ScratchImage.GetImageCount(),
            DirectX::WIC_FLAGS_NONE,
            DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG),
            path);
    }
    else if (ext == L".tif")
    {
        hr = DirectX::SaveToWICFile(
            m_ScratchImage.GetImages(),
            m_ScratchImage.GetImageCount(),
            DirectX::WIC_FLAGS_NONE,
            DirectX::GetWICCodec(DirectX::WIC_CODEC_TIFF),
            path);
    }
    else if (ext == L".hdp" || ext == L".jxr" || ext == L".wdp")
    {
        hr = DirectX::SaveToWICFile(
            m_ScratchImage.GetImages(),
            m_ScratchImage.GetImageCount(),
            DirectX::WIC_FLAGS_NONE,
            DirectX::GetWICCodec(DirectX::WIC_CODEC_WMP),
            path);
    }
    else if (ext == L".heif" || ext == L".heic")
    {
        hr = DirectX::SaveToWICFile(
            m_ScratchImage.GetImages(),
            m_ScratchImage.GetImageCount(),
            DirectX::WIC_FLAGS_NONE,
            DirectX::GetWICCodec(DirectX::WIC_CODEC_HEIF),
            path);
    }
    else
    {
        hr = E_FAIL;
        ELOG("Error : Unsupported File Extension (%s)", ext.c_str());
    }

    if (SUCCEEDED(hr))
    {
        ILOG("Info : Texture Output success. path = %s", path);
        return true;
    }

    return false;
}