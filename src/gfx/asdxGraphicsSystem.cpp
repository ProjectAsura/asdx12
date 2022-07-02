//-----------------------------------------------------------------------------
// File : asdxGraphicsDevice.cpp
// Desc : Graphics Device.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxGraphicsSystem.h>
#include <gfx/asdxCommandQueue.h>
#include <gfx/asdxDescriptor.h>
#include <gfx/asdxDisposer.h>
#include <gfx/asdxCommandList.h>
//#include <gfx/asdxResourceUploader.h>
#include <fnd/asdxSpinLock.h>
#include <fnd/asdxRef.h>
#include <fnd/asdxLogger.h>
#include <ShlObj.h>
#include <strsafe.h>


namespace {

//-------------------------------------------------------------------------------------------------
//      PIXキャプチャー用のDLLをロードします.
//-------------------------------------------------------------------------------------------------
void LoadPixGpuCpatureDll()
{
    LPWSTR programFilesPath = nullptr;
    SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

    wchar_t pixSearchPath[MAX_PATH] = {};
    StringCchCopy(pixSearchPath, MAX_PATH, programFilesPath);
    StringCchCat(pixSearchPath, MAX_PATH, L"\\Microsoft PIX\\*");

    WIN32_FIND_DATA findData;
    bool foundPixInstallation = false;
    wchar_t newestVersionFound[MAX_PATH] = {};

    HANDLE hFind = FindFirstFile(pixSearchPath, &findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do 
        {
            if (((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) &&
                 (findData.cFileName[0] != '.'))
            {
                if (!foundPixInstallation || wcscmp(newestVersionFound, findData.cFileName) <= 0)
                {
                    foundPixInstallation = true;
                    StringCchCopy(newestVersionFound, _countof(newestVersionFound), findData.cFileName);
                }
            }
        } 
        while (FindNextFile(hFind, &findData) != 0);
    }

    FindClose(hFind);

    if (!foundPixInstallation)
    {
        return;
    }

    wchar_t dllPath[MAX_PATH] = {};
    StringCchCopy(dllPath, wcslen(pixSearchPath), pixSearchPath);
    StringCchCat(dllPath, MAX_PATH, &newestVersionFound[0]);
    StringCchCat(dllPath, MAX_PATH, L"\\WinPixGpuCapturer.dll");

    if (GetModuleHandleW(L"WinPixGpuCapturer.dll") == 0)
    {
        LoadLibraryW(dllPath);
    }
}

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// GraphicsSystem class
///////////////////////////////////////////////////////////////////////////////
class GraphicsSystem
{
    //========================================================================
    // list of friend classes and methods.
    //========================================================================
    /* NOTHING */

public:

    //=========================================================================
    // public variables.
    //=========================================================================


    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      唯一のインスタンスを取得します.
    //!
    //! @return     唯一のインスタンスを返却します.
    //-------------------------------------------------------------------------
    static GraphicsSystem& Instance();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      desc        構成設定です.
    //! @retval true    初期化に成功.
    //! @retval fasle   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(const DeviceDesc& desc);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      ID3D12Device8を取得します.
    //!
    //! @return     ID3D12Device8を返却します.
    //-------------------------------------------------------------------------
    ID3D12Device8* GetDevice() const;

    //-------------------------------------------------------------------------
    //! @brief      IDXGIFactory7を取得します.
    //!
    //! @return     IDXGIFactory7を返却します.
    //-------------------------------------------------------------------------
    IDXGIFactory7* GetFactory() const;

    //-------------------------------------------------------------------------
    //! @brief      グラフィックスキューを取得します.
    //!
    //! @return     グラフィックスキューを返却します.
    //-------------------------------------------------------------------------
    CommandQueue* GetGraphicsQueue() const;

    //-------------------------------------------------------------------------
    //! @brief      コンピュートキューを取得します.
    //!
    //! @return     コンピュートキューを返却します.
    //-------------------------------------------------------------------------
    CommandQueue* GetComputeQueue() const;

    //-------------------------------------------------------------------------
    //! @brief      コピーキューを取得します.
    //!
    //! @return     コピーキューを返却します.
    //-------------------------------------------------------------------------
    CommandQueue* GetCopyQueue() const;

    //-------------------------------------------------------------------------
    //! @brief      ビデオデコードキューを取得します.
    //!
    //! @return     ビデオデコードキューを返却します.
    //-------------------------------------------------------------------------
    CommandQueue* GetVideoDecodeQueue() const;

    //-------------------------------------------------------------------------
    //! @brief      ビデオプロセスキューを取得します.
    //!
    //! @return     ビデオプロセスキューを返却します.
    //-------------------------------------------------------------------------
    CommandQueue* GetVideoProcessQueue() const;

    //-------------------------------------------------------------------------
    //! @brief      ビデオエンコードキューを取得します.
    //!
    //! @return     ビデオエンコードキューを返却します.
    //-------------------------------------------------------------------------
    CommandQueue* GetVideoEncodeQueue() const;

    //-------------------------------------------------------------------------
    //! @brief      ディスクリプターを確保します.
    //!
    //! @param[in]      index       ディスクリプタタイプです.
    //! @param[out]     ppResult    ディスクリプタの確保先です.
    //! @retval true    確保に成功.
    //! @retval false   確保に失敗.
    //-------------------------------------------------------------------------
    bool AllocHandle(uint8_t heapType, Descriptor** ppResult);

    //-------------------------------------------------------------------------
    //! @brief      アロー演算子です.
    //-------------------------------------------------------------------------
    ID3D12Device8* operator-> () const;

    //-------------------------------------------------------------------------
    //! @brief      ディスクリプタヒープを設定します.
    //-------------------------------------------------------------------------
    void SetDescriptorHeaps(ID3D12GraphicsCommandList* pCmdList);

    //-------------------------------------------------------------------------
    //! @brief      コマンドキューの実行完了を待機します.
    //-------------------------------------------------------------------------
    void WaitIdle();

    //-------------------------------------------------------------------------
    //! @brief      オブジェクトディスポーザーに追加します.
    //!
    //! @param[in]      pResource       破棄リソース.
    //! @param[in]      lifeTime        生存フレーム数です.
    //--------------------------------------------------------------------------
    void Dispose(ID3D12Object*& pResource, uint8_t lifeTime);

    //-------------------------------------------------------------------------
    //! @brief      ディスクリプタディスポーザーに追加します.
    //!
    //! @param[in]      pDescriptor     破棄ディスクリプタ.
    //! @param[in]      lifeTime        生存フレーム数です.
    //-------------------------------------------------------------------------
    void Dispose(Descriptor*& pDescriptor, uint8_t lifeTime);

    //-------------------------------------------------------------------------
    //! @brief      フレーム同期を行います.
    //-------------------------------------------------------------------------
    void FrameSync();

    //-------------------------------------------------------------------------
    //! @brief      強制破棄を行います.
    //-------------------------------------------------------------------------
    void ClearDisposer();

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    static GraphicsSystem           s_Instance;                 //!< シングルトンインスタンス.
    RefPtr<IDXGIFactory7>           m_pFactory;                 //!< DXGIファクトリーです.
    RefPtr<ID3D12Debug3>            m_pDebug;                   //!< デバッグオブジェクト.
    RefPtr<ID3D12InfoQueue>         m_pInfoQueue;               //!< インフォキュー.
    RefPtr<ID3D12Device8>           m_pDevice;                  //!< デバイス.
    RefPtr<CommandQueue>            m_pGraphicsQueue;           //!< グラフィックスキュー.
    RefPtr<CommandQueue>            m_pComputeQueue;            //!< コンピュートキュー.
    RefPtr<CommandQueue>            m_pCopyQueue;               //!< コピーキュー.
    RefPtr<CommandQueue>            m_pVideoDecodeQueue;        //!< ビデオデコードキュー.
    RefPtr<CommandQueue>            m_pVideoProcessQueue;       //!< ビデオプロセスキュー.
    RefPtr<CommandQueue>            m_pVideoEncodeQueue;        //!< ビデオエンコードキュー.
    DescriptorHeap                  m_DescriptorHeap[4];        //!< ディスクリプタヒープ.
    Disposer<ID3D12Object>          m_ObjectDisposer;           //!< オブジェクトディスポーザー.
    Disposer<Descriptor>            m_DescriptorDisposer;       //!< ディスクリプタディスポーザー.
    SpinLock                        m_SpinLock;                 //!< スピンロックです.

    //=========================================================================
    // private methods
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    GraphicsSystem() = default;

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~GraphicsSystem() = default;

    GraphicsSystem              (const GraphicsSystem&) = delete;   // アクセス禁止.
    GraphicsSystem& operator =  (const GraphicsSystem&) = delete;   // アクセス禁止.
};


GraphicsSystem GraphicsSystem::s_Instance = {};

//-----------------------------------------------------------------------------
//      唯一のインスタンスを取得します.
//-----------------------------------------------------------------------------
GraphicsSystem& GraphicsSystem::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool GraphicsSystem::Init(const DeviceDesc& deviceDesc)
{
    // PIXキャプチャー設定.
    if (deviceDesc.EnableCapture)
    { LoadPixGpuCpatureDll(); }

    if (deviceDesc.EnableDebug)
    {
        asdx::RefPtr<ID3D12Debug> debug;
        auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddress()));
        if (SUCCEEDED(hr))
        {
            hr = debug->QueryInterface(IID_PPV_ARGS(m_pDebug.GetAddress()));
            if (SUCCEEDED(hr))
            {
                m_pDebug->EnableDebugLayer();
            #if 0 // 呼び出すとメッシュシェーダが表示されなくなるので，封印.
                m_pDebug->SetEnableGPUBasedValidation(TRUE);
            #endif
            }
        }
    }

