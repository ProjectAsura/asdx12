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
#include <edit/asdxGuiMgr.h>
#include <res/asdxResTexture.h>
#include <gfx/asdxGfxMisc.h>
#include <imgui.h>


namespace {

static const uint32_t kMaxSpriteCount = 4096;
static const uint32_t kMaxBatchCount  = 16;

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

    m_ClearColor[0] = 0.2f;
    m_ClearColor[1] = 0.2f;
    m_ClearColor[2] = 0.2f;
    m_ClearColor[3] = 1.0f;

    m_DeviceDesc.MaxShaderResourceCount = 8192;
    m_DeviceDesc.MaxSamplerCount = 128;
    m_DeviceDesc.MaxColorTargetCount = 256;
    m_DeviceDesc.MaxDepthTargetCount = 256;

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
        const auto path = "../res/font/07やさしさゴシック.ttf";
        if (!asdx::GuiMgr::Instance().Init(pCmd, m_hWnd, m_Width, m_Height, m_SwapChainFormat, path))
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
    m_PointClamp.Term();
    m_LinearClamp.Term();
    m_SpriteRenderer.Term();
    m_Texture.Term();

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
        const auto w = 350.0f;
        const auto h = 150.0f;
        const auto x = 10.0f;
        const auto y = 10.0f;

        ImGui::SetNextWindowPos(ImVec2(m_Width - (w + x), m_Height - (h + y)));
        ImGui::SetNextWindowSize(ImVec2(w, h));

