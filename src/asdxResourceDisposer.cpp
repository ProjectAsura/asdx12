//-----------------------------------------------------------------------------
// File : asdxResourceDisposer.cpp
// Desc : Resource Disposer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxResourceDisposer.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ResourceDisposer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ResourceDisposer::ResourceDisposer()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ResourceDisposer::~ResourceDisposer()
{ Clear(); }

//-----------------------------------------------------------------------------
//      リソースを登録します.
//-----------------------------------------------------------------------------
void ResourceDisposer::Push(ID3D12Resource*& pResource, uint8_t lifeTime)
{
    if (pResource == nullptr)
    { return; }

    std::lock_guard<SpinLock> locker(m_SpinLock);

    Item item;
    item.pResource = pResource;
    item.LifeTime  = lifeTime;

    m_List.push_back(item);

    pResource = nullptr;
}

//-----------------------------------------------------------------------------
//      フレーム同期し，遅延解放を実行します.
//-----------------------------------------------------------------------------
void ResourceDisposer::FrameSync()
{
    std::lock_guard<SpinLock> locker(m_SpinLock);

    auto itr = m_List.begin();
    while(itr != m_List.end())
    {
        itr->LifeTime--;
        if (itr->LifeTime <= 0)
        {
            if (itr->pResource != nullptr)
            {
                itr->pResource->Release();
                itr->pResource = nullptr;
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
void ResourceDisposer::Clear()
{
    std::lock_guard<SpinLock> locker(m_SpinLock);

    auto itr = m_List.begin();
    while(itr != m_List.end())
    {
        if (itr->pResource != nullptr)
        {
            // GPUが実行中だとここで落ちるはずなので，
            // GPUの処理が終わるの確認してから呼んでね.
            itr->pResource->Release();
            itr->pResource = nullptr;
            itr->LifeTime  = 0;
        }

        itr = m_List.erase(itr);
    }

    // 念のため.
    m_List.clear();
}

} // namespace asdx