    if (deviceDesc.EnableDRED)
    {
        // DRED有効化.
        asdx::RefPtr<ID3D12DeviceRemovedExtendedDataSettings1> dred;
        auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(dred.GetAddress()));
        if (SUCCEEDED(hr))
        {
            dred->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            dred->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            dred->SetBreadcrumbContextEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
        }
    }

    // DXGIファクトリを生成.
    {
        uint32_t flags = 0;
        if (deviceDesc.EnableDebug)
        { flags |= DXGI_CREATE_FACTORY_DEBUG; }

        RefPtr<IDXGIFactory2> factory;
        auto hr = CreateDXGIFactory2( flags, IID_PPV_ARGS(factory.GetAddress()) );
        if ( FAILED(hr) )
        {
            ELOG("Error : CreateDXGIFactory2() Failed. errcode = 0x%x", hr);
            return false;
        }

        hr = factory->QueryInterface(IID_PPV_ARGS(m_pFactory.GetAddress()));
        if ( FAILED(hr) )
        {
            ELOG("Error : QueryInterface() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // デバイス生成.
    {
        asdx::RefPtr<ID3D12Device> device;
        auto hr = D3D12CreateDevice( nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(device.GetAddress()) );
        if (FAILED(hr))
        {
            ELOG("Error : D3D12CreateDevice() Failed. errcode = 0x%x", hr);
            return false;
        }

        // ID3D12Device8に変換.
        hr = device->QueryInterface(IID_PPV_ARGS(m_pDevice.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : QueryInterface() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_pDevice->SetName(L"asdxDevice");

        // ID3D12InfoQueueに変換.
        if (deviceDesc.EnableDebug)
        {
            hr = m_pDevice->QueryInterface(IID_PPV_ARGS(m_pInfoQueue.GetAddress()));
            if (SUCCEEDED(hr))
            {
                // エラー発生時にブレークさせる.
                if (deviceDesc.EnableBreakOnError)
                { m_pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE); }

                // 警告発生時にブレークさせる.
                if (deviceDesc.EnableBreakOnWarning)
                { m_pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE); }

                // クリア値が違うという警告は無効化しておく.
                m_pInfoQueue->SetBreakOnID(D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, FALSE);
                m_pInfoQueue->SetBreakOnID(D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE, FALSE);
            }
        }
    }

    // 定数バッファ・シェーダリソース・アンオーダードアクセスビュー用ディスクリプタヒープ.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = deviceDesc.MaxShaderResourceCount;
        desc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if ( !m_DescriptorHeap[desc.Type].Init(m_pDevice.GetPtr(), &desc ) )
        {
            ELOG("Error : DescriptorHeap::Init() Failed.");
            return false;
        }
    }

    // サンプラー用ディスクリプタヒープ.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = deviceDesc.MaxSamplerCount;
        desc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if ( !m_DescriptorHeap[desc.Type].Init(m_pDevice.GetPtr(), &desc ) )
        {
            ELOG("Error : DescriptorHeap::Init() Failed");
            return false;
        }
    }

    // レンダーターゲットビュー用ディスクリプタヒープ.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = deviceDesc.MaxColorTargetCount;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        if ( !m_DescriptorHeap[desc.Type].Init(m_pDevice.GetPtr(), &desc ) )
        {
            ELOG("Error : DescriptorHeap::Init() Failed.");
            return false;
        }
    }

    // 深度ステンシルビュー用ディスクリプタヒープ.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = deviceDesc.MaxDepthTargetCount;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        if ( !m_DescriptorHeap[desc.Type].Init(m_pDevice.GetPtr(), &desc ) )
        {
            ELOG("Error : DescriptorHeap::Init() Failed");
            return false;
        }
    }

    // グラフィックスキューの生成.
    auto ret = CommandQueue::Create(
        m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_DIRECT, m_pGraphicsQueue.GetAddress());
    if (!ret)
    {
        ELOG("Error : Queue::Create() Failed.");
        return false;
    }

    // コンピュートキューの生成.
    ret = CommandQueue::Create(
        m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_COMPUTE, m_pComputeQueue.GetAddress());
    if (!ret)
    {
        ELOG("Error : Queue::Create() Failed.");
        return false;
    }

    // コピーキューの生成.
    ret = CommandQueue::Create(
        m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_COPY, m_pCopyQueue.GetAddress());
    if (!ret)
    {
        ELOG("Error : Queue::Create() Failed.");
        return false;
    }

    // ビデオデコードキューの生成.
    ret = CommandQueue::Create(
        m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE, m_pVideoDecodeQueue.GetAddress());
    if (!ret)
    {
        ELOG("Error : Queue::Create() Failed.");
        return false;
    }

    // ビデオプロセスキューの生成.
    ret = CommandQueue::Create(
        m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS, m_pVideoProcessQueue.GetAddress());
    if (!ret)
    {
        ELOG("Error : Queue::Create() Failed.");
        return false;
    }

    // ビデオエンコードキューの生成.
    ret = CommandQueue::Create(
        m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE, m_pVideoEncodeQueue.GetAddress());
    if (!ret)
    {
        ELOG("Error : Queue::Create() Failed.");
        return false;
    }
 
    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void GraphicsSystem::Term()
{
    m_ObjectDisposer    .Clear();
    m_DescriptorDisposer.Clear();

    m_pGraphicsQueue    .Reset();
    m_pComputeQueue     .Reset();
    m_pCopyQueue        .Reset();
    m_pVideoDecodeQueue .Reset();
    m_pVideoProcessQueue.Reset();
    m_pVideoEncodeQueue .Reset();

    for(auto i=0; i<4; ++i)
    {
        m_DescriptorHeap[i].Term();
    }

    m_pDevice   .Reset();
    m_pInfoQueue.Reset();
    m_pDebug    .Reset();
    m_pFactory  .Reset();
}

