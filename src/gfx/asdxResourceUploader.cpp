//-----------------------------------------------------------------------------
// File : asdxResourceUploader.cpp
// Desc : Resource Uploader.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxResourceUploader.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ResourceUploader class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ResourceUploader::ResourceUploader()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ResourceUploader::~ResourceUploader()
{ Clear(); }

//-----------------------------------------------------------------------------
//      リソースを登録します.
//-----------------------------------------------------------------------------
void ResourceUploader::Push(IUploadResource* pResource, uint8_t lifeTime)
{
    if (pResource == nullptr)
    { return; }

    Item item = {};
    item.pResource = pResource;
    item.LifeTime  = lifeTime;

    m_Queue.push(item);
}

//-----------------------------------------------------------------------------
//      アップロード処理を行います.
//-----------------------------------------------------------------------------
void ResourceUploader::Upload(ID3D12GraphicsCommandList* pCmdList)
{
    auto count = m_Queue.size();
    for(size_t i=0; i<count; ++i)
    {
        auto& item = m_Queue.front();
        m_Queue.pop();

        item.pResource->Upload(pCmdList);

        m_List.push_back(item);
    }
}

//-----------------------------------------------------------------------------
//      フレーム同期を行い，遅延解放を実行します.
//-----------------------------------------------------------------------------
void ResourceUploader::FrameSync()
{
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
void ResourceUploader::Clear()
{
    auto count = m_Queue.size();
    for(size_t i=0; i<count; ++i)
    {
        auto& item = m_Queue.front();
        m_Queue.pop();

        item.pResource->Release();
        item.pResource = nullptr;
        item.LifeTime  = 0;
    }

    auto itr = m_List.begin();
    while(itr != m_List.end())
    {
        if (itr->pResource != nullptr)
        {
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
