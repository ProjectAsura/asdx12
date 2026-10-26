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
    for(size_t i=0; i<res.Meshes.size(); ++i)
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
        for(size_t i=0; i<res.Materials.size(); ++i)
        {
            m_Materials[i] = res.Materials[i];
        }
    }

    // スケルトン初期化.
    m_Skeleton.Init(res.Bones);

    if (!res.MeshInstances.empty())
    {
        m_MeshInstances.reserve(res.MeshInstances.size());
        for(size_t i=0; i<res.MeshInstances.size(); ++i)
        {
            const auto& src = res.MeshInstances[i];
            auto& dst = m_MeshInstances[i];

            dst.Hashes.resize(src.Hashes.size());
            for(size_t j=0; j<dst.Hashes.size(); ++j)
            { dst.Hashes[j] = src.Hashes[j]; }

            dst.Meshes.resize(src.Meshes.size());
            for(size_t j=0; j<dst.Meshes.size(); ++j)
            { dst.Meshes[j] = src.Meshes[j]; }

            dst.Transforms.resize(src.Transform.size());
            for(size_t j=0; j<dst.Transforms.size(); ++j)
            { dst.Transforms[j] = src.Transform[j]; }
        }
    }


    if (!res.HashTable.empty())
    {
        for(auto i=0; i<res.HashTable.size(); ++i)
        {
            m_HashTable[res.HashTable[i].Hash].assign(
                res.HashTable[i].Name.c_str(),
                res.HashTable[i].Name.size());
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

    // マテリアル名解放.
    m_Materials.clear();

    // スケルトン解放
    m_Skeleton.Term();

    // メッシュインスタンス解放.
    m_MeshInstances.clear();

    for(size_t i=0; i<m_HashTable.size(); ++i)
        m_HashTable[i].clear();
    m_HashTable.clear();
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
{ return m_MeshInstances.size(); }

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
//      メッシュインタンスを取得します.
//-----------------------------------------------------------------------------
ResMeshInstance Model::GetMeshInstance(size_t index) const
{
    assert(index < m_MeshInstances.size());

    ResMeshInstance result;
    result.Meshes    = ArrayView<uint32_t> (m_MeshInstances[index].Meshes    .data(), m_MeshInstances[index].Meshes    .size());
    result.Hashes    = ArrayView<uint64_t> (m_MeshInstances[index].Hashes    .data(), m_MeshInstances[index].Hashes    .size());
    result.Transform = ArrayView<Matrix4x3>(m_MeshInstances[index].Transforms.data(), m_MeshInstances[index].Transforms.size());
    return result;
}

//-----------------------------------------------------------------------------
//      メッシュを検索します.
//-----------------------------------------------------------------------------
bool Model::FindMesh(const char* name, size_t& index) const
{
    auto hash = CalcHash(name);

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
bool Model::FindMaterial(const char* name, size_t& index) const
{
    auto hash = CalcHash(name);

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
bool Model::FindName(uint64_t hash, StringView& result) const
{
    auto itr = m_HashTable.find(hash);
    if (itr == m_HashTable.end())
    {
        result = StringView();
        return false;
    }

    result = StringView(itr->second.c_str());
    return true;
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

} // namespace asdx