//-----------------------------------------------------------------------------
//      ID3D12Device8を取得します.
//-----------------------------------------------------------------------------
ID3D12Device8* GraphicsSystem::GetDevice() const
{ return m_pDevice.GetPtr(); }

//-----------------------------------------------------------------------------
//      IDXGIFactory7を取得します.
//-----------------------------------------------------------------------------
IDXGIFactory7* GraphicsSystem::GetFactory() const
{ return m_pFactory.GetPtr(); }

//-----------------------------------------------------------------------------
//      グラフィックスキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GraphicsSystem::GetGraphicsQueue() const
{ return m_pGraphicsQueue.GetPtr(); }

//-----------------------------------------------------------------------------
//      コンピュートキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GraphicsSystem::GetComputeQueue() const
{ return m_pComputeQueue.GetPtr(); }

//-----------------------------------------------------------------------------
//      コピーキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GraphicsSystem::GetCopyQueue() const
{ return m_pCopyQueue.GetPtr(); }

//-----------------------------------------------------------------------------
//      ビデオデコードキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GraphicsSystem::GetVideoDecodeQueue() const
{ return m_pVideoDecodeQueue.GetPtr(); }

//-----------------------------------------------------------------------------
//      ビデオプロセスキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GraphicsSystem::GetVideoProcessQueue() const
{ return m_pVideoProcessQueue.GetPtr(); }

