//-----------------------------------------------------------------------------
// File : asdxBlockTaskManager.cpp
// Desc : Block Task Manager.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <fw/asdxBlockTaskManager.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// BlockTaskManager class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
BlockTaskManager::BlockTaskManager()
: TaskManagerBase()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
BlockTaskManager::~BlockTaskManager()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool BlockTaskManager::Init(uint32_t blockSize, uint32_t blockCount)
{
    auto size = blockSize * blockCount;
    m_pBuffer = reinterpret_cast<uint8_t*>(malloc(size));
    if (m_pBuffer == nullptr)
    {
        ELOGA("Error : Out of Memory.");
        return false;
    }

    if (!m_Heap.Init(blockCount))
    {
        ELOGA("Error : IndexHeap::Init() Failed.");
        return false;
    }

    m_BlockSize = blockSize;
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void BlockTaskManager::Term()
{
    auto itr = m_TaskList.begin();
    while(itr != m_TaskList.end())
    {
        itr->OnRemove();
        itr = m_TaskList.erase(itr);
    }

    m_Heap.Term();

    m_BlockSize = 0;
    if (m_pBuffer != nullptr)
    {
        free(m_pBuffer);
        m_pBuffer = nullptr;
    }
}

//-----------------------------------------------------------------------------
//      全タスクを削除します.
//-----------------------------------------------------------------------------
void BlockTaskManager::Reset()
{
    auto itr = m_TaskList.begin();
    while(itr != m_TaskList.end())
    {
        auto pTask = &(*itr);
        itr->OnRemove();
        itr = m_TaskList.erase(itr);
        Free(pTask);
    }
}

//-----------------------------------------------------------------------------
//      割り当て済みタスク数を取得します.
//-----------------------------------------------------------------------------
uint32_t BlockTaskManager::GetUsedCount() const
{ return m_Heap.GetUsedCount(); }

//-----------------------------------------------------------------------------
//      未割当タスク数を取得します.
//-----------------------------------------------------------------------------
uint32_t BlockTaskManager::GetFreeCount() const
{ return m_Heap.GetFreeCount(); }

//-----------------------------------------------------------------------------
//      最大タスク数を取得します.
//-----------------------------------------------------------------------------
uint32_t BlockTaskManager::GetCapacity() const
{ return m_Heap.GetCapacity(); }

//-----------------------------------------------------------------------------
//      メモリを確保します.
//-----------------------------------------------------------------------------
void* BlockTaskManager::Alloc(size_t size)
{
    if (m_pBuffer == nullptr)
    {
        ELOGA("Error : Invalid Operation.");
        return nullptr;
    }

    if (size > m_BlockSize)
    {
        ELOGA("Error : Invalid Argument. BlockSize exceed. size = %zu, blockSize = %u", size, m_BlockSize);
        return nullptr;
    }

    auto index = m_Heap.Alloc();
    if (index == UINT32_MAX)
    {
        ELOGA("Error : Out of Memory.");
        return nullptr;
    }

    auto buf = m_pBuffer + (m_BlockSize * index);
    return buf;
}

//-----------------------------------------------------------------------------
//      メモリを解放します.
//-----------------------------------------------------------------------------
void BlockTaskManager::Free(void* ptr)
{
    if (m_pBuffer == nullptr)
    {
        ELOGA("Error : Invalid Operation.");
        return;
    }

    auto diff  = uintptr_t(ptr) - uintptr_t(m_pBuffer);
    auto index = uint32_t(diff / m_BlockSize);

    m_Heap.Free(index);
    memset(ptr, 0, m_BlockSize);
}

} // namespace asdx
