//-----------------------------------------------------------------------------
// File : asdxDevice.cpp
// Desc : Graphics Device.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxDevice.h>
#include <gfx/asdxBuffer.h>
#include <gfx/asdxCommandQueue.h>
#include <gfx/asdxDisposer.h>
#include <gfx/asdxCommandList.h>
#include <gfx/asdxDescriptorHeap.h>
#include <fnd/asdxList.h>
#include <fnd/asdxSpinLock.h>
#include <fnd/asdxRef.h>
#include <fnd/asdxLogger.h>
#include <cassert>
#include <strsafe.h>
#include <vector>
#include <algorithm>
#include <tuple>
#include <ShlObj.h>
#include <D3D12MemAlloc.h>


namespace {

///////////////////////////////////////////////////////////////////////////////
// QuaddVertex
///////////////////////////////////////////////////////////////////////////////
struct QuadVertex
{
    float   Position[2];
    float   TexCoord[2];

    QuadVertex(float x, float y, float u, float v)
    {
        Position[0] = x;
        Position[1] = y;
        TexCoord[0] = u;
        TexCoord[1] = v;
    }
};

//-----------------------------------------------------------------------------
//      PIXキャプチャー用のDLLをロードします.
//-----------------------------------------------------------------------------
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
    struct DescriptorPair
    {
        DescriptorHeap*  pHeap;      //!< ディスクリプタヒープ.
        OffsetHandle     Handle;     //!< オフセットハンドル.

        void Release()
        {
            if (pHeap)
            {
                pHeap->Free(Handle);
                pHeap = nullptr;
            }
        }
    };

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
    ID3D12Device8* GetDevice() const { return m_pDevice.GetPtr(); }

    //-------------------------------------------------------------------------
    //! @brief      IDXGIFactory7を取得します.
    //!
    //! @return     IDXGIFactory7を返却します.
    //-------------------------------------------------------------------------
    IDXGIFactory7* GetFactory() const { return m_pFactory.GetPtr(); }

    //-------------------------------------------------------------------------
    //! @brief      グラフィックスキューを取得します.
    //!
    //! @return     グラフィックスキューを返却します.
    //-------------------------------------------------------------------------
    CommandQueue* GetGraphicsQueue() const { return m_pGraphicsQueue.GetPtr(); }

    //-------------------------------------------------------------------------
    //! @brief      コンピュートキューを取得します.
    //!
    //! @return     コンピュートキューを返却します.
    //-------------------------------------------------------------------------
    CommandQueue* GetComputeQueue() const { return m_pComputeQueue.GetPtr(); }

    //-------------------------------------------------------------------------
    //! @brief      コピーキューを取得します.
    //!
    //! @return     コピーキューを返却します.
    //-------------------------------------------------------------------------
    CommandQueue* GetCopyQueue() const { return m_pCopyQueue.GetPtr(); }

    //-------------------------------------------------------------------------
    //! @brief      ディスクリプタヒープを取得します(RTV用).
    //! 
    //! @return     ディスクリプタヒープを返却します(RTV用).
    //-------------------------------------------------------------------------
    DescriptorHeap* GetHeapRTV() { return &m_HeapRTV; }

    //-------------------------------------------------------------------------
    //! @brief      ディスクリプタヒープを取得します(DSV用).
    //! 
    //! @return     ディスクリプタヒープを返却します(DSV用).
    //-------------------------------------------------------------------------
    DescriptorHeap* GetHeapDSV() { return &m_HeapDSV; }

    //-------------------------------------------------------------------------
    //! @brief      ディスクリプタヒープを取得します(Resource用).
    //! 
    //! @return     ディスクリプタヒープを返却します(Resource用).
    //-------------------------------------------------------------------------
    DescriptorHeap* GetHeapResource() { return &m_HeapResource; }

    //-------------------------------------------------------------------------
    //! @brief      ディスクリプタヒープを取得します(Sampler用).
    //! 
    //! @return     ディスクリプタヒープを返却します(Sampler用).
    //-------------------------------------------------------------------------
    DescriptorHeap* GetHeapSampler() { return &m_HeapSampler; }

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
    //--------------------------------------------------------------------------
    void Dispose(ID3D12Object*& pResource);

    //-------------------------------------------------------------------------
    //! @brief      フレーム同期を行います.
    //-------------------------------------------------------------------------
    void FrameSync();