        auto flags = ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoTitleBar;
        if (ImGui::Begin(u8"Info", nullptr, flags))
        {
            ImGui::Text(u8"FPS : %.2f", GetFPS());
            ImGui::Separator();

            const auto& meta = m_ScratchImage.GetMetadata();
   
            ImGui::Text(u8"Dimension : %s", ToString(meta.dimension));
            ImGui::Text(u8"Width     : %zu", meta.width);
            ImGui::Text(u8"Height    : %zu", meta.height);
            ImGui::Text(u8"Depth     : %zu", meta.depth);
            ImGui::Text(u8"ArraySize : %zu", meta.arraySize);
            ImGui::Text(u8"MipLevels : %zu", meta.mipLevels);
            ImGui::Text(u8"Format    : %s", asdx::ToString(meta.format));

            ImGui::End();
        }
    }

    // コンテキストメニュー.
    {
        if (ImGui::IsMouseClicked(1))
        { ImGui::OpenPopup(u8"ContextMenu"); }

        if (ImGui::BeginPopup(u8"ContextMenu"))
        {
            if (ImGui::BeginMenu(u8"ファイル"))
            {
                MenuFile(pCmd);
                ImGui::EndMenu();
            }
            // テクスチャが読み込まれているときのみ.
            if (m_ScratchImage.GetImageCount() > 0)
            {
                if (ImGui::BeginMenu(u8"フォーマット"))
                {
                    MenuFormat(pCmd);
                    ImGui::EndMenu();
                }
            }
            if (ImGui::BeginMenu(u8"ヘルプ"))
            {
                MenuHelp();
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }
    }

    if (m_OpenConvert)
    {
        ImGui::OpenPopup(u8"FormatConversion");
        m_OpenConvert = false;
    }

    if (m_OpenResize)
    { 
        ImGui::OpenPopup(u8"Resize");
        m_OpenResize = false;
    }

    auto doConvert = false;
    auto doResize  = false;

    if (ImGui::BeginPopupModal(u8"FormatConversion"))
    {
        ImGui::Text(u8"変換前フォーマット : %s", asdx::ToString(m_ScratchImage.GetMetadata().format));
        ImGui::Combo(u8"変換後フォーマット", &m_FormatIndex, EnumrateFormat, nullptr, GetFormatCount());

        auto close = false;
        if (ImGui::Button(u8"キャンセル"))
        {
            close = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(u8"OK"))
        {
            doConvert = true;
            close = true;
        }

        if (close)
        { ImGui::CloseCurrentPopup(); }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal(u8"Resize"))
    {
        int w = int(m_ResizedWidth);
        int h = int(m_ResizedHeight);

        if (ImGui::InputInt(u8"横幅", &w))
        { m_ResizedWidth = size_t(w); }

        if (ImGui::InputInt(u8"高さ", &h))
        { m_ResizedHeight = size_t(h); }

        auto close = false;
        if (ImGui::Button(u8"キャンセル"))
        {
            close = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(u8"OK"))
        {
            doResize = true;
            close = true;
        }

        if (close)
        { ImGui::CloseCurrentPopup(); }

        ImGui::EndPopup();
    }

    if (doConvert)
    {
        auto format = GetDXGIFormat(m_FormatIndex);

        // 同じフォーマットの場合は変換せずに終了.
        if (format == m_ScratchImage.GetMetadata().format)
            return;

        DirectX::ScratchImage scratchImage;
        HRESULT hr = S_OK;

        if (asdx::IsCompressed(format))
        {
            hr = DirectX::Compress(
                m_ScratchImage.GetImages(), m_ScratchImage.GetImageCount(), m_ScratchImage.GetMetadata(),
                format,
                DirectX::TEX_COMPRESS_PARALLEL,
                DirectX::TEX_THRESHOLD_DEFAULT,
                scratchImage);
        }
        else
        {
            hr = DirectX::Convert(
                m_ScratchImage.GetImages(), m_ScratchImage.GetImageCount(), m_ScratchImage.GetMetadata(),
                format,
                DirectX::TEX_FILTER_DEFAULT,
                DirectX::TEX_THRESHOLD_DEFAULT,
                scratchImage);
        }
        if (SUCCEEDED(hr))
        {
            m_ScratchImage = std::move(scratchImage);
            RecreateTexture(pCmd);
        }
    }
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

    // テクスチャを描画.
    if (m_Texture.GetGpuHandleSRV().ptr != 0)
    {
        auto& meta = m_ScratchImage.GetMetadata();

        auto w = (m_Width  < meta.width)  ? m_Width  : meta.width;
        auto h = (m_Height < meta.height) ? m_Height : meta.height;

        m_SpriteRenderer.SetPipelineState(pCmd);
        m_SpriteRenderer.SetTexture(m_Texture.GetGpuHandleSRV(), m_PointClamp.GetGpuHandle());
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
    if (ImGui::MenuItem(u8"開く"))
    {
        const char* filter = 
            "テクスチャファイル(*.dds, *.tga, *.hdr, *.bmp, *.jpg, *.jpeg, *.png, *.tif, *.tiff, *.gif, *.hdp)\0*.dds;*.tga;*.hdr;*.bmp;*.jpg;*.jpeg;*.png;*.tif;*.tiff;*.gif;*.hdp;*.jxr;*.wdp;*.heif;*.heic\0"
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

        std::string path;
        if (asdx::OpenFileDlg(filter, path))
        {
            auto wpath = asdx::ToStringW(path);
            if (LoadScratchImage(wpath.c_str()))
            { RecreateTexture(pCmd); }
        }
    }
    if (ImGui::MenuItem(u8"名前を付けて保存"))
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
        std::string base;
        std::string ext = ".txb";

        if (asdx::SaveFileDlg(filter, base, ext))
        {
            m_OutputPath = base + ext;
            auto wpath = asdx::ToStringW(m_OutputPath);
            SaveScratchImage(wpath.c_str());
        }
    }
    if (ImGui::MenuItem(u8"上書き保存"))
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

    if (ImGui::MenuItem(u8"ミップマップ生成"))
    {
        DirectX::ScratchImage scratchImage;

        auto hr = DirectX::GenerateMipMaps(
            m_ScratchImage.GetImages(), m_ScratchImage.GetImageCount(), m_ScratchImage.GetMetadata(),
            DirectX::TEX_FILTER_CUBIC, 0, scratchImage);
        if (SUCCEEDED(hr))
        {
            m_ScratchImage = std::move(scratchImage);
            RecreateTexture(pCmd);
        }
    }
    if (ImGui::MenuItem(u8"フォーマット変換"))
    {
        m_OpenConvert = true;
    }
    if (ImGui::MenuItem(u8"テクスチャをリサイズ"))
    {
        m_OpenResize = true;
    }

}

//-----------------------------------------------------------------------------
//      ヘルプメニューの処理.
//-----------------------------------------------------------------------------
void TextureViewer::MenuHelp()
{
    if (ImGui::MenuItem(u8"バージョン情報"))
    {
        asdx::InfoDlg("Version Info",
            "TextureViewer ver 0.0\n"
            "Build 0.0\n"
            "Copyright(c) Project Asura.");
    }
    if (ImGui::MenuItem(u8"ライセンス情報"))
    {
    }
}

//-----------------------------------------------------------------------------
//      テクスチャを再生成します.
//-----------------------------------------------------------------------------
void TextureViewer::RecreateTexture(ID3D12GraphicsCommandList* pCmd)
{
    std::vector<uint8_t>   blob;
    asdx::TextureConverter conv;

    // asdx形式に変換.
    if (!conv.Convert(m_ScratchImage, blob))
    {
        ELOG("Error : TexutreConverter::Convret() Failed.");
        return;
    }

    // テクスチャを遅延解放.
    m_Texture.Term();

    // テクスチャバイナリをロード.
    asdx::TextureBinary bin;
    bin.Load(std::move(blob));

    // リソースを取得.
    asdx::ResTexture res = bin.GetResource();

    // テクスチャ初期化.
    if (!m_Texture.Init(pCmd, res))
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

    std::vector<uint8_t>   blob;
    asdx::TextureConverter conv;
    if (!conv.Convert(m_ScratchImage, blob))
    {
        ELOG("Error : TextureConverter::Convert() Failed.");
        return;
    }

    FILE* fp = nullptr;
    auto err = fopen_s(&fp, path, "wb");
    if (err != 0)
    {
        ELOG("Error : File Open Failed. path = %s", path);
        return;
    }

    fwrite(blob.data(), blob.size(), 1, fp);
    fclose(fp);

    ILOG("Info : TextureBinary Output success. path = %s", path);
}

