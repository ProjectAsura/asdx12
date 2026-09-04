//-----------------------------------------------------------------------------
// File : asdxBlockHeap.cpp
// Desc : Block Heap.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <fnd/asdxBlockHeap.h>
#include <cstdlib>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// BlockHeap class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
BlockHeap::BlockHeap()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
BlockHeap::~BlockHeap()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool BlockHeap::Init(uint32_t blockSize, uint32_t blockCount)
{
    assert(m_pBuffer == nullptr);

    auto size = blockSize * blockCount;
    m_pBuffer = reinterpret_cast<uint8_t*>(malloc(size));
    if (m_pBuffer == nullptr)
    {
        ELOGA("Error : Out of Memory.");
        return false;
    }

    m_BlockSize = blockSize;

    if (!m_IndexHeap.Init(blockCount))
    {
        ELOGA("Error : IndexHeap::Init() Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void BlockHeap::Term()
{
    m_IndexHeap.Term();
 
    if (m_pBuffer != nullptr)
    {
        free(m_pBuffer);
        m_pBuffer = nullptr;
    }

    m_BlockSize = 0;
}

//-----------------------------------------------------------------------------
//      メモリを確保します.
//-----------------------------------------------------------------------------
void* BlockHeap::Alloc()
{
    auto idx = m_IndexHeap.Alloc();
    if (idx == UINT32_MAX)
        return nullptr;

    auto ptr = m_pBuffer + (m_BlockSize * idx);
    memset(ptr, 0, sizeof(m_BlockSize));
    return ptr;
}

//-----------------------------------------------------------------------------
//      メモリを解放します.
//-----------------------------------------------------------------------------
void BlockHeap::Free(void* ptr)
{
    if (ptr == nullptr)
        return;

    auto dif = uintptr_t(ptr) - uintptr_t(m_pBuffer);
    auto idx = uint32_t(dif / m_BlockSize);
    m_IndexHeap.Free(idx);
    memset(ptr, 0xff, sizeof(m_BlockSize));
}

//-----------------------------------------------------------------------------
//      使用ブロック数を取得します.
//-----------------------------------------------------------------------------
uint32_t BlockHeap::GetUsedCount() const
{ return m_IndexHeap.GetUsedCount(); }

//-----------------------------------------------------------------------------
//      未割当ブロック数を取得します.
//-----------------------------------------------------------------------------
uint32_t BlockHeap::GetFreeCount() const
{ return m_IndexHeap.GetFreeCount(); }

//-----------------------------------------------------------------------------
//      最大ブロック数を取得します.
//-----------------------------------------------------------------------------
uint32_t BlockHeap::GetCapacity() const
{ return m_IndexHeap.GetCapacity(); }

//-----------------------------------------------------------------------------
//      ブロックサイズを取得します.
//-----------------------------------------------------------------------------
uint32_t BlockHeap::GetBlockSize() const
{ return m_BlockSize; }

} // namespace asdx