    //-------------------------------------------------------------------------
    //! @brief      強制破棄を行います.
    //-------------------------------------------------------------------------
    void ClearDisposer();

    //-------------------------------------------------------------------------
    //! @brief      サポートされているディスプレイ解像度を取得します.
    //! 
    //! @param[in]          format          フォーマット.
    //! @param[out]         infos           ディスプレイ情報.
    //-------------------------------------------------------------------------
    void GetDisplayInfo(DXGI_FORMAT format, std::vector<DisplayInfo>& infos);

    //-------------------------------------------------------------------------
    //! @brief      フルスクリーン矩形用頂点バッファを取得します.
    //-------------------------------------------------------------------------
    const VertexBuffer& GetQuadVB() const { return m_QuadVB; }

    //-------------------------------------------------------------------------
    //! @brief      D3D12メモリアロケータを取得します.
    //-------------------------------------------------------------------------
    D3D12MA::Allocator* GetAllocator() const { return m_pAllocator.GetPtr(); }

    //-------------------------------------------------------------------------
    //! @brief      DXRをサポートしているかどうか.
    //-------------------------------------------------------------------------
    bool IsSupportDXR() const { return m_SupportDXR; }

    //-------------------------------------------------------------------------
    //! @brief      DXR Tier を取得します.
    //-------------------------------------------------------------------------
    D3D12_RAYTRACING_TIER GetDXRTier() const { return m_DxrTier; }

    //-------------------------------------------------------------------------
    //! @brief      GPUアップロードヒープをサポートしているかどうか.
    //-------------------------------------------------------------------------
    bool IsSupportGpuUploadHeap() const { return m_SupportGpuUploadHeap; }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    static GraphicsSystem           s_Instance;                 //!< シングルトンインスタンス.
    RefPtr<IDXGIFactory7>           m_pFactory;                 //!< DXGIファクトリーです.
    RefPtr<IDXGIAdapter1>           m_pAdapter;                 //!< DXGIアダプターです.
    RefPtr<IDXGIOutput6>            m_pOutput;                  //!< DXGIアウトプットです.
    RefPtr<ID3D12Debug3>            m_pDebug;                   //!< デバッグオブジェクト.
    RefPtr<ID3D12InfoQueue>         m_pInfoQueue;               //!< インフォキュー.
    RefPtr<ID3D12Device8>           m_pDevice;                  //!< デバイス.
    RefPtr<CommandQueue>            m_pGraphicsQueue;           //!< グラフィックスキュー.
    RefPtr<CommandQueue>            m_pComputeQueue;            //!< コンピュートキュー.
    RefPtr<CommandQueue>            m_pCopyQueue;               //!< コピーキュー.
    DescriptorHeap                  m_HeapRTV;                  //!< RTVディスクリプタヒープ.
    DescriptorHeap                  m_HeapDSV;                  //!< DSVディスクリプタヒープ.
    DescriptorHeap                  m_HeapResource;             //!< リソースディスクリプタヒープ.
    DescriptorHeap                  m_HeapSampler;              //!< サンプラーヒープ.
    Disposer<ID3D12Object>          m_ObjectDisposer;           //!< オブジェクトディスポーザー.
    Disposer<DescriptorPair>        m_DescriptorDisposer;       //!< ディスクリプタディスポーザー.
    SpinLock                        m_SpinLock;                 //!< スピンロックです.
    VertexBuffer                    m_QuadVB;                   //!< フルスクリーン描画用三角形.
    RefPtr<D3D12MA::Allocator>      m_pAllocator;               //!< D3D12メモリアロケータ.

