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
#include <gfx/asdxModel.h>


namespace {

//-----------------------------------------------------------------------------
//      頂点バッファを初期化します.
//-----------------------------------------------------------------------------
template<typename T>
bool InitVB(asdx::VertexBuffer& dst, const asdx::ArrayView<T>& src)
{
    // 存在しないものもいるので，空なら成功扱い.
    if (src.empty())
        return true;

    if (!dst.Init(sizeof(T) * src.size(), sizeof(T)))
        return false;

    auto ptr = dst.MapAs<T>();
    assert(ptr != nullptr);
    memcpy(ptr, src.data(), sizeof(T) * src.size());
    dst.Unmap();

    return true;
}

//-----------------------------------------------------------------------------
//      インデックスバッファを初期化します.
//-----------------------------------------------------------------------------
bool InitIB(asdx::IndexBuffer& dst, const asdx::ArrayView<uint32_t>& src)
{
    // 絶対に存在するはずなので，空なら失敗扱い.
    if (src.empty())
        return false;

    if (!dst.Init(sizeof(uint32_t) * src.size()))
        return false;

    auto ptr = dst.MapAs<uint32_t>();
    assert(ptr != nullptr);
    memcpy(ptr, src.data(), sizeof(uint32_t) * src.size());
    dst.Unmap();

    return true;
}

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Mesh class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Mesh::Mesh()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Mesh::~Mesh()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Mesh::Init(const res::Mesh& mesh)
{
    // 名前を設定.
    m_Name = MeshProxy::GetName(mesh).c_str();

    // マテリアルIDを設定.
    m_MaterialId = MeshProxy::GetMaterialId(mesh);

    // 位置座標バッファ初期化.
    auto pos = MeshProxy::GetPositions(mesh);
    if (!InitVB(m_Positions, pos))
    {
        ELOG("Error : Positions Initialize Failed.");
        return false;
    }

    // 法線バッファ初期化.
    auto nrm = MeshProxy::GetNormals(mesh);
    if (!InitVB(m_Normals, nrm))
    {
        ELOG("Error : Normal Initailize Failed.");
        return false;
    }

    // 接線バッファ初期化.
    auto tans = MeshProxy::GetTangents(mesh);
    if (!InitVB(m_Tangents, tans))
    {
        ELOG("Error : Tangents Initialize Failed.");
        return false;
    }

    // カラーバッファ初期化.
    auto cols = MeshProxy::GetColors(mesh);
    if (!InitVB(m_Colors, cols))
    {
        ELOG("Error : Colors Initialize Failed.");
        return false;
    }

    // テクスチャ座標バッファ初期化.
    auto uvs = MeshProxy::GetTexCoords(mesh);
    if (!InitVB(m_TexCoords, uvs))
    {
        ELOG("Error : TexCoords Initialize Failed.");
        return false;
    }

    // ボーンインデックスバッファ初期化.
    auto boneIds = MeshProxy::GetBoneIndices(mesh);
    if (!InitVB(m_BoneIndices, boneIds))
    {
        ELOG("Error : BoneIndices Initialize Failed.");
        return false;
    }

    // ボーンウェイトバッファ初期化.
    auto boneWeights = MeshProxy::GetBoneWeights(mesh);
    if (!InitVB(m_BoneWeights, boneWeights))
    {
        ELOG("Error : BoneWeights Initailize Failed.");
        return false;
    }

    // 頂点インデックスバッファ初期化.
    auto vertIds = MeshProxy::GetVerexIndices(mesh);
    if (!InitIB(m_VertexIndices, vertIds))
    {
        ELOG("Error : VertexIndices Initailize Failed.");
        return false;
    }

    // 頂点数を設定.
    m_VertexCount = uint32_t(pos.size());

    // 頂点インデックス数を設定.
    m_IndexCount = uint32_t(vertIds.size());

    // バウンディングスフィアを設定.
    m_BoundingSphere = MeshProxy::GetBounds(mesh);

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Mesh::Term()
{
    m_Name.clear();
    m_VertexCount   = 0;
    m_IndexCount    = 0;
    m_MaterialId    = UINT32_MAX;

    m_Positions     .Term();
    m_Normals       .Term();
    m_Tangents      .Term();
    m_Colors        .Term();
    m_TexCoords     .Term();
    m_BoneIndices   .Term();
    m_BoneWeights   .Term();
    m_VertexIndices .Term();
}

//-----------------------------------------------------------------------------
//      メッシュ名を取得します.
//-----------------------------------------------------------------------------
const std::string& Mesh::GetName() const
{ return m_Name; }

//-----------------------------------------------------------------------------
//      マテリアルIDを取得します.
//-----------------------------------------------------------------------------
uint32_t Mesh::GetMaterialId() const
{ return m_MaterialId; }

//-----------------------------------------------------------------------------
//      位置座標バッファを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetPositions() const
{ return m_Positions; }

//-----------------------------------------------------------------------------
//      法線バッファを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetNormals() const
{ return m_Normals; }

//-----------------------------------------------------------------------------
//      接線バッファを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetTangents() const
{ return m_Tangents; }

//-----------------------------------------------------------------------------
//      カラーバッファを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetColors() const
{ return m_Colors; }

//-----------------------------------------------------------------------------
//      テクスチャ座標バッファを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetTexCoords() const
{ return m_TexCoords; }

//-----------------------------------------------------------------------------
//      ボーンインデックスバッファを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetBoneIndices() const
{ return m_BoneIndices; }

//-----------------------------------------------------------------------------
//      ボーンウェイトバッファを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetBoneWeights() const
{ return m_BoneWeights; }

//-----------------------------------------------------------------------------
//      頂点インデックスバッファを取得します.
//-----------------------------------------------------------------------------
const IndexBuffer& Mesh::GetIndices() const
{ return m_VertexIndices; }

//-----------------------------------------------------------------------------
//      頂点数を取得します.
//-----------------------------------------------------------------------------
uint32_t Mesh::GetVertexCount() const
{ return m_VertexCount; }

//-----------------------------------------------------------------------------
//      頂点インデックス数を取得します.
//-----------------------------------------------------------------------------
uint32_t Mesh::GetIndexCount() const
{ return m_IndexCount; }

//-----------------------------------------------------------------------------
//      バウンディングスフィアを取得します.
//-----------------------------------------------------------------------------
const BoundingSphere3& Mesh::GetBoundingSphere() const
{ return m_BoundingSphere; }

//-----------------------------------------------------------------------------
//      レイトレーシングジオメトリトライアングル設定を取得します.
//-----------------------------------------------------------------------------
D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC Mesh::GetTrianglesDesc() const
{
    D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC result = {};
    result.IndexFormat                  = DXGI_FORMAT_R32_UINT;
    result.VertexFormat                 = DXGI_FORMAT_R32G32B32_FLOAT;
    result.IndexCount                   = m_IndexCount;
    result.VertexCount                  = m_VertexCount;
    result.IndexBuffer                  = m_VertexIndices.GetGpuAddress();
    result.VertexBuffer.StartAddress    = m_Positions    .GetGpuAddress();
    result.VertexBuffer.StrideInBytes   = sizeof(Vector3);
    return result;
}

//-----------------------------------------------------------------------------
//      ユーザーデータを設定します.
//-----------------------------------------------------------------------------
void Mesh::SetUserData(void* value)
{ m_pUserData = value; }

//-----------------------------------------------------------------------------
//      ユーザーデータを取得します.
//-----------------------------------------------------------------------------
void* Mesh::GetUserData() const
{ return m_pUserData; }


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
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Model::Init(std::vector<uint8_t>&& binary)
{
    // モデルバイナリロード.
    m_ModelBinary.Load(std::move(binary));

    // マテリアルスロット確保.
    m_MaterialSlots.resize(m_ModelBinary.GetMaterialCount());
    for(auto i=0u; i<m_ModelBinary.GetMaterialCount(); ++i)
    { m_MaterialSlots[i] = 0; }

    // メッシュ生成.
    m_Meshes.resize(m_ModelBinary.GetMeshCount());
    for(auto i=0u; i<m_ModelBinary.GetMeshCount(); ++i)
    {
        const auto& res = m_ModelBinary.GetMesh(i);
        if (!m_Meshes[i].Init(res))
        {
            ELOG("Error : Mesh Initialize Failed. index = %zu", i);
            return false;
        }
    }

    // バウンディングスフィア設定.
    m_BoundingSphere = m_ModelBinary.GetBoundingSphere();

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Model::Term()
{
    // メッシュ解放.
    for(size_t i=0; i<m_Meshes.size(); ++i)
    {
        m_Meshes[i].Term();
    }
    m_Meshes.clear();

    // マテリアルスロット解放.
    m_MaterialSlots.clear();

    // バウンディングスフィア初期化.
    m_BoundingSphere = BoundingSphere3();

    // モデルバイナリ破棄.
    m_ModelBinary.Term();
}

//-----------------------------------------------------------------------------
//      マテリアル数を取得します.
//-----------------------------------------------------------------------------
uint32_t Model::GetMaterialCount() const
{ return m_ModelBinary.GetMaterialCount(); }

//-----------------------------------------------------------------------------
//      マテリアル名を取得します.
//-----------------------------------------------------------------------------
const char* Model::GetMaterialName(uint32_t index) const
{ return m_ModelBinary.GetMaterial(index); }

//-----------------------------------------------------------------------------
//      マテリアルを取得します.
//-----------------------------------------------------------------------------
uintptr_t Model::GetMaterial(uint32_t index) const
{
    assert(index < m_MaterialSlots.size());
    return m_MaterialSlots[index];
}

//-----------------------------------------------------------------------------
//      マテリアルを設定します.
//-----------------------------------------------------------------------------
void Model::SetMaterial(uint32_t index, uintptr_t pMaterial)
{
    assert(index < m_MaterialSlots.size());
    m_MaterialSlots[index] = pMaterial;
}

//-----------------------------------------------------------------------------
//      ボーンを持つかどうかチェックします.
//-----------------------------------------------------------------------------
bool Model::HasBone() const
{ return m_ModelBinary.GetBoneCount() > 0; }

//-----------------------------------------------------------------------------
//      ボーン数を取得します.
//-----------------------------------------------------------------------------
uint32_t Model::GetBoneCount() const
{ return m_ModelBinary.GetBoneCount(); }

//-----------------------------------------------------------------------------
//      ボーン名を取得します.
//-----------------------------------------------------------------------------
const res::Bone& Model::GetBone(uint32_t index) const
{ return m_ModelBinary.GetBone(index); }

//-----------------------------------------------------------------------------
//      メッシュ数を取得します.
//-----------------------------------------------------------------------------
uint32_t Model::GetMeshCount() const
{ return uint32_t(m_Meshes.size()); }

//-----------------------------------------------------------------------------
//      メッシュを取得します.
//-----------------------------------------------------------------------------
const Mesh* Model::GetMesh(uint32_t index) const
{
    assert(index < m_Meshes.size());
    return &m_Meshes[index];
}

//-----------------------------------------------------------------------------
//      リソースメッシュを取得します.
//-----------------------------------------------------------------------------
const res::Mesh& Model::GetResMesh(uint32_t index) const
{ return m_ModelBinary.GetMesh(index); }

//-----------------------------------------------------------------------------
//      バウンディングスフィアを取得します.
//-----------------------------------------------------------------------------
const BoundingSphere3& Model::GetBoundingSphere() const
{ return m_BoundingSphere; }

//-----------------------------------------------------------------------------
//      ユーザーデータを設定します.
//-----------------------------------------------------------------------------
void Model::SetUserData(void* value)
{ m_pUserData = value; }

//-----------------------------------------------------------------------------
//      ユーザーデータを取得します.
//-----------------------------------------------------------------------------
void* Model::GetUserData() const
{ return m_pUserData; }

//-----------------------------------------------------------------------------
//      ボーン名を検索します.
//-----------------------------------------------------------------------------
bool Model::FindBone(const char* name, uint32_t& index) const
{ return m_ModelBinary.FindBone(name, index); }

//-----------------------------------------------------------------------------
//      マテリアル名を検索します.
//-----------------------------------------------------------------------------
bool Model::FindMaterial(const char* name, uint32_t& index) const
{ return m_ModelBinary.FindMaterial(name, index); }

//-----------------------------------------------------------------------------
//      メッシュ名を検索します.
//-----------------------------------------------------------------------------
bool Model::FindMesh(const char* name, uint32_t& index) const
{ return m_ModelBinary.FindMesh(name, index); }

//-----------------------------------------------------------------------------
//      モデルを生成します.
//-----------------------------------------------------------------------------
bool Model::Create(std::vector<uint8_t>&& binary, Model** ppModel)
{
    auto instance = new(std::nothrow) Model();
    if (instance == nullptr)
    {
        ELOG("Error : Out of Memory.");
        return false;
    }

    if (!instance->Init(std::move(binary)))
    {
        ELOG("Error : Model::Init() Failed.");
        instance->Release();
        return false;
    }

    (*ppModel) = instance;
    return true;
}

} // namespace asdx
