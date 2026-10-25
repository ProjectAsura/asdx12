//-----------------------------------------------------------------------------
// File : asdxMaterialManager.cpp
// Desc : Material Manager.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <fnd/asdxHash.h>
#include <fnd/asdxFileIO.h>
#include <gfx/asdxMaterialManager.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// MaterialManager class
///////////////////////////////////////////////////////////////////////////////
MaterialManager MaterialManager::s_Instance = {};

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
MaterialManager::MaterialManager()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
MaterialManager::~MaterialManager()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool MaterialManager::Init()
{
    // 初期化済み.
    if (m_Initialized)
        return true;

    ScopedLock<SpinLock> locker(m_SpinLock);

    // 初期化済みフラグを立てる.
    m_Initialized = true;

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void MaterialManager::Term()
{
    // 解放済み.
    if (!m_Initialized)
        return;

    ScopedLock<SpinLock> locker(m_SpinLock);

    // マテリアルを解放.
    for(auto& itr : m_Materials)
    {
        auto item = itr.second;
        itr.second = nullptr;
        SafeRelease(item);
    }
    m_Materials.clear();

    // 初期化フラグを下す.
    m_Initialized = false;
}

//-----------------------------------------------------------------------------
//      生成または取得処理を行います.
//-----------------------------------------------------------------------------
MaterialHolder MaterialManager::GetOrCreate(const char* fullPath)
{
    // 引数チェック.
    if (fullPath == nullptr)
    {
        ELOGA("Error : Invalid Argument.");
        return MaterialHolder();
    }

    // ファイルハッシュを求める.
    auto hash = CalcHash(fullPath);

    // 生成済みかどうかチェック.
    ScopedLock<SpinLock> locker(m_SpinLock);
    auto itr = m_Materials.find(hash);
    if (itr != m_Materials.end())
    {
        auto pMaterial = itr->second;
        pMaterial->AddRef();
        return MaterialHolder(pMaterial, hash);
    }

    // マテリアルバイナリをロード.
    std::vector<uint8_t> blob;
    if (!LoadA(fullPath, blob))
    {
        ELOGA("Error : File Load Failed. path = %s", fullPath);
        return MaterialHolder();
    }

    MaterialBinary binary;
    binary.Load(std::move(blob));

    // リソース取得.
    auto resource = binary.GetResource();

    // マテリアル生成.
    Material* pMaterial = nullptr;
    if (!Material::Create(resource, &pMaterial))
    {
        ELOGA("Error : Material Create Failed. path = %s", fullPath);
        return MaterialHolder();
    }

    m_Materials[hash] = pMaterial;

    return MaterialHolder(pMaterial, hash);
}

//-----------------------------------------------------------------------------
//      削除処理を行います.
//-----------------------------------------------------------------------------
void MaterialManager::Remove(MaterialHolder& holder)
{
    // 無効なら即終了.
    if (!holder.IsValid())
        return;

    if (holder.m_pRes->GetRefCount() > 1)
    {
        // 参照カウントを減らす.
        holder.m_pRes->Release();
    }
    else
    {
        ScopedLock<SpinLock> locker(m_SpinLock);

        // 削除処理.
        auto itr = m_Materials.find(holder.m_Hash);
        if (itr != m_Materials.end())
        {
            auto item = itr->second;
            itr->second = nullptr;
            m_Materials.erase(holder.m_Hash);
            SafeRelease(item);
        }
    }

    // クリア処理.
    holder.m_pRes = nullptr;
    holder.m_Hash = 0;
}

} // namespace asdx