    D3D12_RAYTRACING_TIER           m_DxrTier               = D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
    bool                            m_SupportDXR            = false;    //!< DXRに対応しているかどうか.
    bool                            m_SupportGpuUploadHeap  = false;    //!< GPUアップロードヒープに対応しているかどうか.

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


///////////////////////////////////////////////////////////////////////////////
// GraphicsSystem
///////////////////////////////////////////////////////////////////////////////
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
                m_pDebug->SetEnableGPUBasedValidation(TRUE);
            }

            asdx::RefPtr<ID3D12Debug5> pDebug5;
            hr = debug->QueryInterface(IID_PPV_ARGS(pDebug5.GetAddress()));
            if (SUCCEEDED(hr))
            {
                pDebug5->SetEnableAutoName(TRUE);
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

    // DXGIアダプター生成.
    {
        RefPtr<IDXGIAdapter1> pAdapter;
        for(auto adapterId=0;
            DXGI_ERROR_NOT_FOUND != m_pFactory->EnumAdapterByGpuPreference(adapterId, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(pAdapter.ReleaseAndGetAddress()));
            adapterId++)
        {
            DXGI_ADAPTER_DESC1 desc;
            auto hr = pAdapter->GetDesc1(&desc);
            if (FAILED(hr))
            { continue; }

            hr = D3D12CreateDevice(pAdapter.GetPtr(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr);
            if (SUCCEEDED(hr))
            {
                if (m_pAdapter.GetPtr() == nullptr)
                { m_pAdapter = pAdapter.GetPtr(); }

                RefPtr<IDXGIOutput> pOutput;
                hr = pAdapter->EnumOutputs(0, pOutput.GetAddress());
                if (FAILED(hr))
                { continue; }

                hr = pOutput->QueryInterface(IID_PPV_ARGS(m_pOutput.GetAddress()));
                if (SUCCEEDED(hr))
                { break; }
            }
        }
    }

    // デバイス生成.
    {
        asdx::RefPtr<ID3D12Device> device;
        auto hr = D3D12CreateDevice( m_pAdapter.GetPtr(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(device.GetAddress()) );
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

                m_pInfoQueue->PushStorageFilter(&filter);
            }
        }
    }

    // D3D12MA
    {
        D3D12MA::ALLOCATOR_DESC desc = {};
        desc.Flags      = D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED;
        desc.pDevice    = m_pDevice.GetPtr();
        desc.pAdapter   = m_pAdapter.GetPtr();

        auto hr = D3D12MA::CreateAllocator(&desc, m_pAllocator.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : D3D12MA::CreateAllocator() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // 定数バッファ・シェーダリソース・アンオーダードアクセスビュー用ディスクリプタヒープ.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = deviceDesc.MaxShaderResourceCount;
        desc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if ( !m_HeapResource.Init(m_pDevice.GetPtr(), &desc ) )
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
        if ( !m_HeapSampler.Init(m_pDevice.GetPtr(), &desc ) )
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
        if ( !m_HeapRTV.Init(m_pDevice.GetPtr(), &desc ) )
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
        if ( !m_HeapDSV.Init(m_pDevice.GetPtr(), &desc ) )
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

    // 矩形用
    {
        QuadVertex vertices[] = {
            QuadVertex(-1.0f,  1.0f, 0.0f,  0.0f),
            QuadVertex( 3.0f,  1.0f, 2.0f,  0.0f),
            QuadVertex(-1.0f, -3.0f, 0.0f,  2.0f)
        };

        auto size = sizeof(vertices);
        auto stride = uint32_t(sizeof(vertices[0]));

        if (!m_QuadVB.Init(size, stride))
        {
            ELOG("Error : VertexBuffer::Init() Failed.");
            return false;
        }

        auto dst = m_QuadVB.MapAs<QuadVertex>();
        memcpy(dst, vertices, size);
        m_QuadVB.Unmap();
    }

    // DXRのサポートチェック.
    m_SupportDXR = false;
    m_DxrTier    = D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 options = {};
    auto hr = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options));
    if (SUCCEEDED(hr))
    {
        m_DxrTier    = options.RaytracingTier;
        m_SupportDXR = (m_DxrTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED);
    }

    // GPUアップロードヒープのサポートチェック.
    m_SupportGpuUploadHeap = false;
    D3D12_FEATURE_DATA_D3D12_OPTIONS16 options16 = {};
    hr = m_pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS16, &options16, sizeof(options16));
    if (SUCCEEDED(hr))
    { m_SupportGpuUploadHeap = options16.GPUUploadHeapSupported; }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void GraphicsSystem::Term()
{
    m_QuadVB.Term();

    m_ObjectDisposer    .Clear();
    m_DescriptorDisposer.Clear();

    m_pGraphicsQueue    .Reset();
    m_pComputeQueue     .Reset();
    m_pCopyQueue        .Reset();

    m_HeapRTV       .Term();
    m_HeapDSV       .Term();
    m_HeapResource  .Term();
    m_HeapSampler   .Term();

    m_pAllocator.Reset();

    m_pOutput   .Reset();
    m_pDevice   .Reset();
    m_pInfoQueue.Reset();
    m_pDebug    .Reset();
    m_pAdapter  .Reset();
    m_pFactory  .Reset();
}

