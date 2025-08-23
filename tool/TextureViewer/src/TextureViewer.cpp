//-----------------------------------------------------------------------------
// File : TextureViewer.cpp
// Desc : Texture Viewer Application.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <TextureViewer.h>
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx12.h>
#include <shlwapi.h>

#define WND_CLASS_NAME L"TextureViewer"

#ifndef ELOG
#define ELOG(x, ...)    fprintf_s(stderr, "[File: %s, Line:%d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif//ELOG

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace {

template<typename T>
void SafeRelease(T*& ptr)
{
    if (ptr != nullptr)
    {
        ptr->Release();
        ptr = nullptr;
    }
}

//-----------------------------------------------------------------------------
//      ファイルパスを検索します.
//-----------------------------------------------------------------------------
bool SearchFilePathA( const char* filePath, std::string& result )
{
    if ( filePath == nullptr )
    { return false; }

    if ( strcmp( filePath, " " ) == 0 || strcmp( filePath, "" ) == 0 )
    { return false; }

    char exePath[ 520 ] = { 0 };
    GetModuleFileNameA( nullptr, exePath, 520  );
    exePath[ 519 ] = 0; // null終端化.
    PathRemoveFileSpecA( exePath );

    char dstPath[ 520 ] = { 0 };

    strcpy_s( dstPath, filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "..\\%s", filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "..\\..\\%s", filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "\\res\\%s", filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "%s\\%s", exePath, filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "%s\\..\\%s", exePath, filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "%s\\..\\..\\%s", exePath, filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    sprintf_s( dstPath, "%s\\res\\%s", exePath, filePath );
    if ( PathFileExistsA( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
//      オープンファイルダイアログです.
//-----------------------------------------------------------------------------
bool OpenFileDlg(const char* fileFilter, std::string& result, const std::string& defaultPath)
{
    OPENFILENAMEA ofn;
    ZeroMemory( &ofn, sizeof(ofn) );

    CHAR inputFile     [ MAX_PATH ] = { 0 };
    CHAR inputFileTitle[ MAX_PATH ] = { 0 };
    CHAR initDir       [ MAX_PATH ] = { 0 };

    // パスが設定されていれば初期ディレク処理を設定.
    if (!defaultPath.empty() && defaultPath != "")
    {
        auto path = defaultPath;
        auto idx = path.find_last_of("\\");
        if (idx != std::string::npos && idx == path.length() - 1)
        { path = path.substr(0, idx); }
        strcpy_s(initDir, path.c_str());
    }

    ofn.lStructSize     = sizeof(OPENFILENAMEA);
    ofn.hwndOwner       = nullptr;
    ofn.lpstrFilter     = fileFilter;
    ofn.nMaxFile        = MAX_PATH;
    ofn.nMaxFileTitle   = MAX_PATH;
    ofn.Flags           = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrTitle      = "Open";
    ofn.lpstrFile       = inputFile;
    ofn.lpstrFileTitle  = inputFileTitle;
    ofn.lpstrInitialDir = initDir;

    if ( GetOpenFileNameA( &ofn ) == TRUE )
    {
        result = inputFile;
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
//      セーブファイルダイアログです.
//-----------------------------------------------------------------------------
bool SaveFileDlg(const char* fileFilter, std::string& base, std::string& ext, const std::string& defaultPath)
{
    OPENFILENAMEA ofn;
    ZeroMemory( &ofn, sizeof(ofn) );

    CHAR inputFile     [ MAX_PATH ] = { 0 };
    CHAR inputFileTitle[ MAX_PATH ] = { 0 };
    CHAR templateName  [ MAX_PATH ] = { 0 };
    CHAR initDir       [ MAX_PATH ] = { 0 };

    if (!defaultPath.empty() && defaultPath != "")
    {
        auto path = defaultPath;
        auto idx = path.find_last_of("\\");
        if (idx != std::string::npos && idx == path.length() - 1)
        { path = path.substr(0, idx); }
        strcpy_s(initDir, path.c_str());
    }

    ofn.lStructSize     = sizeof(OPENFILENAMEA);
    ofn.hwndOwner       = nullptr;
    ofn.lpstrFilter     = fileFilter;
    ofn.nMaxFile        = MAX_PATH;
    ofn.nMaxFileTitle   = MAX_PATH;
    ofn.nFilterIndex    = 1;
    ofn.Flags           = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    ofn.lpstrTitle      = "Save As";
    ofn.lpstrFile       = inputFile;
    ofn.lpstrFileTitle  = inputFileTitle;
    ofn.lpTemplateName  = templateName;
    ofn.lpstrInitialDir = initDir;

    if ( GetSaveFileNameA( &ofn ) == TRUE )
    {
        base = std::string( inputFile ).substr( 0, ofn.nFileExtension - 1 );
        if ( ofn.nFileExtension != 0 )
        {
            ext = std::string( inputFile ).substr( ofn.nFileExtension );
        }
        return true;
    }

    return false;
}

} // namespace

///////////////////////////////////////////////////////////////////////////////
// TextureViewer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
TextureViewer::TextureViewer()
: m_hInstance   (nullptr)
, m_hWnd        (nullptr)
, m_Width       (1280)
, m_Height      (720)
, m_StandByMode (false)
{
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
TextureViewer::~TextureViewer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool TextureViewer::Init()
{
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr))
    {
        ELOG("Error : CoInitialize() Failed. errcode = 0%x", hr);
        return false;
    }

    m_hInstance = GetModuleHandleW(nullptr);
    if (!m_hInstance)
    {
        ELOG("Error : GetModuleHandle() Failed.");
        return false;
    }

    WNDCLASSEXW wc = {};
    wc.cbSize           = sizeof(wc);
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = WndProc;
    wc.hInstance        = m_hInstance;
    wc.hIcon            = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor          = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName     = nullptr;
    wc.lpszClassName    = WND_CLASS_NAME;

    RegisterClassExW(&wc);

    RECT rc = { 0, 0, LONG(m_Width), LONG(m_Height) };
    DWORD style = WS_OVERLAPPEDWINDOW;
    AdjustWindowRect(&rc, style, FALSE);

    // ウィンドウ生成.
    m_hWnd = CreateWindowW(
        WND_CLASS_NAME,
        L"TextureViewer",
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        (rc.right - rc.left),
        (rc.bottom - rc.top),
        nullptr,
        nullptr,
        m_hInstance,
        this);
    if (!m_hWnd)
    {
        ELOG("Error : CreateWindowW() Failed.");
        return false;
    }

    bool enableDebugLayer = true;

    if (enableDebugLayer)
    {
        ID3D12Debug* pDebug = nullptr;
        auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug));
        if (SUCCEEDED(hr))
        {
            pDebug->EnableDebugLayer();
        }
    }

    // DXGIファクトリー生成.
    {
        UINT flags = 0;
        auto hr = CreateDXGIFactory2(flags, IID_PPV_ARGS(&m_pDXGIFactory));
        if (FAILED(hr))
        {
            ELOG("Error : CreateDXGIFactory2() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // デバイス生成.
    {
        auto hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice));
        if (FAILED(hr))
        {
            ELOG("Error : D3D12CreateDevice() Failed. errcode = 0x%x", hr);
            return false;
        }

        if (enableDebugLayer)
        {
            ID3D12InfoQueue* pInfoQueue = nullptr;
            hr = m_pDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
            if (SUCCEEDED(hr))
            {
                // エラー発生時にブレークさせる.
                { pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE); }

                // 警告発生時にブレークさせる.
                //{ pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE); }

                // 無視するメッセージID.
                D3D12_MESSAGE_ID denyIds[] = {
                    D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                    D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
                };

                // 無視するメッセージレベル.
                D3D12_MESSAGE_SEVERITY severities[] = {
                    D3D12_MESSAGE_SEVERITY_INFO
                };

                D3D12_INFO_QUEUE_FILTER filter = {};
                filter.DenyList.NumIDs          = _countof(denyIds);
                filter.DenyList.pIDList         = denyIds;
                filter.DenyList.NumSeverities   = _countof(severities);
                filter.DenyList.pSeverityList   = severities;

                pInfoQueue->PushStorageFilter(&filter);
            }

            SafeRelease(pInfoQueue);
        }
    }

    // グラフィックスキューを生成.
    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

        auto hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pGraphicsQueue));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommandQueue() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // スワップチェインの初期化.
    {
        DXGI_SWAP_CHAIN_DESC1 desc = {};
        desc.Width              = m_Width;
        desc.Height             = m_Height;
        desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.Stereo             = FALSE;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.BufferCount        = 2;
        desc.Scaling            = DXGI_SCALING_STRETCH;
        desc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.Flags              = 0;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc = {};
        fullScreenDesc.RefreshRate      = { 1, 60 };
        fullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        fullScreenDesc.Scaling          = DXGI_MODE_SCALING_STRETCHED;
        fullScreenDesc.Windowed         = TRUE;

        IDXGISwapChain1* pSwapChain1 = nullptr;
        auto hr = m_pDXGIFactory->CreateSwapChainForHwnd(m_pGraphicsQueue, m_hWnd, &desc, &fullScreenDesc, nullptr, &pSwapChain1);
        if (FAILED(hr))
        {
            ELOG("Error : IDXGIFactory2::CreateSwapChainForHwnd() Failed. errcode = 0x%x", hr);
            return false;
        }

        hr = pSwapChain1->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
        if (FAILED(hr))
        {
            SafeRelease(pSwapChain1);
            ELOG("Error : IDXGISwapChain2::QueryInterface() Failed. errcode = 0x%x", hr);
            return false;
        }

        SafeRelease(pSwapChain1);
    }

    // RTVディスクリプタヒープの生成.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = 256;

        auto hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pDescriptorHeapRTV));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateDescriptorHeap() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // DSVディスクリプタヒープの生成.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        desc.NumDescriptors = 256;

        auto hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pDescriptorHeapDSV));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateDescriptorHeap() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // リソースディスクリプタヒープの生成.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 512;
        desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        auto hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pDescriptorHeapResource));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateDescriptorHeap() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // レンダーターゲットの生成.
    {
        for(auto i=0; i<2; ++i)
        {
            auto hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pColorBuffer[i]));
            if (FAILED(hr))
            {
                ELOG("Error : IDXGISwapChain::GetBuffer() Failed. errcode = 0x%x", hr);
                return false;
            }

            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.ViewDimension           = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            rtvDesc.Texture2D.MipSlice      = 0;
            rtvDesc.Texture2D.PlaneSlice    = 0;

            m_ColorBufferHandle[i] = m_pDescriptorHeapRTV->GetCPUDescriptorHandleForHeapStart();
            m_ColorBufferHandle[i].ptr += (SIZE_T(i) * m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

            m_pDevice->CreateRenderTargetView(m_pColorBuffer[i], &rtvDesc, m_ColorBufferHandle[i]);
        }
    }

    // 深度ステンシルビューの生成.
    {
        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width              = m_Width;
        desc.Height             = m_Height;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = DXGI_FORMAT_D32_FLOAT;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_HEAP_PROPERTIES props = {};
        props.Type                  = D3D12_HEAP_TYPE_DEFAULT;
        props.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        props.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
        props.CreationNodeMask      = 1;
        props.VisibleNodeMask       = 1;

        auto hr = m_pDevice->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, nullptr, IID_PPV_ARGS(&m_pDepthBuffer));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
            return false;
        }

        D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
        viewDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
        viewDesc.Format             = DXGI_FORMAT_D32_FLOAT;
        viewDesc.Texture2D.MipSlice = 0;

        m_DepthBufferHandle = m_pDescriptorHeapDSV->GetCPUDescriptorHandleForHeapStart();

        m_pDevice->CreateDepthStencilView(m_pDepthBuffer, &viewDesc, m_DepthBufferHandle);
    }

    // イベントを生成.
    {
        m_FenceEvent = CreateEventEx(nullptr, nullptr, FALSE, EVENT_ALL_ACCESS);
        if (m_FenceEvent == nullptr)
        {
            ELOG("Error : CreateEventEx() Failed.");
            return false;
        }
    }

    // フェンス生成.
    {
        auto hr = m_pDevice->CreateFence(1, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateFence() Failed. errcode = 0x%x", hr);
            return false;
        }
        m_FenceValue = 1;
    }

    // コマンドアロケータ生成.
    {
        for(auto i=0; i<2; ++i)
        {
            auto hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator[i]));
            if (FAILED(hr))
            {
                ELOG("Error : ID3D12Device::CreateCommandAllocator() Failed. errcode = 0x%x", hr);
                return false;
            }
        }
    }

    // コマンドリスト生成.
    {
        auto hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator[0], nullptr, IID_PPV_ARGS(&m_pCommandList));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommandList() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // ImGui初期化.
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();

        // フォント読み込み.
        {
            //const char* path = u8"やさしさゴシック.tff";
            //io.Fonts->AddFontFromFileTTF(path, 12.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
        }

        ImGui::StyleColorsDark();

        if (!ImGui_ImplWin32_Init(m_hWnd))
        {
            ELOG("Error : ImGui_ImplWin32_Init() Failed.");
            return false;
        }

        if (!ImGui_ImplDX12_Init(m_pDevice, 2, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 
            m_pDescriptorHeapResource,
            m_pDescriptorHeapResource->GetCPUDescriptorHandleForHeapStart(),
            m_pDescriptorHeapResource->GetGPUDescriptorHandleForHeapStart()))
        {
            ELOG("Error : ImGui_ImplDX12_Init() Failed.");
            return false;
        }
    }


    // コマンド終了.
    m_pCommandList->Close();

    UpdateWindow(m_hWnd);
    ShowWindow(m_hWnd, SW_SHOWNORMAL);
    SetFocus(m_hWnd);

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void TextureViewer::Term()
{
    m_StopDraw = true;
    WaitIdle();

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (m_FenceEvent != nullptr)
    {
        CloseHandle(m_FenceEvent);
        m_FenceEvent = nullptr;
    }

    SafeRelease(m_pFence);

    SafeRelease(m_pDescriptorHeapResource);
    SafeRelease(m_pDescriptorHeapDSV);
    SafeRelease(m_pDescriptorHeapRTV);

    SafeRelease(m_pCommandList);
    for(auto i=0; i<2; ++i)
    {
        SafeRelease(m_pCommandAllocator[i]);
    }

    for(auto i=0; i<2; ++i)
    {
        SafeRelease(m_pColorBuffer[i]);
        m_ColorBufferHandle[i] = {};
    }

    SafeRelease(m_pDepthBuffer);
    m_DepthBufferHandle = {};

    SafeRelease(m_pSwapChain);
    SafeRelease(m_pGraphicsQueue);
    SafeRelease(m_pDevice);
    SafeRelease(m_pDXGIFactory);

    if (m_hInstance != nullptr)
    { UnregisterClass(WND_CLASS_NAME, m_hInstance); }

    m_StopDraw = false;
}

