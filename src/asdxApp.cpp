//-----------------------------------------------------------------------------
// File : asdxApp.cpp
// Desc : Application Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <list>
#include <cassert>
#include <asdxApp.h>
#include <asdxMath.h>
#include <asdxLogger.h>


namespace /* anonymous */ {

///////////////////////////////////////////////////////////////////////////////
// ApplicationList class
///////////////////////////////////////////////////////////////////////////////
class ApplicationList
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    typedef std::list< asdx::Application* >                 List;
    typedef std::list< asdx::Application* >::iterator       ListItr;
    typedef std::list< asdx::Application* >::const_iterator ListCItr;

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    ApplicationList()
    { m_List.clear(); }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~ApplicationList()
    { m_List.clear(); }

    //-------------------------------------------------------------------------
    //! @brief      push_back()のラッパー関数です.
    //-------------------------------------------------------------------------
    void PushBack( asdx::Application* pApp )
    { m_List.push_back( pApp ); }

    //-------------------------------------------------------------------------
    //! @brief      push_front()のラッパー関数です.
    //-------------------------------------------------------------------------
    void PushFront( asdx::Application* pApp )
    { m_List.push_front( pApp ); }

    //-------------------------------------------------------------------------
    //! @brief      pop_back()のラッパー関数です.
    //-------------------------------------------------------------------------
    void PopBack()
    { m_List.pop_back(); }

    //-------------------------------------------------------------------------
    //! @brief      pop_front()のラッパー関数です.
    //-------------------------------------------------------------------------
    void PopFront()
    { m_List.pop_front(); }

    //-------------------------------------------------------------------------
    //! @brief      clear()のラッパー関数です.
    //-------------------------------------------------------------------------
    void Clear()
    { m_List.clear(); }

    //-------------------------------------------------------------------------
    //! @brief      remove()のラッパー関数です.
    //-------------------------------------------------------------------------
    void Remove( asdx::Application* pApp )
    { m_List.remove( pApp ); }

    //-------------------------------------------------------------------------
    //! @brief      begin()のラッパー関数です.
    //-------------------------------------------------------------------------
    ListItr Begin()
    { return m_List.begin(); }

    //-------------------------------------------------------------------------
    //! @brief      begin()のラッパー関数です(const版).
    //-------------------------------------------------------------------------
    ListCItr Begin() const
    { return m_List.begin(); }

    //-------------------------------------------------------------------------
    //! @brief      end()のラッパー関数です.
    //-------------------------------------------------------------------------
    ListItr End()
    { return m_List.end(); }

    //-------------------------------------------------------------------------
    //! @brief      end()のラッパー関数です(const版).
    //-------------------------------------------------------------------------
    ListCItr End() const
    { return m_List.end(); }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    List    m_List;         //!< リストです.

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

// アプリケーションリスト.
ApplicationList     g_AppList;

//-----------------------------------------------------------------------------
//      領域の交差を計算します.
//-----------------------------------------------------------------------------
inline int ComputeIntersectionArea
(
    int ax1, int ay1,
    int ax2, int ay2,
    int bx1, int by1,
    int bx2, int by2
)
{
    return asdx::Max(0, asdx::Min(ax2, bx2) - asdx::Max(ax1, bx1))
         * asdx::Max(0, asdx::Min(ay2, by2) - asdx::Max(ay1, by1));
}

//-----------------------------------------------------------------------------
//      nullptrかどうかを考慮してdeleteします.
//-----------------------------------------------------------------------------
template<typename T>
void SafeDelete(T*& ptr)
{
    if (ptr != nullptr)
    {
        delete ptr;
        ptr = nullptr;
    }
}

//-----------------------------------------------------------------------------
//      nullptrかどうかを考慮してdelete[]します.
//-----------------------------------------------------------------------------
template<typename T>
void SafeDeleteArray(T*& ptr)
{
    if (ptr != nullptr)
    {
        delete[] ptr;
        ptr = nullptr;
    }
}

//-----------------------------------------------------------------------------
//      nullptrかどうかを考慮して解放処理を行います.
//-----------------------------------------------------------------------------
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
//      色度を変換した値を取得します.
//-----------------------------------------------------------------------------
inline UINT GetCoord(float value)
{ return static_cast<UINT>(value * 50000.0f); }

//-----------------------------------------------------------------------------
//      輝度を変換した値を取得します.
//-----------------------------------------------------------------------------
inline UINT GetLuma(float value)
{ return static_cast<UINT>(value * 10000.0f); }

} // namespace /* anonymous */


