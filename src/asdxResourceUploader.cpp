//-----------------------------------------------------------------------------
// File : asdxResourceUploader.cpp
// Desc : Resource Uploader.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxResourceUploader.h>


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
constexpr uint32_t kLifeTime = 4;    // 4フレーム分

} // namespace


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
void ResourceUploader::Push(IUploadResource* pItem)
{
    if (pItem == nullptr)
    { return; }

    m_Queue.push(pItem);
}

//-----------------------------------------------------------------------------
//      アップロード処理を行います.
//-----------------------------------------------------------------------------
void ResourceUploader::Upload(ID3D12GraphicsCommandList* pCmdList)
{
    auto count = m_Queue.size();
    for(size_t i=0; i<count; ++i)
    {
        IUploadResource* resource = m_Queue.front();
        m_Queue.pop();

        resource->Upload(pCmdList);

        Item item;
        item.pResource = resource;
        item.LifeTime  = kLifeTime;

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
        IUploadResource* resource = m_Queue.front();
        m_Queue.pop();

        resource->Release();
        resource = nullptr;
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