//-----------------------------------------------------------------------------
//      メインループです.
//-----------------------------------------------------------------------------
int TextureViewer::MainLoop()
{
    MSG msg = {};
    while(WM_QUIT != msg.message)
    {
        auto hasMsg = PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
        if (hasMsg)
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        else
        {
            OnDraw();
        }
    }

    return (int)msg.wParam;
}

//-----------------------------------------------------------------------------
//      アプリケーションを実行します.
//-----------------------------------------------------------------------------
int TextureViewer::Run()
{
    int ret = -1;

    if (Init())
    { ret = MainLoop(); }

    return ret;
}

void TextureViewer::Wait(UINT64 fenceValue, UINT32 timeout)
{
    if (m_pFence == nullptr)
        return;

    if (m_pFence->GetCompletedValue() < fenceValue)
    {
        auto hr = m_pFence->SetEventOnCompletion(fenceValue, m_FenceEvent);
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Fence::SetEventOnCompletion() Failed. errcode = 0x%x", hr);
            return;
        }

        WaitForSingleObject(m_FenceEvent, timeout);
    }
}

void TextureViewer::WaitIdle()
{
    auto fence = 0;
    auto hr = m_pGraphicsQueue->Signal(m_pFence, 0);
    if (FAILED(hr))
    {
        return;
    }

    Wait(0, INFINITE);
}

