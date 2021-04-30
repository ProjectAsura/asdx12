//-----------------------------------------------------------------------------
// File : asdxDescriptor.cpp
// Desc : Descriptor Moudle.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <gfx/asdxDescriptor.h>
#include <fnd/asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Descriptor class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Descriptor::Descriptor()
: List<Descriptor>::Node()
, m_pHeap       (nullptr)
, m_HandleCPU   ()
, m_HandleGPU   ()
, m_RefCount    (1)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Descriptor::~Descriptor()
{
    auto pHeap = m_pHeap;
    if (pHeap != nullptr)
    { pHeap->Free(this); }
}

//-----------------------------------------------------------------------------
//      参照カウントを増やします.
//-----------------------------------------------------------------------------
void Descriptor::AddRef()
{ m_RefCount++; }

//-----------------------------------------------------------------------------
//      解放処理を行います.
//-----------------------------------------------------------------------------
void Descriptor::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { this->~Descriptor(); }
}

//-----------------------------------------------------------------------------
//      参照カウントを取得します.
//-----------------------------------------------------------------------------
uint32_t Descriptor::GetCount() const
{ return m_RefCount; }

//-----------------------------------------------------------------------------
//      CPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE Descriptor::GetHandleCPU() const
{ return m_HandleCPU; }

//-----------------------------------------------------------------------------
//      GPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE Descriptor::GetHandleGPU() const
{ return m_HandleGPU; }


///////////////////////////////////////////////////////////////////////////////
// DescritptorHeap class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
DescriptorHeap::DescriptorHeap()
: m_pHeap           (nullptr)
, m_Descriptors     (nullptr)
, m_IncrementSize   (0)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
DescriptorHeap::~DescriptorHeap()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool DescriptorHeap::Init(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    if (pDesc->NumDescriptors == 0)
    { return true; }

    auto hr = pDevice->CreateDescriptorHeap(pDesc, IID_PPV_ARGS(&m_pHeap));
    if ( FAILED(hr) )
    {
        ELOG("Error : ID3D12Device::CreateDescriptorHeap() Failed. errcode = 0x%x", hr);
        return false;
    }

    m_pHeap->SetName(L"asdxDescriptorHeap");

    // インクリメントサイズを取得.
    m_IncrementSize = pDevice->GetDescriptorHandleIncrementSize(pDesc->Type);

    // ディスクリプタ生成.
    m_Descriptors = new Descriptor[pDesc->NumDescriptors];

    auto hasHandleGPU = (pDesc->Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

    for(auto i=0u; i<pDesc->NumDescriptors; ++i)
    {
        // ヒープを設定.
        m_Descriptors[i].m_pHeap = this;

        // CPUハンドルをディスクリプタを割り当て.
        {
            auto handleCPU = m_pHeap->GetCPUDescriptorHandleForHeapStart();
            handleCPU.ptr += UINT64(m_IncrementSize) * i;
            m_Descriptors[i].m_HandleCPU = handleCPU;
        }

        // GPUハンドルをディスクリプタを割り当て
        if (hasHandleGPU)
        {
            auto handleGPU = m_pHeap->GetGPUDescriptorHandleForHeapStart();
            handleGPU.ptr += UINT64(m_IncrementSize) * i;
            m_Descriptors[i].m_HandleGPU = handleGPU;
        }

        // 未使用リストに追加.
        m_FreeList.PushBack(&m_Descriptors[i]);
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void DescriptorHeap::Term()
{
    m_FreeList.Clear();
    m_UsedList.Clear();

    if (m_Descriptors != nullptr)
    {
        delete[] m_Descriptors;
        m_Descriptors = nullptr;
    }

    if (m_pHeap != nullptr)
    {
        m_pHeap->Release();
        m_pHeap = nullptr;
    }
}

//-----------------------------------------------------------------------------
//      ディスクリプタを生成します.
//-----------------------------------------------------------------------------
Descriptor* DescriptorHeap::Alloc()
{
    auto node = m_FreeList.PopFront();
    m_UsedList.PushBack(node);
    return node;
}

//-----------------------------------------------------------------------------
//      ディスクリプタを破棄します.
//-----------------------------------------------------------------------------
void DescriptorHeap::Free(Descriptor* pValue)
{
    if (pValue == nullptr)
    { return; }

    m_UsedList.Remove(pValue);
    m_FreeList.PushBack(pValue);
}

//-----------------------------------------------------------------------------
//      ディスクリプタヒープを取得します.
//-----------------------------------------------------------------------------
ID3D12DescriptorHeap* DescriptorHeap::GetD3D12DescriptorHeap() const
{ return m_pHeap; }

//-----------------------------------------------------------------------------
//      割り当て済みハンドル数を取得します.
//-----------------------------------------------------------------------------
uint32_t DescriptorHeap::GetAllocatedCount() const
{ return uint32_t(m_UsedList.GetCount()); }

//-----------------------------------------------------------------------------
//      割り当て可能なハンドル数を取得します.
//-----------------------------------------------------------------------------
uint32_t DescriptorHeap::GetAvailableCount() const
{ return uint32_t(m_FreeList.GetCount()); }

//-----------------------------------------------------------------------------
//      ハンドル数を取得します.
//-----------------------------------------------------------------------------
uint32_t DescriptorHeap::GetHandleCount() const
{ return uint32_t(m_UsedList.GetCount() + m_FreeList.GetCount()); }
 
} // namespace a3d
