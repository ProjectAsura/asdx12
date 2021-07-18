//-----------------------------------------------------------------------------
// File : asdxView.cpp
// Desc : View Object
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <atomic>
#include <gfx/asdxView.h>
#include <gfx/asdxDescriptor.h>
#include <gfx/asdxGraphicsSystem.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ConstantBufferView class
///////////////////////////////////////////////////////////////////////////////
class ConstantBufferView : public IConstantBufferView
{
public:
    ConstantBufferView
    (
        ID3D12Resource*                         pResource,
        const D3D12_CONSTANT_BUFFER_VIEW_DESC*  pDesc,
        Descriptor*                             pDescriptor
    )
    : m_RefCount  (1)
    , m_Resource  (pResource)
    , m_Descriptor(pDescriptor)
    {
        m_Resource->AddRef();
        memcpy(&m_Desc, pDesc, sizeof(m_Desc));
    }

    ~ConstantBufferView()
    {
        Dispose(m_Descriptor);
        Dispose(m_Resource); 
    }

    void AddRef() override
    { m_RefCount++; }

    void Release() override
    {
        m_RefCount--;
        if (m_RefCount == 0)
        { delete this; }
    }

    uint32_t GetCount() const override
    { return m_RefCount; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetHandleCPU() const override
    { return m_Descriptor->GetHandleCPU(); }

    D3D12_GPU_DESCRIPTOR_HANDLE GetHandleGPU() const override
    { return m_Descriptor->GetHandleGPU(); }

    uint32_t GetDescriptorIndex() const override
    { return m_Descriptor->GetIndex(); }

    ID3D12Resource* GetResource() const override
    { return m_Resource; }

    D3D12_CONSTANT_BUFFER_VIEW_DESC GetDesc() const override
    { return m_Desc; }

private:
    std::atomic<uint32_t>           m_RefCount;
    ID3D12Resource*                 m_Resource;
    Descriptor*                     m_Descriptor;
    D3D12_CONSTANT_BUFFER_VIEW_DESC m_Desc;
};


///////////////////////////////////////////////////////////////////////////////
// RenderTargetView class
///////////////////////////////////////////////////////////////////////////////
class RenderTargetView : public IRenderTargetView
{
public:
    RenderTargetView
    (
        ID3D12Resource*                         pResource,
        const D3D12_RENDER_TARGET_VIEW_DESC*    pDesc,
        Descriptor*                             pDescriptor
    )
    : m_RefCount    (1)
    , m_Resource    (pResource)
    , m_Descriptor  (pDescriptor)
    {
        m_Resource->AddRef();
        memcpy(&m_Desc, pDesc, sizeof(m_Desc));
    }

    ~RenderTargetView()
    {
        Dispose(m_Descriptor);
        Dispose(m_Resource);
    }

    void AddRef() override
    { m_RefCount++; }

    void Release() override
    {
        m_RefCount--;
        if (m_RefCount == 0)
        { delete this; }
    }

    uint32_t GetCount() const override
    { return m_RefCount; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetHandleCPU() const override
    { return m_Descriptor->GetHandleCPU(); }

    D3D12_GPU_DESCRIPTOR_HANDLE GetHandleGPU() const override
    { return m_Descriptor->GetHandleGPU(); }

    uint32_t GetDescriptorIndex() const override
    { return m_Descriptor->GetIndex(); }

    ID3D12Resource* GetResource() const override
    { return m_Resource; }

    D3D12_RENDER_TARGET_VIEW_DESC GetDesc() const override
    { return m_Desc; }

private:
    std::atomic<uint32_t>           m_RefCount;
    ID3D12Resource*                 m_Resource;
    Descriptor*                     m_Descriptor;
    D3D12_RENDER_TARGET_VIEW_DESC   m_Desc;
};


///////////////////////////////////////////////////////////////////////////////
// DepthStencilView class
///////////////////////////////////////////////////////////////////////////////
class DepthStencilView : public IDepthStencilView
{
public:
    DepthStencilView
    (
        ID3D12Resource*                         pResource,
        const D3D12_DEPTH_STENCIL_VIEW_DESC*    pDesc,
        Descriptor*                             pDescriptor
    )
    : m_RefCount    (1)
    , m_Resource    (pResource)
    , m_Descriptor  (pDescriptor)
    {
        m_Resource->AddRef();
        memcpy(&m_Desc, pDesc, sizeof(m_Desc));
    }

