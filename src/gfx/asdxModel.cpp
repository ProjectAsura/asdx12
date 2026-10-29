//-----------------------------------------------------------------------------
// File : asdxModel.cpp
// Desc : Model.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <res/asdxResModel.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxFileIO.h>
#include <fnd/asdxPath.h>
#include <fnd/asdxHash.h>
#include <gfx/asdxModel.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Model class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Model::Model()
: m_RefCount(1)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Model::~Model()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Model::Init(const ResModel& res)
{
    // メッシュが空かどうかチェック.
    if (res.Meshes.empty())
    {
        ELOGA("Error : Invalid Argument.");
        return false;
    }

    // メッシュ初期化.
    m_Meshes.resize(res.Meshes.size());
    for(auto i=0u; i<res.Meshes.size(); ++i)
    {
        if (!m_Meshes[i].Init(res.Meshes[i]))
        {
            ELOGA("Error : Mesh Initialize Failed. index = %zu", i);
            return false;
        }
    }

    // マテリアル名設定.
    if (!res.Materials.empty())
    {
        m_Materials.resize(res.Materials.size());
        for(auto i=0u; i<res.Materials.size(); ++i)
            m_Materials[i] = res.Materials[i];
    }

    // スケルトン初期化.
    m_Skeleton.Init(res.Bones);

    uint64_t instanceCount = 0;

    // メッシュバッチ初期化.
    if (!res.Bathces.empty())
    {
        m_Batches.resize(res.Bathces.size());
        for(auto i=0u; i<res.Bathces.size(); ++i)
        {
            const auto& batch = res.Bathces[i];
            m_Batches[i].Meshes.resize(batch.Meshes.size());
            for(auto j=0u; j<batch.Meshes.size(); ++j)
                m_Batches[i].Meshes[j] = batch.Meshes[j];

            m_Batches[i].Instances.resize(batch.Instances.size());
            for(auto j=0u; j<batch.Instances.size(); ++j)
                m_Batches[i].Instances[j] = batch.Instances[j];

            instanceCount += batch.Instances.size();
        }
    }

    // メッシュインタンス数を設定.
    m_MeshInstanceCount = instanceCount;

    // ハッシュテーブル初期化.
    if (!res.HashTable.empty())
    {
        for(auto i=0; i<res.HashTable.size(); ++i)
        {
            m_HashTable[res.HashTable[i].Hash].assign(
                res.HashTable[i].View.c_str(),
                res.HashTable[i].View.size());
        }
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Model::Init(const ModelBinary& bin)
{
    // メッシュが空かどうかチェック.
    if (bin.GetMeshCount() == 0)
    {
        ELOGA("Error : Invalid Argument.");
        return false;
    }

    // メッシュ初期化.
    {
        auto count = bin.GetMeshCount();
        m_Meshes.resize(count);
        for(auto i=0u; i<count; ++i)
        {
            auto res = bin.GetMesh(i);
            if (!m_Meshes[i].Init(res))
            {
                ELOGA("Error : Mesh Initialize Failed. index = %zu", i);
                return false;
            }
        }
    }

    // マテリアル名設定.
    if (bin.GetMaterialCount() != 0)
    {
        auto count = bin.GetMaterialCount();
        m_Materials.resize(count);
        for(auto i=0u; i<count; ++i)
            m_Materials[i] = bin.GetMaterialHash(i);
    }

    // スケルトン初期化.
    m_Skeleton.Init(bin);

    uint64_t instanceCount = 0;
    // メッシュバッチ初期化.
    if (bin.GetMeshBatchCount() != 0)
    {
        auto count = bin.GetMeshBatchCount();
        m_Batches.resize(count);
        for(auto i=0u; i<count; ++i)
        {
            const auto batch = bin.GetMeshBatch(i);
            m_Batches[i].Meshes.resize(batch.Meshes.size());
            for(auto j=0u; j<batch.Meshes.size(); ++j)
                m_Batches[i].Meshes[j] = batch.Meshes[j];

            m_Batches[i].Instances.resize(batch.Instances.size());
            for(auto j=0u; j<batch.Instances.size(); ++j)
                m_Batches[i].Instances[j] = batch.Instances[j];

            instanceCount += batch.Instances.size();
        }
    }

    // メッシュインタンス数を設定.
    m_MeshInstanceCount = instanceCount;

    // ハッシュテーブル初期化.
    if (bin.GetHashStringCount() != 0)
    {
        auto count = bin.GetHashStringCount();
        for(auto i=0u; i<count; ++i)
        {
            auto pair = bin.GetHashString(i);
            m_HashTable[pair.Hash].assign(
                pair.View.c_str(),
                pair.View.size());
        }
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Model::Term()
{
    // メッシュ解放.
    for(size_t i=0; i<m_Meshes.size(); ++i)
        m_Meshes[i].Term();
    m_Meshes.clear();
    m_Meshes.shrink_to_fit();

    // マテリアル名解放.
    m_Materials.clear();
    m_Materials.shrink_to_fit();

    // スケルトン解放
    m_Skeleton.Term();

    for(size_t i=0; i<m_Batches.size(); ++i)
    {
        m_Batches[i].Instances.clear();
        m_Batches[i].Instances.shrink_to_fit();

        m_Batches[i].Meshes.clear();
        m_Batches[i].Meshes.shrink_to_fit();
    }
    m_Batches.clear();
    m_Batches.shrink_to_fit();

    for(size_t i=0; i<m_HashTable.size(); ++i)
        m_HashTable[i].clear();
    m_HashTable.clear();

    m_MeshInstanceCount = 0;
}

//-----------------------------------------------------------------------------
//      参照カウンタを増やします.
//-----------------------------------------------------------------------------
void Model::AddRef()
{ m_RefCount++; }

//-----------------------------------------------------------------------------
//      解放処理を行います.
//-----------------------------------------------------------------------------
void Model::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-----------------------------------------------------------------------------
//      参照カウンタを取得します.
//-----------------------------------------------------------------------------
uint32_t Model::GetRefCount() const
{ return m_RefCount; }

//-----------------------------------------------------------------------------
//      メッシュ数を取得します.
//-----------------------------------------------------------------------------
size_t Model::GetMeshCount() const
{ return m_Meshes.size(); }

//-----------------------------------------------------------------------------
//      マテリアル数を取得します.
//-----------------------------------------------------------------------------
size_t Model::GetMaterialCount() const
{ return m_Materials.size(); }

//-----------------------------------------------------------------------------
//      メッシュインタンス数を取得します.
//-----------------------------------------------------------------------------
size_t Model::GetMeshInstanceCount() const
{ return m_MeshInstanceCount; }

//-----------------------------------------------------------------------------
//      メッシュバッチ数を返却します.
//-----------------------------------------------------------------------------
size_t Model::GetMeshBatchCount() const
{ return m_Batches.size(); }

//-----------------------------------------------------------------------------
//      メッシュを取得します.
//-----------------------------------------------------------------------------
const Mesh& Model::GetMesh(size_t index) const
{
    assert(index < m_Meshes.size());
    return m_Meshes[index];
}

//-----------------------------------------------------------------------------
//      マテリアル名を取得します.
//-----------------------------------------------------------------------------
uint64_t Model::GetMaterial(size_t index) const
{
    assert(index < m_Materials.size());
    return m_Materials[index];
}

//-----------------------------------------------------------------------------
//      スケルトンを取得します.
//-----------------------------------------------------------------------------
const Skeleton& Model::GetSkeleton() const
{ return m_Skeleton; }

//-----------------------------------------------------------------------------
//      メッシュバッチを取得します.
//-----------------------------------------------------------------------------
ResMeshBatch Model::GetMeshBatch(size_t index) const
{
    assert(index < m_Batches.size());

    const auto& batch = m_Batches[index];
    
    ResMeshBatch result = {};
    result.Meshes    = ArrayView<uint32_t>(batch.Meshes.data(), batch.Meshes.size());
    result.Instances = ArrayView<ResMeshInstance>(batch.Instances.data(), batch.Instances.size());
    return result;
}


//-----------------------------------------------------------------------------
//      メッシュを検索します.
//-----------------------------------------------------------------------------
bool Model::FindMesh(uint64_t hash, size_t& index) const
{
    for(size_t i=0; i<m_Meshes.size(); ++i)
    {
        if (m_Meshes[i].GetHash() == hash)
        {
            index = i;
            return true;
        }
    }

    index = SIZE_MAX;
    return false;
}

//-----------------------------------------------------------------------------
//      マテリアルを検索します.
//-----------------------------------------------------------------------------
bool Model::FindMaterial(uint64_t hash, size_t& index) const
{
    for(size_t i=0; i<m_Materials.size(); ++i)
    {
        if (m_Materials[i] == hash)
        {
            index = i;
            return true;
        }
    }

    index = SIZE_MAX;
    return false;
}

//-----------------------------------------------------------------------------
//      名前を検索します.
//-----------------------------------------------------------------------------
StringView Model::FindName(uint64_t hash) const
{
    auto itr = m_HashTable.find(hash);
    if (itr == m_HashTable.end())
    {
        return StringView();
    }

    return StringView(itr->second.c_str());
}

//-----------------------------------------------------------------------------
//      生成処理を行います.
//-----------------------------------------------------------------------------
bool Model::Create(const ResModel& res, Model** ppModel)
{
    auto instance = new(std::nothrow) Model;
    if (instance == nullptr)
    {
        ELOGA("Error : Out of memory.");
        return false;
    }

    if (!instance->Init(res))
    {
        ELOGA("Error : Model Initialize Failed.");
        instance->Release();
        return false;
    }

    (*ppModel) = instance;
    return true;
}

//-----------------------------------------------------------------------------
//      生成処理を行います.
//-----------------------------------------------------------------------------
bool Model::Create(const ModelBinary& bin, Model** ppModel)
{
    auto instance = new(std::nothrow) Model;
    if (instance == nullptr)
    {
        ELOGA("Error : Out of memory.");
        return false;
    }

    if (!instance->Init(bin))
    {
        ELOGA("Error : Model Initialize Failed.");
        instance->Release();
        return false;
    }

    (*ppModel) = instance;
    return true;
}

} // namespace asdx