//-----------------------------------------------------------------------------
//      ディスクリプタヒープを設定します.
//-----------------------------------------------------------------------------
void GraphicsSystem::SetDescriptorHeaps(ID3D12GraphicsCommandList* pCmdList)
{
    if (pCmdList == nullptr)
    { return; }

    // コピーコマンドリストには SetDescriptoreHeaps() は使えない.
    if (pCmdList->GetType() == D3D12_COMMAND_LIST_TYPE_COPY)
    { return; }

    ID3D12DescriptorHeap* pHeaps[] = {
        m_HeapResource.m_Heap.GetPtr(),
        m_HeapSampler .m_Heap.GetPtr()
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
}

//-----------------------------------------------------------------------------
//      オブジェクトディスポーザーに追加します.
//-----------------------------------------------------------------------------
void GraphicsSystem::Dispose(ID3D12Object*& pResource)
{ m_ObjectDisposer.Push(pResource); }

//-----------------------------------------------------------------------------
//      フレーム同期を取ります.
//-----------------------------------------------------------------------------
void GraphicsSystem::FrameSync()
{
    m_ObjectDisposer.FrameSync();

    m_HeapRTV     .FrameSync();
    m_HeapDSV     .FrameSync();
    m_HeapResource.FrameSync();
    m_HeapSampler .FrameSync();
}

//-----------------------------------------------------------------------------
//      強制破棄を行います.
//-----------------------------------------------------------------------------
void GraphicsSystem::ClearDisposer()
{
    m_ObjectDisposer.Clear();
}

//-----------------------------------------------------------------------------
//      ディスプレイ情報を取得します.
//-----------------------------------------------------------------------------
void GraphicsSystem::GetDisplayInfo(DXGI_FORMAT format, std::vector<DisplayInfo>& infos)
{
    if (!m_pOutput)
    { return; }

    UINT count = 0;
    auto hr = m_pOutput->GetDisplayModeList(format, DXGI_ENUM_MODES_SCALING, &count, nullptr);
    if (FAILED(hr) || count == 0)
    { return; }

    std::vector<DXGI_MODE_DESC> descs;
    descs.resize(count);

    hr = m_pOutput->GetDisplayModeList(format, DXGI_ENUM_MODES_SCALING, &count, descs.data());
    if (FAILED(hr))
    { return; }

    infos.resize(count);
    for(size_t i=0; i<infos.size(); ++i)
    {
        infos[i].Width          = descs[i].Width;
        infos[i].Height         = descs[i].Height;
        infos[i].RefreshRate    = descs[i].RefreshRate;
    }

    // 解像度が大きい順にする.
    std::sort(infos.begin(), infos.end(), [](DisplayInfo& lhs, DisplayInfo& rhs)
    {
        auto refreshRateLhs = double(lhs.RefreshRate.Numerator) / double(lhs.RefreshRate.Denominator);
        auto refreshRateRhs = double(rhs.RefreshRate.Numerator) / double(rhs.RefreshRate.Denominator);
        return std::tie(lhs.Width, lhs.Height, refreshRateLhs) > std::tie(rhs.Width, rhs.Height, refreshRateRhs);
    });
}

//-----------------------------------------------------------------------------
//      システムを初期化します.
//-----------------------------------------------------------------------------
bool SystemInit(const DeviceDesc& desc)
{ return GraphicsSystem::Instance().Init(desc); }

//-----------------------------------------------------------------------------
//      システムを終了します.
//-----------------------------------------------------------------------------
void SystemTerm()
{ GraphicsSystem::Instance().Term(); }

//-----------------------------------------------------------------------------
//      システムがアイドル状態になるまで待機します.
//-----------------------------------------------------------------------------
void SystemWaitIdle()
{ GraphicsSystem::Instance().WaitIdle(); }

//-----------------------------------------------------------------------------
//      フレーム同期します.
//-----------------------------------------------------------------------------
void FrameSync()
{ GraphicsSystem::Instance().FrameSync(); }

//-----------------------------------------------------------------------------
//      オブジェクトを破棄します.
//-----------------------------------------------------------------------------
void DisposeObject(ID3D12Object*& pObject)
{ GraphicsSystem::Instance().Dispose(pObject); }

//-----------------------------------------------------------------------------
//      ディスポーザーをクリアします.
//-----------------------------------------------------------------------------
void ClearDisposer()
{ GraphicsSystem::Instance().ClearDisposer(); }

//-----------------------------------------------------------------------------
//      ディスクリプタヒープを設定します.
//-----------------------------------------------------------------------------
void SetDescriptorHeaps(ID3D12GraphicsCommandList* pCmd)
{ GraphicsSystem::Instance().SetDescriptorHeaps(pCmd); }

//-----------------------------------------------------------------------------
//      グラフィックスキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GetGraphicsQueue()
{ return GraphicsSystem::Instance().GetGraphicsQueue(); }

//-----------------------------------------------------------------------------
//      コンピュートキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GetComputeQueue()
{ return GraphicsSystem::Instance().GetComputeQueue(); }

//-----------------------------------------------------------------------------
//      コピーキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GetCopyQueue()
{ return GraphicsSystem::Instance().GetCopyQueue(); }

//-----------------------------------------------------------------------------
//      デバイスを取得します.
//-----------------------------------------------------------------------------
ID3D12Device8* GetD3D12Device() 
{ return GraphicsSystem::Instance().GetDevice(); }

//-----------------------------------------------------------------------------
//      DXGIファクトリを取得します.
//-----------------------------------------------------------------------------
IDXGIFactory7* GetDXGIFactory()
{ return GraphicsSystem::Instance().GetFactory(); }

//-----------------------------------------------------------------------------
//     レンダーターゲットビューディスクリプタヒープを取得します.
//-----------------------------------------------------------------------------
DescriptorHeap* GetRtvDescriptorHeap()
{ return GraphicsSystem::Instance().GetHeapRTV(); }

//-----------------------------------------------------------------------------
//      深度ステンシルビューディスクリプタヒープを取得します.
//-----------------------------------------------------------------------------
DescriptorHeap* GetDsvDescriptorHeap()
{ return GraphicsSystem::Instance().GetHeapDSV(); }

//-----------------------------------------------------------------------------
//      リソースディスクリプタヒープを取得します.
//-----------------------------------------------------------------------------
DescriptorHeap* GetResourceDescriptorHeap()
{ return GraphicsSystem::Instance().GetHeapResource(); }

//-----------------------------------------------------------------------------
//      サンプラーディスクリプタヒープを取得します.
//-----------------------------------------------------------------------------
DescriptorHeap* GetSamplerDescriptorHeap()
{ return GraphicsSystem::Instance().GetHeapSampler(); }

//-----------------------------------------------------------------------------
//      ディスプレイ情報を取得します.
//-----------------------------------------------------------------------------
void GetDisplayInfo(DXGI_FORMAT format, std::vector<DisplayInfo>& result)
{ GraphicsSystem::Instance().GetDisplayInfo(format, result); }

//-----------------------------------------------------------------------------
//      D3D12メモリアロケータを取得します.
//-----------------------------------------------------------------------------
D3D12MA::Allocator* GetD3D12MA()
{ return GraphicsSystem::Instance().GetAllocator(); }

//-----------------------------------------------------------------------------
//      フルスクリーン矩形を描画します.
//-----------------------------------------------------------------------------
void DrawQuad(ID3D12GraphicsCommandList* pCmd)
{
    auto vbv = GraphicsSystem::Instance().GetQuadVB().GetVBV();
    pCmd->IASetVertexBuffers(0, 1, &vbv);
    pCmd->IASetIndexBuffer(nullptr);
    pCmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCmd->DrawInstanced(3, 1, 0, 0);
}

//-----------------------------------------------------------------------------
//      DXRをサポートしているかどうかチェックします.
//-----------------------------------------------------------------------------
bool IsSupportDXR()
{ return GraphicsSystem::Instance().IsSupportDXR(); }

//-----------------------------------------------------------------------------
//      DXR Tierを取得します.
//-----------------------------------------------------------------------------
D3D12_RAYTRACING_TIER GetDXRTier()
{ return GraphicsSystem::Instance().GetDXRTier(); }

//-----------------------------------------------------------------------------
//      GPUアップロードヒープをサポートしているかどうかチェックします.
//-----------------------------------------------------------------------------
bool IsSupportGpuUploadHeap()
{ return GraphicsSystem::Instance().IsSupportGpuUploadHeap(); }

} // namespace asdx