//-----------------------------------------------------------------------------
//      表示処理を行います.
//-----------------------------------------------------------------------------
void TextureViewer::Present(UINT syncInterval)
{
    if (m_hWnd == nullptr || m_pSwapChain == nullptr)
        return;

    HRESULT hr = S_OK;
    if (m_StandByMode)
    {
        hr = m_pSwapChain->Present(syncInterval, DXGI_PRESENT_TEST);
        if (hr == S_OK)
        { m_StandByMode = false; }

        return;
    }

    hr = m_pSwapChain->Present(syncInterval, 0);

    switch(hr)
    {
    case DXGI_ERROR_DEVICE_RESET:
        {
            MessageBoxW(m_hWnd, L"DXGI_ERROR_DEVICE_RESET Occurred. Shutting Down.", L"Fatal Error", MB_OK | MB_ICONERROR);
            PostQuitMessage(1);
        }
        break;

    case DXGI_ERROR_DEVICE_REMOVED:
        {
            MessageBoxW(m_hWnd, L"DXGI_ERROR_DEICE_REMOVED Occurred. Shutting Down.", L"Fatal Error", MB_OK | MB_ICONERROR);
            PostQuitMessage(2);
        }
        break;

    case DXGI_STATUS_OCCLUDED:
        {
            m_StandByMode = true;
        }
        break;

    case S_OK:
        {
            /* DO_NOTHING */
        }
        break;

    default:
        {
            ELOG("Error : IDXGISwapChain::Present() Failed. errcode = 0x%x", hr);
        }
        break;
    }
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void TextureViewer::OnDraw()
{
    if (m_pCommandList == nullptr || m_pGraphicsQueue == nullptr || m_pSwapChain == nullptr || m_StopDraw)
        return;

    auto idx = m_pSwapChain->GetCurrentBackBufferIndex();

    if (m_pColorBuffer[idx] == nullptr)
        return;

    // ImGuiでのUI設定.
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        UIStatus();

        UIPopupMenu();
    }

    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX   = 0.0f;
    viewport.TopLeftY   = 0.0f;
    viewport.Width      = FLOAT(m_Width);
    viewport.Height     = FLOAT(m_Height);
    viewport.MinDepth   = 0.0f;
    viewport.MaxDepth   = 1.0f;

    D3D12_RECT scissor = {};
    scissor.left    = 0;
    scissor.right   = m_Width;
    scissor.top     = 0;
    scissor.bottom  = m_Height;

    const float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };

    m_pCommandList->Reset(m_pCommandAllocator[idx], nullptr);

    ID3D12DescriptorHeap* pDescriptorHeaps[] = {
        m_pDescriptorHeapResource
    };
    m_pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

    m_pCommandList->RSSetViewports(1, &viewport);
    m_pCommandList->RSSetScissorRects(1, &scissor);

    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_pColorBuffer[idx];
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        m_pCommandList->ResourceBarrier(1, &barrier);
    }

    m_pCommandList->OMSetRenderTargets(1, &m_ColorBufferHandle[idx], FALSE, &m_DepthBufferHandle);
    m_pCommandList->ClearRenderTargetView(m_ColorBufferHandle[idx], clearColor, 0, nullptr);
    m_pCommandList->ClearDepthStencilView(m_DepthBufferHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // テクスチャ描画.
    {
    }

    // ImGui描画.
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_pCommandList);

    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_pColorBuffer[idx];
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

        m_pCommandList->ResourceBarrier(1, &barrier);
    }

    m_pCommandList->Close();

    ID3D12CommandList* ppCommandLists[] = { m_pCommandList };
    m_pGraphicsQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    {
        auto fence = m_FenceValue;
        auto hr = m_pGraphicsQueue->Signal(m_pFence, m_FenceValue);
        if (SUCCEEDED(hr))
        { m_FenceValue++; }

        Wait(fence, INFINITE);
    }

    Present(1);
}