    ~DepthStencilView()
    {
        Dispose(m_Descriptor);
        Dispose(m_Resource);
    }

    void AddRef() override
    { m_RefCount++; }

    void Release() override
    {
        m_RefCount--;
        if (m_RefCount == 0)
        { delete this; }
    }

    uint32_t GetCount() const override
    { return m_RefCount; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetHandleCPU() const override
    { return m_Descriptor->GetHandleCPU(); }

    D3D12_GPU_DESCRIPTOR_HANDLE GetHandleGPU() const override
    { return m_Descriptor->GetHandleGPU(); }

    uint32_t GetDescriptorIndex() const override
    { return m_Descriptor->GetIndex(); }

    ID3D12Resource* GetResource() const override
    { return m_Resource; }

    D3D12_DEPTH_STENCIL_VIEW_DESC GetDesc() const override
    { return m_Desc; }
   
private:
    std::atomic<uint32_t>           m_RefCount;
    ID3D12Resource*                 m_Resource;
    Descriptor*                     m_Descriptor;
    D3D12_DEPTH_STENCIL_VIEW_DESC   m_Desc;
};

///////////////////////////////////////////////////////////////////////////////
// ShaderResourceView class
///////////////////////////////////////////////////////////////////////////////
class ShaderResourceView : public IShaderResourceView
{
public:
    ShaderResourceView
    (
        ID3D12Resource*                         pResource,
        const D3D12_SHADER_RESOURCE_VIEW_DESC*  pDesc,
        Descriptor*                             pDescriptor
    )
    : m_RefCount    (1)
    , m_Resource    (pResource)
    , m_Descriptor  (pDescriptor)
    {
        m_Resource->AddRef();
        memcpy(&m_Desc, pDesc, sizeof(m_Desc));
    }

    ~ShaderResourceView()
    {
        Dispose(m_Descriptor);
        Dispose(m_Resource);
    }

    void AddRef() override
    { m_RefCount++; }

    void Release() override
    {
        m_RefCount--;
        if (m_RefCount == 0)
        { delete this; }
    }

    uint32_t GetCount() const override
    { return m_RefCount; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetHandleCPU() const override
    { return m_Descriptor->GetHandleCPU(); }

    D3D12_GPU_DESCRIPTOR_HANDLE GetHandleGPU() const override
    { return m_Descriptor->GetHandleGPU(); }

    uint32_t GetDescriptorIndex() const override
    { return m_Descriptor->GetIndex(); }

    ID3D12Resource* GetResource() const override
    { return m_Resource; }

    D3D12_SHADER_RESOURCE_VIEW_DESC GetDesc() const override
    { return m_Desc; }

private:
    std::atomic<uint32_t>           m_RefCount;
    ID3D12Resource*                 m_Resource;
    Descriptor*                     m_Descriptor;
    D3D12_SHADER_RESOURCE_VIEW_DESC m_Desc;
};

///////////////////////////////////////////////////////////////////////////////
// UnorderedAccessView class
///////////////////////////////////////////////////////////////////////////////
class UnorderedAccessView : public IUnorderedAccessView
{
public:
    UnorderedAccessView
    (
        ID3D12Resource* pResource,
        ID3D12Resource* pCounterResource,
        const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc,
        Descriptor* pDescriptor
    )
    : m_RefCount        (1)
    , m_Resource        (pResource)
    , m_CounterResource (pCounterResource)
    , m_Descriptor      (pDescriptor)
    {
        m_Resource->AddRef();
        if (m_CounterResource != nullptr)
        { m_CounterResource->AddRef(); }
        memcpy(&m_Desc, pDesc, sizeof(m_Desc));
    }

    ~UnorderedAccessView()
    {
        Dispose(m_Descriptor);
        Dispose(m_Resource);
        Dispose(m_CounterResource);
    }

    void AddRef() override
    { m_RefCount++; }

    void Release() override
    {
        m_RefCount--;
        if (m_RefCount == 0)
        { delete this; }
    }