//-----------------------------------------------------------------------------
//      スクラッチイメージをロードします.
//-----------------------------------------------------------------------------
bool TextureViewer::LoadScratchImage(const wchar_t* path)
{
    // テクスチャを読み込む.
    auto ext = asdx::ToLowerW(asdx::GetExtW(path).c_str());

    HRESULT hr = S_OK;
    DirectX::ScratchImage scratchImage;
    if (ext == L"dds")
    {
        hr = DirectX::LoadFromDDSFile(path, DirectX::DDS_FLAGS_NONE, nullptr, scratchImage);
    }
    else if (ext == L"tga")
    {
        hr = DirectX::LoadFromTGAFile(path, DirectX::TGA_FLAGS_NONE, nullptr, scratchImage);
    }
    else if (ext == L"hdr")
    {
        hr = DirectX::LoadFromHDRFile(path, nullptr, scratchImage);
    }
    else if (ext == L"bmp" || ext == L"jpg" || ext == L"jpeg" || ext == L"png" || ext == L"tif" 
          || ext == L"tiff" || ext == L"gif" || ext == L"hdp" || ext == L"wdp" || ext == L"jxr"
          || ext == L"heif" || ext == L"heic")
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
        m_ScratchImage = std::move(scratchImage);
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
    auto ext = asdx::ToLowerW(asdx::GetExtW(path).c_str());

    HRESULT hr = S_OK;
    DirectX::ScratchImage scratchImage;
    if (ext == L"txb")
    {
        std::vector<uint8_t>   blob;
        asdx::TextureConverter conv;
        if (!conv.Convert(m_ScratchImage, blob))
        {
            ELOG("Error : TextureConverter::Convert() Failed.");
            return false;
        }

        FILE* fp = nullptr;
        auto err = _wfopen_s(&fp, path, L"wb");
        if (err != 0)
        {
            ELOG("Error : File Open Failed. path = %s", path);
            return false;
        }

        fwrite(blob.data(), blob.size(), 1, fp);
        fclose(fp);

        hr = S_OK;
    }
    else if (ext == L"dds")
    {
        hr = DirectX::SaveToDDSFile(
            m_ScratchImage.GetImages(),
            m_ScratchImage.GetImageCount(),
            m_ScratchImage.GetMetadata(),
            DirectX::DDS_FLAGS_NONE,
            path);
    }
    else if (ext == L"tga")
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
    else if (ext == L"hdr")
    {
        assert(m_ScratchImage.GetImageCount() > 0);
        auto images = m_ScratchImage.GetImages();
        hr = DirectX::SaveToHDRFile(
            images[0],
            path);
    }
    else if (ext == L"bmp")
    {
        hr = DirectX::SaveToWICFile(
            m_ScratchImage.GetImages(),
            m_ScratchImage.GetImageCount(),
            DirectX::WIC_FLAGS_NONE,
            DirectX::GetWICCodec(DirectX::WIC_CODEC_BMP),
            path);
    }
    else if (ext == L"jpg")
    {
        hr = DirectX::SaveToWICFile(
            m_ScratchImage.GetImages(),
            m_ScratchImage.GetImageCount(),
            DirectX::WIC_FLAGS_NONE,
            DirectX::GetWICCodec(DirectX::WIC_CODEC_JPEG),
            path);
    }
    else if (ext == L"png")
    {
        hr = DirectX::SaveToWICFile(
            m_ScratchImage.GetImages(),
            m_ScratchImage.GetImageCount(),
            DirectX::WIC_FLAGS_NONE,
            DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG),
            path);
    }
    else if (ext == L"tif")
    {
        hr = DirectX::SaveToWICFile(
            m_ScratchImage.GetImages(),
            m_ScratchImage.GetImageCount(),
            DirectX::WIC_FLAGS_NONE,
            DirectX::GetWICCodec(DirectX::WIC_CODEC_TIFF),
            path);
    }
    else if (ext == L"hdp" || ext == L"jxr" || ext == L"wdp")
    {
        hr = DirectX::SaveToWICFile(
            m_ScratchImage.GetImages(),
            m_ScratchImage.GetImageCount(),
            DirectX::WIC_FLAGS_NONE,
            DirectX::GetWICCodec(DirectX::WIC_CODEC_WMP),
            path);
    }
    else if (ext == L"heif" || ext == L"heic")
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