//-----------------------------------------------------------------------------
//      リサイズ処理を行います.
//-----------------------------------------------------------------------------
void TextureViewer::OnResize(UINT w, UINT h)
{
    // サイズが同じなら無駄に処理させない.
    if (m_Width == w && m_Height == h)
        return;

    m_Width  = w;
    m_Height = h;
    m_StopDraw = true;

    // コマンドの完了を待機
    WaitIdle();

    for(auto i=0; i<2; ++i)
    {
        SafeRelease(m_pColorBuffer[i]);
    }
    SafeRelease(m_pDepthBuffer);

    auto hr = m_pSwapChain->ResizeBuffers(2, m_Width, m_Height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    if (FAILED(hr))
    {
        ELOG("Error : IDXGISwapChain::ResizeBuffer() Failed. errcode = 0x%x", hr);
        return;
    }

    for(auto i=0; i<2; ++i)
    {
        auto hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pColorBuffer[i]));
        if (FAILED(hr))
        {
            ELOG("Error : IDXGISwapChain::GetBuffer() Failed. errcode = 0x%x", hr);
            return;
        }

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.ViewDimension           = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        rtvDesc.Texture2D.MipSlice      = 0;
        rtvDesc.Texture2D.PlaneSlice    = 0;

        m_ColorBufferHandle[i] = m_pDescriptorHeapRTV->GetCPUDescriptorHandleForHeapStart();
        m_ColorBufferHandle[i].ptr += (SIZE_T(i) * m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

        m_pDevice->CreateRenderTargetView(m_pColorBuffer[i], &rtvDesc, m_ColorBufferHandle[i]);
    }

    // 深度ステンシルビューの生成.
    {
        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width              = m_Width;
        desc.Height             = m_Height;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = DXGI_FORMAT_D32_FLOAT;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_HEAP_PROPERTIES props = {};
        props.Type                  = D3D12_HEAP_TYPE_DEFAULT;
        props.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        props.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
        props.CreationNodeMask      = 1;
        props.VisibleNodeMask       = 1;

        auto hr = m_pDevice->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, nullptr, IID_PPV_ARGS(&m_pDepthBuffer));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
            return;
        }

        D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
        viewDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
        viewDesc.Format             = DXGI_FORMAT_D32_FLOAT;
        viewDesc.Texture2D.MipSlice = 0;

        m_DepthBufferHandle = m_pDescriptorHeapDSV->GetCPUDescriptorHandleForHeapStart();

        m_pDevice->CreateDepthStencilView(m_pDepthBuffer, &viewDesc, m_DepthBufferHandle);
    }

    m_StopDraw = false;
}

