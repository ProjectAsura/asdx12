//-----------------------------------------------------------------------------
// File : asdxDescriptorHolder.cpp
// Desc : Descriptor Holder.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxDescriptorHolder.h>
#include <gfx/asdxDescriptorHeap.h>
#include <gfx/asdxDevice.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// DescriptorHolder class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
DescriptorHolder::DescriptorHolder()
: m_HeapType(HEAP_NONE)
, m_Handle  ()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
DescriptorHolder::DescriptorHolder(HEAP_TYPE heapType, OffsetHandle handle)
: m_HeapType(heapType)
, m_Handle  (handle)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
DescriptorHolder::~DescriptorHolder()
{ Reset(); }

//-----------------------------------------------------------------------------
//      解放処理を行います.
//-----------------------------------------------------------------------------
void DescriptorHolder::Reset()
{
    if (!m_Handle.IsValid())
        return;

    switch(m_HeapType)
    {
        case HEAP_RTV: { GetRtvDescriptorHeap()->Free(m_Handle); } break;
        case HEAP_DSV: { GetDsvDescriptorHeap()->Free(m_Handle); } break;
        case HEAP_RES: { GetResourceDescriptorHeap()->Free(m_Handle); } break;
        case HEAP_SMP: { GetSamplerDescriptorHeap ()->Free(m_Handle); } break;
        default: break;
    }
}

//-----------------------------------------------------------------------------
//      CPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHolder::GetHandleCPU(uint32_t offset) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE result = {};
    if (m_Handle.IsValid())
    {
        switch(m_HeapType)
        {
            case HEAP_RTV: { result = GetRtvDescriptorHeap()->GetHandleCPU(m_Handle, offset); } break;
            case HEAP_DSV: { result = GetDsvDescriptorHeap()->GetHandleCPU(m_Handle, offset); } break;
            case HEAP_RES: { result = GetResourceDescriptorHeap()->GetHandleCPU(m_Handle, offset); } break;
            case HEAP_SMP: { result = GetSamplerDescriptorHeap ()->GetHandleCPU(m_Handle, offset); } break;
            default: break;
        }
    }
    return result;
}

//-----------------------------------------------------------------------------
//      GPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHolder::GetHandleGPU(uint32_t offset) const
{
    D3D12_GPU_DESCRIPTOR_HANDLE result = {};
    if (m_Handle.IsValid())
    {
        switch(m_HeapType)
        {
            case HEAP_RES: { result = GetResourceDescriptorHeap()->GetHandleGPU(m_Handle, offset); } break;
            case HEAP_SMP: { result = GetSamplerDescriptorHeap ()->GetHandleGPU(m_Handle, offset); } break;
            default: break;
        }
    }
    return result;
}

//-----------------------------------------------------------------------------
//      バインドレス用インデックスを取得します.
//-----------------------------------------------------------------------------
uint32_t DescriptorHolder::GetIndex(uint32_t offset) const
{ return m_Handle.GetOffset() + offset; }

//-----------------------------------------------------------------------------
//      有効かどうか判定します.
//-----------------------------------------------------------------------------
bool DescriptorHolder::IsValid() const
{ return m_Handle.IsValid(); }

} // namespace asdx
