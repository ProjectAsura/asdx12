//-----------------------------------------------------------------------------
// File : asdxDescriptorHeap.cpp
// Desc : Descriptor Heap.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <gfx/asdxDescriptorHeap.h>
#include <fnd/asdxLogger.h>


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t kDefaultFrameCount = 4;   //!< 待機フレーム数です.

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// DescriptorHeap class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
DescriptorHeap::DescriptorHeap()
: m_Heap            (nullptr)
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
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    auto hr = pDevice->CreateDescriptorHeap(pDesc, IID_PPV_ARGS(m_Heap.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateDescriptorHeap() Failed. errcode = 0x%x", hr);
        return false;
    }

    m_Allocator.Init(pDesc->NumDescriptors, pDesc->NumDescriptors);
    m_IncrementSize = pDevice->GetDescriptorHandleIncrementSize(pDesc->Type);

    m_DisposeList.resize(kDefaultFrameCount);

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void DescriptorHeap::Term()
{
    {
        ScopedLock<SpinLock> lock(m_SpinLock);

        for(auto& list : m_DisposeList)
        {
            auto itr = list.begin();
            while(itr != list.end())
            {
                auto handle = (*itr);
                if (handle.IsValid())
                { m_Allocator.Free(handle); }
                itr = list.erase(itr);
            }
            list.clear();
        }
    }

    m_Allocator.Term();
    m_Heap.Reset();
}

//-----------------------------------------------------------------------------
//      オフセットハンドルを確保します.
//-----------------------------------------------------------------------------
OffsetHandle DescriptorHeap::Alloc(uint32_t count)
{
    ScopedLock<SpinLock> lock(m_SpinLock);
    return m_Allocator.Alloc(count);
}

//-----------------------------------------------------------------------------
//      オフセットハンドルを解放します.
//-----------------------------------------------------------------------------
void DescriptorHeap::Free(OffsetHandle& handle)
{
    if (!handle.IsValid())
        return;

    ScopedLock<SpinLock> lock(m_SpinLock);
    m_DisposeList.front().push_back(handle);
    handle = OffsetHandle();
}

//-----------------------------------------------------------------------------
//      CPUハンドルに変換します.
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetHandleCPU(const OffsetHandle& holder, uint32_t offset) const
{
    assert(offset < holder.GetSize()); 
    auto result = m_Heap->GetCPUDescriptorHandleForHeapStart();
    result.ptr += SIZE_T((holder.GetOffset() + offset) * m_IncrementSize);
    return result;
}

//-----------------------------------------------------------------------------
//      GPUハンドルに変換します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetHandleGPU(const OffsetHandle& holder, uint32_t offset) const
{
    assert(offset < holder.GetSize()); 
    auto result = m_Heap->GetGPUDescriptorHandleForHeapStart();
    result.ptr += UINT64((holder.GetOffset() + offset) * m_IncrementSize);
    return result;
}

//-----------------------------------------------------------------------------
//      使用中のハンドル数を取得します.
//-----------------------------------------------------------------------------
uint32_t DescriptorHeap::GetUsedCount() const
{ return m_Allocator.GetUsedSize(); }

//-----------------------------------------------------------------------------
//      使用可能なハンドル数を取得します.
//-----------------------------------------------------------------------------
uint32_t DescriptorHeap::GetFreeCount() const
{ return m_Allocator.GetFreeSize(); }

//-----------------------------------------------------------------------------
//      フレーム同期を行います.
//-----------------------------------------------------------------------------
void DescriptorHeap::FrameSync()
{
    ScopedLock<SpinLock> lock(m_SpinLock);

    // 先頭を末端に移動し，前にずらす.
    std::rotate(m_DisposeList.begin(), (++m_DisposeList.begin()), m_DisposeList.end());

    auto& list = m_DisposeList.front();
    auto itr = list.begin();
    while(itr != list.end())
    {
        auto handle = (*itr);
        if (handle.IsValid())
        { m_Allocator.Free(handle); }
        itr = list.erase(itr);
    }
    list.clear();
}

} // namespace asdx