namespace asdx  {

// ウィンドウクラス名です.
#ifndef ASDX_WND_CLASSNAME
#define ASDX_WND_CLASSNAME      TEXT("asdxWindowClass")
#endif//ASDX_WND_CLAASNAME


///////////////////////////////////////////////////////////////////////////////////////////////////
// Application class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Application::Application()
: m_hInst               ( nullptr )
, m_hWnd                ( nullptr )
, m_AllowTearing        ( false )
, m_MultiSampleCount    ( 1 )
, m_MultiSampleQuality  ( 0 )
, m_SwapChainCount      ( 2 )
, m_SwapChainFormat     ( DXGI_FORMAT_R10G10B10A2_UNORM )
, m_DepthStencilFormat  ( DXGI_FORMAT_D32_FLOAT )
, m_pSwapChain4         ( nullptr )
, m_SampleMask          ( 0 )
, m_StencilRef          ( 0 )
, m_Width               ( 960 )
, m_Height              ( 540 )
, m_AspectRatio         ( 1.7777f )
, m_Title               ( L"asdxApplication" )
, m_Timer               ()
, m_FrameCount          ( 0 )
, m_FPS                 ( 0.0f )
, m_LatestUpdateTime    ( 0.0f )
, m_IsStopRendering     ( false )
, m_IsStandbyMode       ( false )
, m_hIcon               ( nullptr )
, m_hMenu               ( nullptr )
, m_hAccel              ( nullptr )
{
    // Corn Flower Blue.
    m_ClearColor[0] = 0.392156899f;
    m_ClearColor[1] = 0.584313750f;
    m_ClearColor[2] = 0.929411829f;
    m_ClearColor[3] = 1.000000000f;

#if defined(DEBUG) || defined(_DEBUG)
    m_DeviceDesc.EnableDebug = true;
#else
    m_DeviceDesc.EnableDebug = false;
#endif
    m_DeviceDesc.MaxColorTargetCount    = 128;
    m_DeviceDesc.MaxDepthTargetCount    = 128;
    m_DeviceDesc.MaxSamplerCount        = 128;
    m_DeviceDesc.MaxShaderResourceCount = 4096;
}

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
Application::Application( LPCWSTR title, UINT width, UINT height, HICON hIcon, HMENU hMenu, HACCEL hAccel )
: m_hInst               ( nullptr )
, m_hWnd                ( nullptr )
, m_AllowTearing        ( false )
, m_MultiSampleCount    ( 1 )
, m_MultiSampleQuality  ( 0 )
, m_SwapChainCount      ( 2 )
, m_SwapChainFormat     ( DXGI_FORMAT_R10G10B10A2_UNORM )
, m_DepthStencilFormat  ( DXGI_FORMAT_D32_FLOAT )
, m_pSwapChain4         ( nullptr )
, m_Width               ( width )
, m_Height              ( height )
, m_AspectRatio         ( (float)width/(float)height )
, m_Title               ( title )
, m_Timer               ()
, m_FrameCount          ( 0 )
, m_FPS                 ( 0.0f )
, m_LatestUpdateTime    ( 0.0f )
, m_IsStopRendering     ( false )
, m_IsStandbyMode       ( false )
, m_hIcon               ( hIcon )
, m_hMenu               ( hMenu )
{
    // Corn Flower Blue.
    m_ClearColor[0] = 0.392156899f;
    m_ClearColor[1] = 0.584313750f;
    m_ClearColor[2] = 0.929411829f;
    m_ClearColor[3] = 1.000000000f;

#if defined(DEBUG) || defined(_DEBUG)
    m_DeviceDesc.EnableDebug = true;
#else
    m_DeviceDesc.EnableDebug = false;
#endif
    m_DeviceDesc.MaxColorTargetCount    = 128;
    m_DeviceDesc.MaxDepthTargetCount    = 128;
    m_DeviceDesc.MaxSamplerCount        = 128;
    m_DeviceDesc.MaxShaderResourceCount = 4096;
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Application::~Application()
{ TermApp(); }

//-----------------------------------------------------------------------------
//      描画停止フラグを設定します.
//-----------------------------------------------------------------------------
void Application::SetStopRendering( bool isStopRendering )
{
    std::lock_guard<std::mutex> locker(m_Mutex);
    m_IsStopRendering = isStopRendering;
}

//-----------------------------------------------------------------------------
//      描画停止フラグを取得します.
//-----------------------------------------------------------------------------
bool Application::IsStopRendering()
{
    std::lock_guard<std::mutex> locker(m_Mutex);
    return m_IsStopRendering;
}

//-----------------------------------------------------------------------------
//      フレームカウントを取得します.
//-----------------------------------------------------------------------------
DWORD Application::GetFrameCount()
{
    std::lock_guard<std::mutex> locker(m_Mutex);
    return m_FrameCount;
}

//-----------------------------------------------------------------------------
//      FPSを取得します.
//-----------------------------------------------------------------------------
FLOAT Application::GetFPS()
{
    std::lock_guard<std::mutex> locker(m_Mutex);
    return m_FPS;
}

//-----------------------------------------------------------------------------
//      アプリケーションを初期化します.
//-----------------------------------------------------------------------------
bool Application::InitApp()
{
    // COMライブラリの初期化.
    HRESULT hr = CoInitialize( nullptr );
    if ( FAILED(hr) )
    {
        DLOG( "Error : Com Library Initialize Failed." );
        return false;
    }

    // COMライブラリのセキュリティレベルを設定.
    hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL);

    // セキュリティ設定の結果をチェック.
    if ( FAILED(hr) )
    {
        DLOG( "Error : Com Library Initialize Security Failed." );
        return false;
    }

    // ウィンドウの初期化.
    if ( !InitWnd() )
    {
        DLOG( "Error : InitWnd() Failed." );
        return false;
    }

    // Direct3Dの初期化.
    if ( !InitD3D() )
    {
        DLOG( "Error : InitD3D() Failed." );
        return false;
    }

    // アプリケーション固有の初期化.
    if ( !OnInit() )
    {
        ELOG( "Error : OnInit() Failed." );
        return false;
    }

    // ウィンドウを表示します.
    ShowWindow( m_hWnd, SW_SHOWNORMAL );
    UpdateWindow( m_hWnd );

    // フォーカスを設定します.
    SetFocus( m_hWnd );

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      アプリケーションの終了処理.
//-----------------------------------------------------------------------------
void Application::TermApp()
{
    // コマンドの完了を待機.
    // TODO:

    // アプリケーション固有の終了処理.
    OnTerm();

    // Direct3Dの終了処理.
    TermD3D();

    // ウィンドウの終了処理.
    TermWnd();

    // COMライブラリの終了処理.
    CoUninitialize();
}

//-----------------------------------------------------------------------------
//      ウィンドウの初期化処理.
//-----------------------------------------------------------------------------
bool Application::InitWnd()
{
    // インスタンスハンドルを取得.
    HINSTANCE hInst = GetModuleHandle( nullptr );
    if ( !hInst )
    {
        DLOG( "Error : GetModuleHandle() Failed. ");
        return false;
    }

    // アイコンなしの場合はロード.
    if ( m_hIcon == nullptr )
    { m_hIcon = LoadIcon( hInst, IDI_APPLICATION ); }

    // 拡張ウィンドウクラスの設定.
    WNDCLASSEXW wc;
    wc.cbSize           = sizeof( WNDCLASSEXW );
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = MsgProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInst;
    wc.hIcon            = m_hIcon;
    wc.hCursor          = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground    = (HBRUSH)( COLOR_WINDOW + 1 );
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = ASDX_WND_CLASSNAME;
    wc.hIconSm          = m_hIcon;

    // ウィンドウクラスを登録します.
    if ( !RegisterClassExW( &wc ) )
    {
        // エラーログ出力.
        DLOG( "Error : RegisterClassEx() Failed." );

        // 異常終了.
        return false;
    }

    // インスタンスハンドルを設定.
    m_hInst = hInst;

    // 矩形の設定.
    RECT rc = { 0, 0, static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };

#if 0 // リサイズしたくない場合.
    //DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
#else // リサイズ許可.
    DWORD style = WS_OVERLAPPEDWINDOW;
#endif
    // 指定されたクライアント領域を確保するために必要なウィンドウ座標を計算します.
    AdjustWindowRect( &rc, style, FALSE );

    // ウィンドウを生成します.
    m_hWnd = CreateWindowW(
        ASDX_WND_CLASSNAME,
        m_Title,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        ( rc.right - rc.left ),
        ( rc.bottom - rc.top ),
        NULL,
        m_hMenu,
        hInst,
        NULL
    );

    // 生成チェック.
    if ( !m_hWnd )
    {
        // エラーログ出力.
        DLOG( "Error : CreateWindow() Failed." );

        // 異常終了.
        return false;
    }

    // アプリケーションリストに登録します.
    g_AppList.PushBack( this );

    // タイマーを開始します.
    m_Timer.Start();

    // 開始時刻を取得.
    m_LatestUpdateTime = m_Timer.GetElapsedTime();

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      ウィンドウの終了処理.
//-----------------------------------------------------------------------------
void Application::TermWnd()
{
    // タイマーを止めます.
    m_Timer.Stop();

    // ウィンドウクラスの登録を解除.
    if ( m_hInst != nullptr )
    { UnregisterClass( ASDX_WND_CLASSNAME, m_hInst ); }

    if ( m_hAccel )
    { DestroyAcceleratorTable( m_hAccel ); }

    if ( m_hMenu )
    { DestroyMenu( m_hMenu ); }

    if ( m_hIcon )
    { DestroyIcon( m_hIcon ); }

    // タイトル名をクリア.
    m_Title = nullptr;

    // ハンドルをクリア.
    m_hInst  = nullptr;
    m_hWnd   = nullptr;
    m_hIcon  = nullptr;
    m_hMenu  = nullptr;
    m_hAccel = nullptr;

    // アプリケーションリストから削除します.
    g_AppList.Remove( this );
}

//-----------------------------------------------------------------------------
//      Direct3Dの初期化処理.
//-----------------------------------------------------------------------------
bool Application::InitD3D()
{
    HRESULT hr = S_OK;

    // ウィンドウサイズを取得します.
    RECT rc;
    GetClientRect( m_hWnd, &rc );
    UINT w = rc.right - rc.left;
    UINT h = rc.bottom - rc.top;

    // 取得したサイズを設定します.
    m_Width       = w;
    m_Height      = h;

    // アスペクト比を算出します.
    m_AspectRatio = (FLOAT)w / (FLOAT)h;

    // デバイスの初期化.
    if (!GfxDevice().Init(&m_DeviceDesc))
    {
        ELOG("Error : GraphicsDeivce::Init() Failed.");
        return false;
    }

    // スワップチェインの初期化
    {
        DXGI_RATIONAL refreshRate;
        GetDisplayRefreshRate(refreshRate);

        // スワップチェインの構成設定.
        DXGI_SWAP_CHAIN_DESC1 desc = {};
        desc.Width              = w;
        desc.Height             = h;
        desc.Format             = m_SwapChainFormat;
        desc.Stereo             = FALSE;
        desc.SampleDesc.Count   = m_MultiSampleCount;
        desc.SampleDesc.Quality = m_MultiSampleQuality;
        desc.BufferCount        = m_SwapChainCount;
        desc.Scaling            = DXGI_SCALING_STRETCH;
        desc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.Flags              = (m_AllowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc = {};
        fullScreenDesc.RefreshRate      = refreshRate;
        fullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        fullScreenDesc.Scaling          = DXGI_MODE_SCALING_STRETCHED;
        fullScreenDesc.Windowed         = TRUE;

        RefPtr<IDXGISwapChain1> pSwapChain1;
        auto pQueue = GfxDevice().GetGraphicsQueue()->GetQueue();
        hr = GfxDevice().GetFactory()->CreateSwapChainForHwnd(pQueue, m_hWnd, &desc, &fullScreenDesc, nullptr, pSwapChain1.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : IDXGIFactory2::CreateSwapChainForHwnd() Failed. errcode = 0x%x", hr);
            return false;
        }

        if (m_AllowTearing)
        { GfxDevice().GetFactory()->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER); }

        // IDXGISwapChain4にキャスト.
        hr = pSwapChain1->QueryInterface(IID_PPV_ARGS(m_pSwapChain4.GetAddress())); // MEMO : QueryInterface() を実行すると何故かメモリリークが発生するようになる.
        if ( FAILED( hr ) )
        {
            m_pSwapChain4.Reset();
            ELOG( "Warning : IDXGISwapChain4 Conversion Faild.");
        }
        else
        {
            wchar_t name[] = L"asdxSwapChain4\0";
            m_pSwapChain4->SetPrivateData(WKPDID_D3DDebugObjectNameW, sizeof(name), name);

            // HDR出力チェック.
            CheckSupportHDR();
        }
    }

    // カラーターゲットの初期化.
    {
        m_ColorTarget.resize(m_SwapChainCount);

        for(auto i=0u; i<m_SwapChainCount; ++i)
        {
            if (!m_ColorTarget[i].Init(GfxDevice(), m_pSwapChain4.GetPtr(), i, true))
            {
                ELOG("Error : ColorTarget::Init() Failed.");
                return false;
            }
        }
    }

    // 深度ターゲットの初期化.
    {
        TargetDesc desc;
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Alignment          = 0;
        desc.Width              = w;
        desc.Height             = h;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = m_DepthStencilFormat;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.InitState          = D3D12_RESOURCE_STATE_DEPTH_WRITE;

        if (!m_DepthTarget.Init(GfxDevice(), &desc))
        {
            ELOG("Error : DepthTarget::Init() Failed.");
            return false;
        }
    }

    // ビューポートの設定.
    m_Viewport.Width    = (FLOAT)w;
    m_Viewport.Height   = (FLOAT)h;
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;
    m_Viewport.TopLeftX = 0;
    m_Viewport.TopLeftY = 0;

    // シザー矩形の設定.
    m_ScissorRect.left   = 0;
    m_ScissorRect.right  = w;
    m_ScissorRect.top    = 0;
    m_ScissorRect.bottom = h;

    return true;
}

//-----------------------------------------------------------------------------
//      Direct3Dの終了処理.
//-----------------------------------------------------------------------------
void Application::TermD3D()
{
    for(size_t i=0; i<m_ColorTarget.size(); ++i)
    {
        m_ColorTarget[i].Term();
    }
    m_ColorTarget.clear();
    m_DepthTarget.Term();
    m_pSwapChain4.Reset();
    GraphicsDevice::Instance().Term();
}


//-----------------------------------------------------------------------------
//      メインループ処理.
//-----------------------------------------------------------------------------
void Application::MainLoop()
{
    MSG msg = { 0 };

    FrameEventArgs frameEventArgs;

    auto frameCount = 0;

    while( WM_QUIT != msg.message )
    {
        auto gotMsg = PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE );

        if ( gotMsg )
        {
            auto ret = TranslateAccelerator( m_hWnd, m_hAccel, &msg );
            if ( 0 == ret )
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        }
        else
        {
            double time;
            double absTime;
            double elapsedTime;

            // 時間を取得.
            m_Timer.Update( time, absTime, elapsedTime );

            // 0.5秒ごとにFPSを更新.
            auto interval = float( time - m_LatestUpdateTime );
            if ( interval > 0.5 )
            {
                // FPSを算出.
                m_FPS = frameCount / interval;

                // 更新時間を設定.
                m_LatestUpdateTime = time;

                frameCount = 0;
            }

            frameEventArgs.FPS             = 1.0f / (float)elapsedTime;   // そのフレームにおけるFPS.
            frameEventArgs.Time            = time;
            frameEventArgs.ElapsedTime     = elapsedTime;
            frameEventArgs.IsStopDraw      = m_IsStopRendering;

            // フレーム遷移処理.
            OnFrameMove( frameEventArgs );

            // 描画停止フラグが立っていない場合.
            if ( !IsStopRendering() )
            {
                // フレーム描画処理.
                OnFrameRender( frameEventArgs );

                // フレームカウントをインクリメント.
                m_FrameCount++;
            }

            frameCount++;
        }
    }
}

//-----------------------------------------------------------------------------
//      アプリケーションを実行します.
//-----------------------------------------------------------------------------
void Application::Run()
{
    // アプリケーションの初期化処理.
    if ( InitApp() )
    {
        // メインループ処理.
        //MainLoop();
    }

    // アプリケーションの終了処理.
    TermApp();
}

//-----------------------------------------------------------------------------
//      キーイベント処理.
//-----------------------------------------------------------------------------
void Application::KeyEvent( const KeyEventArgs& param )
{
    // キーイベント呼び出し.
    OnKey( param );
}

//-----------------------------------------------------------------------------
//      リサイズイベント処理.
//-----------------------------------------------------------------------------
void Application::ResizeEvent( const ResizeEventArgs& param )
{
    //if (m_pSwapChain4.GetPtr() == nullptr)
    { return; }

    if (m_ColorTarget.empty())
    { return; }

    if (m_DepthTarget.GetResource() == nullptr)
    { return; }

    // マルチサンプル数以下になるとハングすることがあるので，処理をスキップする.
    if ( param.Width  <= m_MultiSampleCount 
      || param.Height <= m_MultiSampleCount)
    { return; }

    m_Width       = param.Width;
    m_Height      = param.Height;
    m_AspectRatio = param.AspectRatio;

    // ビューポートの設定.
    m_Viewport.Width    = (FLOAT)m_Width;
    m_Viewport.Height   = (FLOAT)m_Height;
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;
    m_Viewport.TopLeftX = 0;
    m_Viewport.TopLeftY = 0;

    // シザー矩形の設定.
    m_ScissorRect.left   = 0;
    m_ScissorRect.right  = m_Width;
    m_ScissorRect.top    = 0;
    m_ScissorRect.bottom = m_Height;

    if ( m_pSwapChain4 != nullptr )
    {
        // リサイズ前に一度コマンドを実行(実行しないとメモリリークするぽい).
        //m_pSwapChain4->Present( 0, 0 );

        // コマンドの完了を待機.
        //GfxDevice().GetGraphicsQueue()->Wait(asdx::Queue::kInfinite);

        // 描画ターゲットを解放.
        for(size_t i=0; i<m_ColorTarget.size(); ++i)
        { m_ColorTarget[i].Term(); }

        // 深度ステンシルバッファを解放.
        m_DepthTarget.Term();

        HRESULT hr = S_OK;

        // バッファをリサイズ.
        hr = m_pSwapChain4->ResizeBuffers( m_SwapChainCount, m_Width, m_Height, m_SwapChainFormat, 0 );
        if ( FAILED( hr ) )
        { DLOG( "Error : IDXGISwapChain::ResizeBuffer() Failed." ); }

        for(auto i=0u; i<m_SwapChainCount; ++i)
        {
            if (!m_ColorTarget[i].Init(GfxDevice(), m_pSwapChain4.GetPtr(), i, true))
            { DLOG("Error : ColorTarget::Init() Failed."); }
        }

        TargetDesc desc;
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Alignment          = 0;
        desc.Width              = m_Width;
        desc.Height             = m_Height;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = m_DepthStencilFormat;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.InitState          = D3D12_RESOURCE_STATE_DEPTH_WRITE;

        if ( !m_DepthTarget.Init(GfxDevice(), &desc))
        { DLOG( "Error : DepthStencilTarget::Create() Failed." ); }
    }

    // リサイズイベント呼び出し.
    OnResize( param );
}

//-----------------------------------------------------------------------------
//      マウスイベント処理.
//-----------------------------------------------------------------------------
void Application::MouseEvent( const MouseEventArgs& param )
{
    OnMouse( param );
}

//-----------------------------------------------------------------------------
//      ドロップイベント処理.
//------------------------------------------------------------------------------
void Application::DropEvent( const wchar_t** dropFiles, uint32_t fileNum )
{
    OnDrop( dropFiles, fileNum );
}

//-----------------------------------------------------------------------------
//      ウィンドウプロシージャ.
//-----------------------------------------------------------------------------
LRESULT CALLBACK Application::MsgProc( HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp )
{
    PAINTSTRUCT ps;
    HDC         hdc;

    if ( ( uMsg == WM_KEYDOWN )
      || ( uMsg == WM_SYSKEYDOWN )
      || ( uMsg == WM_KEYUP )
      || ( uMsg == WM_SYSKEYUP ) )
    {
        bool isKeyDown = ( uMsg == WM_KEYDOWN  || uMsg == WM_SYSKEYDOWN );

        DWORD mask = ( 1 << 29 );
        bool isAltDown =( ( lp & mask ) != 0 );

        KeyEventArgs args;
        args.KeyCode   = uint32_t( wp );
        args.IsAltDown = isAltDown;
        args.IsKeyDown = isKeyDown;

        for( ApplicationList::ListItr itr = g_AppList.Begin(); itr != g_AppList.End(); itr++ )
        {
            (*itr)->KeyEvent( args );
        }
    }

    // 古いWM_MOUSEWHEELの定義.
    const UINT OLD_WM_MOUSEWHEEL = 0x020A;

    if ( ( uMsg == WM_LBUTTONDOWN )
      || ( uMsg == WM_LBUTTONUP )
      || ( uMsg == WM_LBUTTONDBLCLK )
      || ( uMsg == WM_MBUTTONDOWN )
      || ( uMsg == WM_MBUTTONUP )
      || ( uMsg == WM_MBUTTONDBLCLK )
      || ( uMsg == WM_RBUTTONDOWN )
      || ( uMsg == WM_RBUTTONUP )
      || ( uMsg == WM_RBUTTONDBLCLK )
      || ( uMsg == WM_XBUTTONDOWN )
      || ( uMsg == WM_XBUTTONUP )
      || ( uMsg == WM_XBUTTONDBLCLK )
      || ( uMsg == WM_MOUSEHWHEEL )             // このWM_MOUSEWHEELは0x020Eを想定.
      || ( uMsg == WM_MOUSEMOVE )
      || ( uMsg == OLD_WM_MOUSEWHEEL ) )
    {
        int x = (short)LOWORD( lp );
        int y = (short)HIWORD( lp );

        int wheelDelta = 0;
        if ( ( uMsg == WM_MOUSEHWHEEL )
          || ( uMsg == OLD_WM_MOUSEWHEEL ) )
        {
            POINT pt;
            pt.x = x;
            pt.y = y;

            ScreenToClient( hWnd, &pt );
            x = pt.x;
            y = pt.y;

            wheelDelta += (short)HIWORD( wp );
        }

        int  buttonState = LOWORD( wp );
        bool isLeftButtonDown   = ( ( buttonState & MK_LBUTTON  ) != 0 );
        bool isRightButtonDown  = ( ( buttonState & MK_RBUTTON  ) != 0 );
        bool isMiddleButtonDown = ( ( buttonState & MK_MBUTTON  ) != 0 );
        bool isSideButton1Down  = ( ( buttonState & MK_XBUTTON1 ) != 0 );
        bool isSideButton2Down  = ( ( buttonState & MK_XBUTTON2 ) != 0 );

        MouseEventArgs args;
        args.X = x;
        args.Y = y;
        args.IsLeftButtonDown   = isLeftButtonDown;
        args.IsMiddleButtonDown = isMiddleButtonDown;
        args.IsRightButtonDown  = isRightButtonDown;
        args.IsSideButton1Down  = isSideButton1Down;
        args.IsSideButton2Down  = isSideButton2Down;

        for( ApplicationList::ListItr itr = g_AppList.Begin(); itr != g_AppList.End(); itr++ )
        {
            (*itr)->MouseEvent( args );
        }
    }

    switch( uMsg )
    {
    case WM_CREATE:
        {
            // ドラッグアンドドロップ可能.
            DragAcceptFiles(hWnd, TRUE);
        }
        break;

    case WM_PAINT:
        {
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
        }
        break;

    case WM_DESTROY:
        { PostQuitMessage( 0 ); }
        break;

    case WM_SIZE:
        {
            UINT w = (UINT)LOWORD( lp );
            UINT h = (UINT)HIWORD( lp );

            // ウインドウ非表示状態に移行する時に縦横1ピクセルのリサイズイベントが発行される
            // マルチサンプル等の関係で縦横1ピクセルは問題が起こるので最少サイズを設定
            ResizeEventArgs args;
            args.Width  = asdx::Max( w, (uint32_t)8 );
            args.Height = asdx::Max( h, (uint32_t)8 );
            args.AspectRatio = float( args.Width ) / args.Height;

            for( ApplicationList::ListItr itr = g_AppList.Begin(); itr != g_AppList.End(); itr++ )
            {
                (*itr)->ResizeEvent( args );
            }
        }
        break;

    case WM_DROPFILES:
        {
            // ドロップされたファイル数を取得.
            uint32_t numFiles = DragQueryFileW((HDROP)wp, 0xFFFFFFFF, NULL, 0);

            // 作業用のバッファを確保.
            const WCHAR** dropFiles = new const WCHAR*[ numFiles ];

            for (uint32_t i=0; i < numFiles; i++)
            {
                // ドロップされたファイル名を取得.
                WCHAR* dropFile = new WCHAR[ MAX_PATH ];
                DragQueryFileW((HDROP)wp, i, dropFile, MAX_PATH);
                dropFiles[ i ] = dropFile;
            }

            // アプリケーションに通知.
            for( ApplicationList::ListItr itr = g_AppList.Begin(); itr != g_AppList.End(); itr++ )
            {
                (*itr)->DropEvent( dropFiles, numFiles );
            }

            // 作業用のバッファを解放.
            for (uint32_t i=0; i < numFiles; i++)
            { SafeDelete( dropFiles[ i ] ); }
            SafeDelete( dropFiles );

            DragFinish((HDROP)wp);
        }
        break;

    case WM_MOVE:
        {
            for( ApplicationList::ListItr itr = g_AppList.Begin(); itr != g_AppList.End(); itr++ )
            {
                (*itr)->CheckSupportHDR();
            }
        }
        break;

    case WM_DISPLAYCHANGE:
        {
            for( ApplicationList::ListItr itr = g_AppList.Begin(); itr != g_AppList.End(); itr++ )
            {
                (*itr)->CheckSupportHDR();
            }
        }
        break;

    case WM_CHAR:
        {
            auto keyCode = static_cast<uint32_t>( wp );
            for( ApplicationList::ListItr itr = g_AppList.Begin(); itr != g_AppList.End(); itr++ )
            {
                (*itr)->OnTyping( keyCode );
            }
        }
        break;

    //case MM_MCINOTIFY:
    //    {
    //        // サウンドマネージャのコールバック.
    //        SndMgr::GetInstance().OnNofity( (uint32_t)lp, (uint32_t)wp );
    //    }
    //    break;
    }

    // ユーザーカスタマイズ用に呼び出し.
    for( ApplicationList::ListItr itr = g_AppList.Begin(); itr != g_AppList.End(); itr++ )
    { (*itr)->OnMsgProc( hWnd, uMsg, wp, lp ); }

    return DefWindowProc( hWnd, uMsg, wp, lp );
}

//-----------------------------------------------------------------------------
//      初期化時の処理.
//-----------------------------------------------------------------------------
bool Application::OnInit()
{
    /* DO_NOTHING */
    return true;
}

//-----------------------------------------------------------------------------
//      終了時の処理.
//-----------------------------------------------------------------------------
void Application::OnTerm()
{
    /* DO_NOTHING */
}

//-----------------------------------------------------------------------------
//      フレーム遷移時の処理.
//-----------------------------------------------------------------------------
void Application::OnFrameMove( FrameEventArgs& )
{
    /* DO_NOTHING */
}

//-----------------------------------------------------------------------------
//      フレーム描画字の処理.
//-----------------------------------------------------------------------------
void Application::OnFrameRender( FrameEventArgs& )
{
    /* DO_NOTHING */
}

//-----------------------------------------------------------------------------
//      コマンドを実行して，画面に表示します.
//-----------------------------------------------------------------------------
void Application::Present( uint32_t syncInterval )
{
    HRESULT hr = S_OK;

    // スタンバイモードかどうかチェック.
    if ( m_IsStandbyMode )
    {
        // テストする.
        hr = m_pSwapChain4->Present( syncInterval, DXGI_PRESENT_TEST );

        // スタンバイモードが解除されたかをチェック.
        if ( hr == S_OK )
        { m_IsStandbyMode = false; }

        // 処理を中断.
        return;
    }

    // 画面更新する.
    hr = m_pSwapChain4->Present( syncInterval, 0 );

    switch( hr )
    {
    // デバイスがリセットされた場合(=コマンドが正しくない場合)
    case DXGI_ERROR_DEVICE_RESET:
        {
            // エラーログ出力.
            ELOG( "Fatal Error : IDXGISwapChain::Present() Failed. ErrorCode = DXGI_ERROR_DEVICE_RESET." );

            // 続行できないのでダイアログを表示.
            MessageBoxW( m_hWnd, L"A Fatal Error Occured. Shutting down.", L"FATAL ERROR", MB_OK | MB_ICONERROR );

            // 終了メッセージを送る.
            PostQuitMessage( 1 );
        }
        break;

    // デバイスが削除された場合(=GPUがぶっこ抜かれた場合かドライバーアップデート中)
    case DXGI_ERROR_DEVICE_REMOVED:
        {
            // エラーログ出力.
            ELOG( "Fatal Error : IDXGISwapChain::Present() Failed. ErrorCode = DXGI_ERROR_DEVICE_REMOVED." );

            // 続行できないのでダイアログを表示.
            MessageBoxW( m_hWnd, L"A Fatal Error Occured. Shutting down.", L"FATAL ERROR", MB_OK | MB_ICONERROR );

            // 終了メッセージを送る.
            PostQuitMessage( 2 );
        }
        break;

    // 表示領域がなければスタンバイモードに入る.
    case DXGI_STATUS_OCCLUDED:
        { m_IsStandbyMode = true; }
        break;

    // 現在のフレームバッファを表示する場合.
    case S_OK:
        { /* DO_NOTHING */ }
        break;
    }
}

//-----------------------------------------------------------------------------
//      ディスプレイがHDR出力をサポートしているかどうかチェックします.
//-----------------------------------------------------------------------------
void Application::CheckSupportHDR()
{
    HRESULT hr = S_OK;

    // ウィンドウ領域を取得.
    RECT rect;
    GetWindowRect(m_hWnd, &rect);

    RefPtr<IDXGIAdapter1> pAdapter;
    hr = GfxDevice().GetFactory()->EnumAdapters1(0, pAdapter.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : IDXGIFactory1::EnumAdapters1() Failed.");
        return;
    }

    UINT i = 0;
    RefPtr<IDXGIOutput> currentOutput;
    RefPtr<IDXGIOutput> bestOutput;
    int bestIntersectArea = -1;

    // 各ディスプレイを調べる.
    while (pAdapter->EnumOutputs(i, currentOutput.GetAddress()) != DXGI_ERROR_NOT_FOUND)
    {
        auto ax1 = rect.left;
        auto ay1 = rect.top;
        auto ax2 = rect.right;
        auto ay2 = rect.bottom;

        // ディスプレイの設定を取得.
        DXGI_OUTPUT_DESC desc;
        hr = currentOutput->GetDesc(&desc);
        if (FAILED(hr))
        { return; }

        auto bx1 = desc.DesktopCoordinates.left;
        auto by1 = desc.DesktopCoordinates.top;
        auto bx2 = desc.DesktopCoordinates.right;
        auto by2 = desc.DesktopCoordinates.bottom;

        // 領域が一致するかどうか調べる.
        int intersectArea = ComputeIntersectionArea(ax1, ay1, ax2, ay2, bx1, by1, bx2, by2);
        if (intersectArea > bestIntersectArea)
        {
            bestOutput = currentOutput;
            bestIntersectArea = intersectArea;
        }

        i++;
    }

    // 一番適しているディスプレイ.
    RefPtr<IDXGIOutput6> pOutput6;
    hr = bestOutput->QueryInterface(IID_PPV_ARGS(pOutput6.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : IDXGIOutput6 Conversion Failed.");
        return;
    }

    // 出力設定を取得.
    hr = pOutput6->GetDesc1(&m_DisplayDesc);
    if (FAILED(hr))
    {
        ELOG("Error : IDXGIOutput6::GetDesc() Failed.");
        return;
    }

    // 正常終了.
}

//-----------------------------------------------------------------------------
//      HDR出力をサポートしているかどうかチェックします.
//-----------------------------------------------------------------------------
bool Application::IsSupportHDR() const
{ return m_DisplayDesc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020; }

//-----------------------------------------------------------------------------
//      ディスプレイ設定を取得します.
//-----------------------------------------------------------------------------
DXGI_OUTPUT_DESC1 Application::GetDisplayDesc() const
{ return m_DisplayDesc; }

//-----------------------------------------------------------------------------
//      色空間を設定します
//-----------------------------------------------------------------------------
bool Application::SetColorSpace(COLOR_SPACE value)
{
    if (m_pSwapChain4.GetPtr() == nullptr)
    { return false; }

    DXGI_HDR_METADATA_HDR10 metaData = {};
    DXGI_COLOR_SPACE_TYPE colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

    metaData.MinMasteringLuminance = GetLuma(m_DisplayDesc.MinLuminance);
    metaData.MaxMasteringLuminance = GetLuma(m_DisplayDesc.MaxLuminance);

    switch (value)
    {
    case COLOR_SPACE_NONE:
        {
            colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
            metaData.RedPrimary[0]   = GetCoord(m_DisplayDesc.RedPrimary[0]);
            metaData.RedPrimary[1]   = GetCoord(m_DisplayDesc.RedPrimary[1]);
            metaData.GreenPrimary[0] = GetCoord(m_DisplayDesc.GreenPrimary[0]);
            metaData.GreenPrimary[1] = GetCoord(m_DisplayDesc.GreenPrimary[1]);
            metaData.BluePrimary[0]  = GetCoord(m_DisplayDesc.BluePrimary[0]);
            metaData.BluePrimary[1]  = GetCoord(m_DisplayDesc.BluePrimary[1]);
            metaData.WhitePoint[0]   = GetCoord(m_DisplayDesc.WhitePoint[0]);
            metaData.WhitePoint[1]   = GetCoord(m_DisplayDesc.WhitePoint[1]);
        }
        break;

    case COLOR_SPACE_SRGB:
        {
            colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
            metaData.RedPrimary[0]   = GetCoord(0.64000f);
            metaData.RedPrimary[1]   = GetCoord(0.33000f);
            metaData.GreenPrimary[0] = GetCoord(0.30000f);
            metaData.GreenPrimary[1] = GetCoord(0.60000f);
            metaData.BluePrimary[0]  = GetCoord(0.15000f);
            metaData.BluePrimary[1]  = GetCoord(0.06000f);
            metaData.WhitePoint[0]   = GetCoord(0.31270f);
            metaData.WhitePoint[1]   = GetCoord(0.32900f);
        }
        break;

    case COLOR_SPACE_BT709:
        {
            colorSpace = DXGI_COLOR_SPACE_RGB_STUDIO_G24_NONE_P709;
            metaData.RedPrimary[0]   = GetCoord(0.64000f);
            metaData.RedPrimary[1]   = GetCoord(0.33000f);
            metaData.GreenPrimary[0] = GetCoord(0.30000f);
            metaData.GreenPrimary[1] = GetCoord(0.60000f);
            metaData.BluePrimary[0]  = GetCoord(0.15000f);
            metaData.BluePrimary[1]  = GetCoord(0.06000f);
            metaData.WhitePoint[0]   = GetCoord(0.31270f);
            metaData.WhitePoint[1]   = GetCoord(0.32900f);
        }
        break;

    case COLOR_SPACE_BT2100_PQ:
        {
            colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
            metaData.RedPrimary[0]   = GetCoord(0.70800f);
            metaData.RedPrimary[1]   = GetCoord(0.29200f);
            metaData.GreenPrimary[0] = GetCoord(0.17000f);
            metaData.GreenPrimary[1] = GetCoord(0.79700f);
            metaData.BluePrimary[0]  = GetCoord(0.13100f);
            metaData.BluePrimary[1]  = GetCoord(0.04600f);
            metaData.WhitePoint[0]   = GetCoord(0.31270f);
            metaData.WhitePoint[1]   = GetCoord(0.32900f);
        }
        break;

    case COLOR_SPACE_BT2100_HLG:
        {
            colorSpace = DXGI_COLOR_SPACE_YCBCR_FULL_GHLG_TOPLEFT_P2020;
            metaData.RedPrimary[0]   = GetCoord(0.70800f);
            metaData.RedPrimary[1]   = GetCoord(0.29200f);
            metaData.GreenPrimary[0] = GetCoord(0.17000f);
            metaData.GreenPrimary[1] = GetCoord(0.79700f);
            metaData.BluePrimary[0]  = GetCoord(0.13100f);
            metaData.BluePrimary[1]  = GetCoord(0.04600f);
            metaData.WhitePoint[0]   = GetCoord(0.31270f);
            metaData.WhitePoint[1]   = GetCoord(0.32900f);
        }
        break;
    }

    UINT flag = 0;
    auto hr = m_pSwapChain4->CheckColorSpaceSupport(colorSpace, &flag);
    if (FAILED(hr))
    {
        ELOG("Error : ISwapChain4::CheckColorSpaceSupport() Failed. errcode = 0x%x", hr);
        return false;
    }

    if ((flag & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT) != DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT)
    { return false; }

    hr = m_pSwapChain4->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(metaData), &metaData);
    if (FAILED(hr))
    {
        ELOG("Error : ISwapChain4::SetHDRMetaData() Failed. errcode = 0x%x", hr);
        return false;
    }

    hr = m_pSwapChain4->SetColorSpace1(colorSpace);
    if (FAILED(hr))
    {
        ELOG("Error : ISwapChain4::SetColorSpace1() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      ディスプレイのリフレッシュレートを取得します.
//-----------------------------------------------------------------------------
bool Application::GetDisplayRefreshRate(DXGI_RATIONAL& result) const
{
    auto hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);

    MONITORINFOEX monitorInfo;
    monitorInfo.cbSize = sizeof(monitorInfo);
    auto ret = GetMonitorInfo(hMonitor, &monitorInfo);
    if (ret == 0)
    {
        ELOG("Error : GetMonitorInfo() Failed.");
        return false;
    }
    
    DEVMODE devMode;
    devMode.dmSize          = sizeof(devMode);
    devMode.dmDriverExtra   = 0;
    ret = EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode);
    if (ret == 0)
    {
        ELOG("Error : EnumDisplaySettings() Failed.");
        return false;
    }
    
    auto useDefaultRefreshRate = (1 == devMode.dmDisplayFrequency) || (0 == devMode.dmDisplayFrequency);
    result.Numerator   = (useDefaultRefreshRate) ? 0 : devMode.dmDisplayFrequency;
    result.Denominator = (useDefaultRefreshRate) ? 0 : 1;

    return true;
}

//-----------------------------------------------------------------------------
//      リサイズ時の処理.
//-----------------------------------------------------------------------------
void Application::OnResize( const ResizeEventArgs& )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      キーイベント時の処理.
//-----------------------------------------------------------------------------
void Application::OnKey( const KeyEventArgs& )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      マウスイベント時の処理.
//-----------------------------------------------------------------------------
void Application::OnMouse( const MouseEventArgs& )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      タイピングイベント時の処理.
//-----------------------------------------------------------------------------
void Application::OnTyping( uint32_t )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      ドロップ時の処理.
//------------------------------------------------------------------------------
void Application::OnDrop( const wchar_t**, uint32_t )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      メッセージプロシージャの処理.
//-----------------------------------------------------------------------------
void Application::OnMsgProc( HWND, UINT, WPARAM, LPARAM )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      フォーカスを持つかどうか判定します.
//-----------------------------------------------------------------------------
bool Application::HasFocus() const
{ return ( GetActiveWindow() == m_hWnd ); }

//------------------------------------------------------------------------------
//      スタンバイモードかどうかチェックします.
//------------------------------------------------------------------------------
bool Application::IsStandByMode() const
{ return m_IsStandbyMode; }

} // namespace asdx