//-----------------------------------------------------------------------------
//      ビデオエンコードキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GraphicsSystem::GetVideoEncodeQueue() const
{ return m_pVideoEncodeQueue.GetPtr(); }

//-----------------------------------------------------------------------------
//      ディスクリプターを確保します.
//-----------------------------------------------------------------------------
bool GraphicsSystem::AllocHandle(uint8_t heapType, Descriptor** ppDescriptor)
{
    ScopedLock locker(&m_SpinLock);

    if (heapType >= 4)
    { return false; }

    auto descriptor = m_DescriptorHeap[heapType].Alloc();
    if (descriptor == nullptr)
    { return false; }

    *ppDescriptor = descriptor;
    return true;
}

//-----------------------------------------------------------------------------
//      アロー演算子です.
//-----------------------------------------------------------------------------
ID3D12Device8* GraphicsSystem::operator-> () const
{ return m_pDevice.GetPtr(); }

//-----------------------------------------------------------------------------
//      ディスクリプタヒープを設定します.
//-----------------------------------------------------------------------------
void GraphicsSystem::SetDescriptorHeaps(ID3D12GraphicsCommandList* pCmdList)
{
    if (pCmdList == nullptr)
    { return; }

    ID3D12DescriptorHeap* pHeaps[] = {
        m_DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].GetD3D12DescriptorHeap(),
        m_DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].GetD3D12DescriptorHeap()
    };

    pCmdList->SetDescriptorHeaps(2, pHeaps);
}