    uint32_t GetCount() const override
    { return m_RefCount; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetHandleCPU() const override
    { return m_Descriptor->GetHandleCPU(); }

    D3D12_GPU_DESCRIPTOR_HANDLE GetHandleGPU() const override
    { return m_Descriptor->GetHandleGPU(); }

    uint32_t GetDescriptorIndex() const override
    { return m_Descriptor->GetIndex(); }

    ID3D12Resource* GetResource() const override
    { return m_Resource; }

    ID3D12Resource* GetCounterResource() const override
    { return m_CounterResource; }

    D3D12_UNORDERED_ACCESS_VIEW_DESC GetDesc() const override
    { return m_Desc; }

private:
    std::atomic<uint32_t>               m_RefCount;
    ID3D12Resource*                     m_Resource;
    ID3D12Resource*                     m_CounterResource;
    Descriptor*                         m_Descriptor;
    D3D12_UNORDERED_ACCESS_VIEW_DESC    m_Desc;
};

//-----------------------------------------------------------------------------
//      定数バッファを生成します.
//-----------------------------------------------------------------------------
bool CreateConstantBufferView
(
    ID3D12Resource*                         pResource,
    const D3D12_CONSTANT_BUFFER_VIEW_DESC*  pDesc,
    IConstantBufferView**                   ppView
)
{
    Descriptor* pDescriptor = nullptr;
    auto ret = AllocDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, &pDescriptor);
    if (!ret)
    { return false; }

    GetD3D12Device()->CreateConstantBufferView(pDesc, pDescriptor->GetHandleCPU());

    auto instance = new ConstantBufferView(pResource, pDesc, pDescriptor);
    *ppView = instance;

    return true;
}

//-----------------------------------------------------------------------------
//      レンダーターゲットビューを生成します.
//-----------------------------------------------------------------------------
bool CreateRenderTargetView
(
    ID3D12Resource*                         pResource,
    const D3D12_RENDER_TARGET_VIEW_DESC*    pDesc,
    IRenderTargetView**                     ppView
)
{
    Descriptor* pDescriptor = nullptr;
    auto ret = AllocDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, &pDescriptor);
    if (!ret)
    { return false; }

    GetD3D12Device()->CreateRenderTargetView(pResource, pDesc, pDescriptor->GetHandleCPU());

    auto instance = new RenderTargetView(pResource, pDesc, pDescriptor);
    *ppView = instance;

    return true;
}

//-----------------------------------------------------------------------------
//      深度ステンシルビューを生成します.
//-----------------------------------------------------------------------------
bool CreateDepthStencilView
(
    ID3D12Resource*                         pResource,
    const D3D12_DEPTH_STENCIL_VIEW_DESC*    pDesc,
    IDepthStencilView**                     ppView
)
{
    Descriptor* pDescriptor = nullptr;
    auto ret = AllocDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, &pDescriptor);
    if (!ret)
    { return false; }

    GetD3D12Device()->CreateDepthStencilView(pResource, pDesc, pDescriptor->GetHandleCPU());

    auto instance = new DepthStencilView(pResource, pDesc, pDescriptor);
    *ppView = instance;

    return true;
}

//-----------------------------------------------------------------------------
//      シェーダリソースビューを生成します.
//-----------------------------------------------------------------------------
bool CreateShaderResourceView
(
    ID3D12Resource*                         pResource,
    const D3D12_SHADER_RESOURCE_VIEW_DESC*  pDesc,
    IShaderResourceView**                   ppView
)
{
    Descriptor* pDescriptor = nullptr;
    auto ret = AllocDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, &pDescriptor);
    if (!ret)
    { return false; }

    GetD3D12Device()->CreateShaderResourceView(pResource, pDesc, pDescriptor->GetHandleCPU());

    auto instance = new ShaderResourceView(pResource, pDesc, pDescriptor);
    *ppView = instance;

    return true;
}

//-----------------------------------------------------------------------------
//      アンオーダードアクセスビューを生成します.
//-----------------------------------------------------------------------------
bool CreateUnorderedAccessView
(
    ID3D12Resource*                         pResource,
    ID3D12Resource*                         pCounterResource,
    const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc,
    IUnorderedAccessView**                  ppView
)
{
    Descriptor* pDescriptor = nullptr;
    auto ret = AllocDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, &pDescriptor);
    if (!ret)
    { return false; }

    GetD3D12Device()->CreateUnorderedAccessView(pResource, pCounterResource, pDesc, pDescriptor->GetHandleCPU());

    auto instance = new UnorderedAccessView(pResource, pCounterResource, pDesc, pDescriptor);
    *ppView = instance;

    return true;
}

} // namespace asdx
