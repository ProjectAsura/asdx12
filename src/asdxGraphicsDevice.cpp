//-----------------------------------------------------------------------------
// File : asdxGraphicsDevice.cpp
// Desc : Graphics Device.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxGraphicsDevice.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// GraphicsDevice class
///////////////////////////////////////////////////////////////////////////////
GraphicsDevice GraphicsDevice::s_Instance = {};

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
GraphicsDevice::GraphicsDevice()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
GraphicsDevice::~GraphicsDevice()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      唯一のインスタンスを取得します.
//-----------------------------------------------------------------------------
GraphicsDevice& GraphicsDevice::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool GraphicsDevice::Init(const Desc* pDesc)
{
    if (pDesc->EnableDebug)
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
        }
    }

    // DXGIファクトリを生成.
    {
        uint32_t flags = 0;
        if (pDesc->EnableDebug)
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

        // ID3D12Device9に変換.
        hr = device->QueryInterface(IID_PPV_ARGS(m_pDevice.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : QueryInterface() Failed. errcode = 0x%x", hr);
            return false;
        }

        // ID3D12InfoQueueに変換.
        if (pDesc->EnableDebug)
        {
            hr = m_pDevice->QueryInterface(IID_PPV_ARGS(m_pInfoQueue.GetAddress()));
            if (SUCCEEDED(hr))
            {
                // エラー発生時にブレークさせる.
                m_pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            }
        }
    }

    // 定数バッファ・シェーダリソース・アンオーダードアクセスビュー用ディスクリプタヒープ.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = pDesc->MaxShaderResourceCount;
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
        desc.NumDescriptors = pDesc->MaxSamplerCount;
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
        desc.NumDescriptors = pDesc->MaxColorTargetCount;
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
        desc.NumDescriptors = pDesc->MaxDepthTargetCount;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        if ( !m_DescriptorHeap[desc.Type].Init(m_pDevice.GetPtr(), &desc ) )
        {
            ELOG("Error : DescriptorHeap::Init() Failed");
            return false;
        }
    }

    // グラフィックスキューの生成.
    m_pGraphicsQueue = Queue::Create(m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_DIRECT);
    if (m_pComputeQueue == nullptr)
    {
        ELOG("Error : Queue::Create() Failed.");
        return false;
    }

    // コンピュートキューの生成.
    m_pComputeQueue = Queue::Create(m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_COMPUTE);
    if (m_pComputeQueue == nullptr)
    {
        ELOG("Error : Queue::Create() Failed.");
        return false;
    }

    // コピーキューの生成.
    m_pCopyQueue = Queue::Create(m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_COPY);
    if (m_pCopyQueue == nullptr)
    {
        ELOG("Error : Queue::Create() Failed.");
        return false;
    }

    // ビデオデコードキューの生成.
    m_pVideoDecodeQueue = Queue::Create(m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE);
    if (m_pVideoDecodeQueue == nullptr)
    {
        ELOG("Error : Queue::Create() Failed.");
        return false;
    }

    // ビデオプロセスキューの生成.
    m_pVideoProcessQueue = Queue::Create(m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS);
    if (m_pVideoProcessQueue == nullptr)
    {
        ELOG("Error : Queue::Create() Failed.");
        return false;
    }

    // ビデオエンコードキューの生成.
    m_pVideoEncodeQueue = Queue::Create(m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE);
    if (m_pVideoEncodeQueue == nullptr)
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
void GraphicsDevice::Term()
{
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
//      ID3D12Device9を取得します.
//-----------------------------------------------------------------------------
ID3D12Device9* GraphicsDevice::GetDevice() const
{ return m_pDevice.GetPtr(); }

//-----------------------------------------------------------------------------
//      IDXGIFactory7を取得します.
//-----------------------------------------------------------------------------
IDXGIFactory7* GraphicsDevice::GetFactory() const
{ return m_pFactory.GetPtr(); }

//-----------------------------------------------------------------------------
//      グラフィックスキューを取得します.
//-----------------------------------------------------------------------------
Queue* GraphicsDevice::GetGraphicsQueue()
{ return m_pGraphicsQueue.GetPtr(); }

//-----------------------------------------------------------------------------
//      コンピュートキューを取得します.
//-----------------------------------------------------------------------------
Queue* GraphicsDevice::GetComputeQueue()
{ return m_pComputeQueue.GetPtr(); }

//-----------------------------------------------------------------------------
//      コピーキューを取得します.
//-----------------------------------------------------------------------------
Queue* GraphicsDevice::GetCopyQueue()
{ return m_pCopyQueue.GetPtr(); }

//-----------------------------------------------------------------------------
//      ビデオデコードキューを取得します.
//-----------------------------------------------------------------------------
Queue* GraphicsDevice::GetVideoDecodeQueue()
{ return m_pVideoDecodeQueue.GetPtr(); }

//-----------------------------------------------------------------------------
//      ビデオプロセスキューを取得します.
//-----------------------------------------------------------------------------
Queue* GraphicsDevice::GetVideoProcessQueue()
{ return m_pVideoProcessQueue.GetPtr(); }

//-----------------------------------------------------------------------------
//      ビデオエンコードキューを取得します.
//-----------------------------------------------------------------------------
Queue* GraphicsDevice::GetVideoEncodeQueue() 
{ return m_pVideoEncodeQueue.GetPtr(); }

//-----------------------------------------------------------------------------
//      ディスクリプターを確保します.
//-----------------------------------------------------------------------------
Descriptor* GraphicsDevice::AllocHandle(int index)
{
    if (index < 0 || index >= 4)
    { return nullptr; }

    return m_DescriptorHeap[index].CreateDescriptor();
}

//-----------------------------------------------------------------------------
//      アロー演算子です.
//-----------------------------------------------------------------------------
ID3D12Device9* GraphicsDevice::operator-> () const
{ return m_pDevice.GetPtr(); }

} // namespace asdx