//-----------------------------------------------------------------------------
//      コマンドキューの実行完了を待機します.
//-----------------------------------------------------------------------------
void GraphicsSystem::WaitIdle()
{
    if (m_pGraphicsQueue.GetPtr() != nullptr)
    {
        auto waitPoint = m_pGraphicsQueue->Signal();
        m_pGraphicsQueue->Sync(waitPoint);
    }

    if (m_pComputeQueue.GetPtr() != nullptr)
    {
        auto waitPoint = m_pComputeQueue->Signal();
        m_pComputeQueue->Sync(waitPoint);
    }

    if (m_pCopyQueue.GetPtr() != nullptr)
    {
        auto waitPoint = m_pCopyQueue->Signal();
        m_pCopyQueue->Sync(waitPoint);
    }

    if (m_pVideoDecodeQueue.GetPtr() != nullptr)
    {
        auto waitPoint = m_pVideoDecodeQueue->Signal();
        m_pVideoDecodeQueue->Sync(waitPoint);
    }

    if (m_pVideoEncodeQueue.GetPtr() != nullptr)
    {
        auto waitPoint = m_pVideoEncodeQueue->Signal();
        m_pVideoEncodeQueue->Sync(waitPoint);
    }

    if (m_pVideoProcessQueue.GetPtr() != nullptr)
    {
        auto waitPoint = m_pVideoProcessQueue->Signal();
        m_pVideoProcessQueue->Sync(waitPoint);
    }
}

