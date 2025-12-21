//-----------------------------------------------------------------------------
// File : asdxTextureManager.cpp
// Desc : Texture Manager.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxHash.h>
#include <fnd/asdxPath.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxFileIO.h>
#include <res/asdxResTexture.h>
#include <gfx/asdxTextureManager.h>
#include <gfx/asdxDevice.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// TextureManager class
///////////////////////////////////////////////////////////////////////////////
TextureManager TextureManager::s_Instance = {};

//-----------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-----------------------------------------------------------------------------
TextureManager& TextureManager::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
TextureManager::TextureManager()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
TextureManager::~TextureManager()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool TextureManager::Init()
{
    ScopedLock<SpinLock> locker(m_SpinLock);

    // 初期化済み.
    if (m_Initialized)
        return true;

    // デバイスを取得します.
    auto pDevice = GetD3D12Device();

    for(auto i=0; i<2; ++i)
    {
        auto type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        // コマンドアロケータを生成.
        auto hr = pDevice->CreateCommandAllocator(type, IID_PPV_ARGS(m_CmdAllocator[i].GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommandAllocator() Failed. errcode = 0x%x", hr);
            return false;
        }

        // コマンドリスト生成.
        hr = pDevice->CreateCommandList(0, type, m_CmdAllocator[i].GetPtr(), nullptr, IID_PPV_ARGS(m_CmdList[i].GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommandList() Failed. errcode = 0x%x", hr);
            return false;
        }

        // NOTE: コマンドはオープンしっぱなしのままでいい.
    }

    // バッファ番号初期化.
    m_BufferIndex = 0;

    // 初期化済みフラグを立てる.
    m_Initialized = true;

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void TextureManager::Term()
{
    ScopedLock<SpinLock> locker(m_SpinLock);

    for(auto i=0; i<2; ++i)
    {
        // コマンドリスト破棄.
        {
            auto item = m_CmdList[i].Detach();
            if (item)
                Dispose(item);
        }

        // コマンドアロケータ破棄.
        {
            auto item = m_CmdAllocator[i].Detach();
            if (item)
                Dispose(item);
        }
    }

    // テクスチャ破棄.
    for(auto& itr : m_Textures)
    {
        auto item = itr.second;
        itr.second = nullptr;
        if (item)
        {
            delete item;
            item = nullptr;
        }
    }
    m_Textures.clear();

    m_BufferIndex = 0;

    // 初期化済みフラグを下す.
    m_Initialized = false;
}

//-----------------------------------------------------------------------------
//      生成または取得処理を行います.
//-----------------------------------------------------------------------------
const Texture* TextureManager::GetOrCreate(const char* fullPath)
{
    // ファイルパスがnullかどうかチェック.
    if (fullPath == nullptr)
    {
        ELOG("Error : Invalid Argument.");
        return nullptr;
    }

    // ハッシュ値を計算.
    auto hash = CalcHash(fullPath);

    // 辞書の中から探し出す.
    ScopedLock<SpinLock> locker(m_SpinLock);
    auto itr = m_Textures.find(hash);
    if (itr != m_Textures.end())
        return itr->second; // 見つかった場合はポインタを返却.

    // テクスチャバイナリをロードする.
    std::vector<uint8_t> blob;
    if (!LoadA(fullPath, blob))
    {
        ELOGA("Error : File Load Failed. path = %s", fullPath);
        return nullptr;
    }

    // テクスチャバイナリ取得.
    TextureBinary binary;
    binary.Load(std::move(blob));

    // テクスチャ初期化.
    auto texture = new Texture();
    auto resource = binary.GetResource();
    if (!texture->Init(m_CmdList[m_BufferIndex].GetPtr(), resource))
    {
        ELOGA("Error : Texture Init failed. path = %s", fullPath);
        delete texture;
        return nullptr;
    }

    // テクスチャ登録.
    m_Textures[hash] = texture;

    // 生成したテクスチャを返却する.
    return texture;
}

//-----------------------------------------------------------------------------
//      削除処理.
//-----------------------------------------------------------------------------
void TextureManager::Remove(const char* fullPath)
{
    // ファイルパスがnullかどうかチェック.
    if (fullPath == nullptr)
        return;

    // ハッシュ値を計算.
    auto hash = CalcHash(fullPath);
    Remove(hash);
}

//-----------------------------------------------------------------------------
//      削除処理.
//-----------------------------------------------------------------------------
void TextureManager::Remove(uint64_t hash)
{
    ScopedLock<SpinLock> locker(m_SpinLock);

    // 検索する.
    auto itr = m_Textures.find(hash);

    // 見つかったら削除.
    if (itr != m_Textures.end())
    {
        auto item = itr->second;
        itr->second = nullptr;
        m_Textures.erase(hash);

        if (item)
        {
            delete item;
            item = nullptr;
        }
    }
}

//-----------------------------------------------------------------------------
//      コマンドリストを入れ替えます.
//-----------------------------------------------------------------------------
ID3D12GraphicsCommandList* TextureManager::Swap()
{
    ScopedLock<SpinLock> locker(m_SpinLock);

    // 返却するコマンドリストを取得.
    auto pCmdList = m_CmdList[m_BufferIndex].GetPtr();

    // コマンドを閉じる.
    pCmdList->Close();

    // バッファ入れ替え.
    m_BufferIndex = (m_BufferIndex + 1) & 0x1;

    // 次に使うコマンドリストをリセットしておく.
    m_CmdList[m_BufferIndex]->Reset(m_CmdAllocator[m_BufferIndex].GetPtr(), nullptr);

    // コマンドが積まれているコマンドリストを返却.
    return pCmdList;
}

} // namespace asdx
