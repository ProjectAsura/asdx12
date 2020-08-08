//-----------------------------------------------------------------------------
// File : asdxDisposer.cpp
// Desc : Object Disposer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxDisposer.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Disposer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Disposer::Disposer()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Disposer::~Disposer()
{ Clear(); }

//-----------------------------------------------------------------------------
//      オブジェクトを登録します.
//-----------------------------------------------------------------------------
void Disposer::Push(ID3D12Object*& pObject, uint8_t lifeTime)
{
    if (pObject == nullptr)
    { return; }

    std::lock_guard<SpinLock> locker(m_SpinLock);

    Item item;
    item.pObject    = pObject;
    item.LifeTime   = lifeTime;

    m_List.push_back(item);

    pObject = nullptr;
}

//-----------------------------------------------------------------------------
//      フレーム同期し，遅延解放を実行します.
//-----------------------------------------------------------------------------
void Disposer::FrameSync()
{
    std::lock_guard<SpinLock> locker(m_SpinLock);

    auto itr = m_List.begin();
    while(itr != m_List.end())
    {
        itr->LifeTime--;
        if (itr->LifeTime <= 0)
        {
            if (itr->pObject != nullptr)
            {
                itr->pObject->Release();
                itr->pObject = nullptr;
            }

            itr = m_List.erase(itr);
        }
        else
        {
            itr++;
        }
    }
}

//-----------------------------------------------------------------------------
//      強制破棄を実行します.
//-----------------------------------------------------------------------------
void Disposer::Clear()
{
    std::lock_guard<SpinLock> locker(m_SpinLock);

    auto itr = m_List.begin();
    while(itr != m_List.end())
    {
        if (itr->pObject != nullptr)
        {
            // GPUが実行中だとここで落ちるはずなので，
            // GPUの処理が終わるの確認してから呼んでね.
            itr->pObject->Release();
            itr->pObject = nullptr;
            itr->LifeTime  = 0;
        }

        itr = m_List.erase(itr);
    }

    // 念のため.
    m_List.clear();
}

} // namespace asdx