//-----------------------------------------------------------------------------
//      オブジェクトディスポーザーに追加します.
//-----------------------------------------------------------------------------
void GraphicsSystem::Dispose(ID3D12Object*& pResource, uint8_t lifeTime)
{ m_ObjectDisposer.Push(pResource, lifeTime); }

//-----------------------------------------------------------------------------
//      ディスクリプタディスポーザーに追加します.
//-----------------------------------------------------------------------------
void GraphicsSystem::Dispose(Descriptor*& pDescriptor, uint8_t lifeTime)
{ m_DescriptorDisposer.Push(pDescriptor, lifeTime); }

//-----------------------------------------------------------------------------
//      フレーム同期を取ります.
//-----------------------------------------------------------------------------
void GraphicsSystem::FrameSync()
{
    m_ObjectDisposer    .FrameSync();
    m_DescriptorDisposer.FrameSync();
}

//-----------------------------------------------------------------------------
//      強制破棄を行います.
//-----------------------------------------------------------------------------
void GraphicsSystem::ClearDisposer()
{
    m_ObjectDisposer    .Clear();
    m_DescriptorDisposer.Clear();
}

bool SystemInit(const DeviceDesc& desc)
{ return GraphicsSystem::Instance().Init(desc); }

void SystemTerm()
{ GraphicsSystem::Instance().Term(); }

void SystemWaitIdle()
{ GraphicsSystem::Instance().WaitIdle(); }

void FrameSync()
{ GraphicsSystem::Instance().FrameSync(); }

void DisposeObject(ID3D12Object*& pObject)
{ GraphicsSystem::Instance().Dispose(pObject, kDefaultLifeTime); }

void DisposeDescriptor(Descriptor*& pDescriptor)
{ GraphicsSystem::Instance().Dispose(pDescriptor, kDefaultLifeTime); }

void ClearDisposer()
{ GraphicsSystem::Instance().ClearDisposer(); }

void SetDescriptorHeaps(ID3D12GraphicsCommandList* pCmd)
{ GraphicsSystem::Instance().SetDescriptorHeaps(pCmd); }

CommandQueue* GetGraphicsQueue()
{ return GraphicsSystem::Instance().GetGraphicsQueue(); }

CommandQueue* GetComputeQueue()
{ return GraphicsSystem::Instance().GetComputeQueue(); }

CommandQueue* GetCopyQueue()
{ return GraphicsSystem::Instance().GetCopyQueue(); }

CommandQueue* GetVideoProcessQueue()
{ return GraphicsSystem::Instance().GetVideoProcessQueue(); }

CommandQueue* GetVideoEncodeQueue()
{ return GraphicsSystem::Instance().GetVideoEncodeQueue(); }

CommandQueue* GetVideoDecodeQueue()
{ return GraphicsSystem::Instance().GetVideoDecodeQueue(); }

bool AllocDescriptor(uint8_t heapType, Descriptor** ppResult)
{ return GraphicsSystem::Instance().AllocHandle(heapType, ppResult); }

ID3D12Device8* GetD3D12Device() 
{ return GraphicsSystem::Instance().GetDevice(); }

IDXGIFactory7* GetDXGIFactory()
{ return GraphicsSystem::Instance().GetFactory(); }

} // namespace asdx