//-----------------------------------------------------------------------------
//      ファイルドロップ時の処理です.
//-----------------------------------------------------------------------------
void TextureViewer::OnDrop(const std::vector<std::string>& files)
{
}

//-----------------------------------------------------------------------------
//      マウス処理です.
//-----------------------------------------------------------------------------
void TextureViewer::OnMouse(int x, int y, int wheelDelta, bool isDownL, bool isDownM, bool isDownR)
{
}

//-----------------------------------------------------------------------------
//      キー処理です.
//-----------------------------------------------------------------------------
void TextureViewer::OnKey(UINT keyCode, bool isKeyDown, bool isAltDown)
{
}

//-----------------------------------------------------------------------------
//      タイピング処理です.
//-----------------------------------------------------------------------------
void TextureViewer::OnTyping(UINT keyCode)
{
}

//-----------------------------------------------------------------------------
//      ウィンドウプロシージャです.
//-----------------------------------------------------------------------------
LRESULT CALLBACK TextureViewer::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    auto pThis = reinterpret_cast<TextureViewer*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch(msg)
    {
    case WM_CREATE:
        {
            auto pCreateArgs = reinterpret_cast<LPCREATESTRUCT>(lp);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateArgs->lpCreateParams));

            DragAcceptFiles(hWnd, TRUE);
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps = {};
            auto hDC = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_DESTROY:
        {
            PostQuitMessage(0);
        }
        break;

    case WM_SIZE:
        {
            if (pThis != nullptr)
            {
                auto w = UINT(LOWORD(lp));
                auto h = UINT(HIWORD(lp));
                pThis->OnResize(w, h);
            }
        }
        break;

    case WM_DROPFILES:
        {
            if (pThis != nullptr)
            {
                auto count = DragQueryFile((HDROP)wp, UINT32_MAX, nullptr, 0);
                std::vector<std::string> files;
                files.resize(count);

                for(size_t i=0; i<files.size(); ++i)
                {
                    char path[MAX_PATH] = {};
                    DragQueryFileA((HDROP)wp, UINT(i), path, MAX_PATH);
                    files[i] = path;
                }

                pThis->OnDrop(files);
                files.clear();

                DragFinish((HDROP)wp);
            }
        }
        break;

    case WM_CHAR:
        {
            if (pThis != nullptr)
            {
                auto keyCode = UINT(wp);
                pThis->OnTyping(keyCode);
            }
        }
        break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
        {
            if (pThis != nullptr)
            {
                DWORD mask = (1 << 29);

                auto isKeyDown = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
                bool isAltDown = !!(lp & mask);

                pThis->OnKey(UINT(wp), isKeyDown, isAltDown);
            }
        }
        break;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    case WM_MOUSEWHEEL:
    case WM_MOUSEMOVE:
        {
            if (pThis != nullptr)
            {
                auto x = int(LOWORD(lp));
                auto y = int(HIWORD(lp));

                int wheelDelta = 0;
                if (msg == WM_MOUSEWHEEL)
                {
                    POINT pt = {};
                    pt.x = x;
                    pt.y = y;

                    ScreenToClient(hWnd, &pt);
                    x = pt.x;
                    y = pt.y;

                    wheelDelta += int(HIWORD(wp));
                }

                auto buttonMask = LOWORD(wp);
                auto isDownL = !!(buttonMask & MK_LBUTTON);
                auto isDownM = !!(buttonMask & MK_MBUTTON);
                auto isDownR = !!(buttonMask & MK_RBUTTON);

                pThis->OnMouse(x, y, wheelDelta, isDownL, isDownM, isDownR);
            }
        }
        break;
    }

    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp))
        return true;

    return DefWindowProc(hWnd, msg, wp, lp);
}

void TextureViewer::UIStatus()
{
    int flags = ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiWindowFlags_NoTitleBar;
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(300, 50));
    ImGui::Begin(u8"Status", nullptr, flags);
    {
        ImGui::Text(u8"Input Path : %s", m_InputPath.c_str());
        ImGui::Text(u8"Output Path :%s", m_OutputPath.c_str());
    }
    ImGui::End();
}

void TextureViewer::UIPopupMenu()
{
    if (!ImGui::IsAnyItemHovered())
    {
        if (ImGui::IsMouseClicked(1))
        {
            ImGui::OpenPopup(u8"PopupMenu");
        }
    }

    if (ImGui::BeginPopup(u8"PopupMenu"))
    {
        if (ImGui::BeginMenu(u8"ファイル"))
        {
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(u8"設定"))
        {
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(u8"ヘルプ"))
        {
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }
}