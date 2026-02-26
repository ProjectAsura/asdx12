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
// Material class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Material::Material()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Material::~Material()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Material::Init(const res::Material& material)
{
    struct MaterialParam
    {
        Vector3 BaseColor;
        float   Alpha;
        float   Occlusion;
        float   Roughness;
        float   Metalness;
        float   Ior;
        Vector3 Emissive;
        float   Reserved;
    };

    auto size = RoundUp<uint64_t>(sizeof(MaterialParam), 256);

    // 定数バッファ初期化.
    if (!m_Buffer.Init(size))
    {
        ELOG("Error : ConstantBuffer::Init() Failed.");
        return false;
    }

    auto ptr = m_Buffer.MapAs<MaterialParam>();
    assert(ptr != nullptr);
    ptr->BaseColor = MaterialProxy::GetBaseColorFactor(material);
    ptr->Alpha     = MaterialProxy::GetAlpha(material);
    ptr->Occlusion = MaterialProxy::GetOcclusionFactor(material);
    ptr->Roughness = MaterialProxy::GetRoughnessFactor(material);
    ptr->Metalness = MaterialProxy::GetMetalnessFactor(material);
    ptr->Ior       = MaterialProxy::GetIor(material);
    ptr->Emissive  = MaterialProxy::GetEmissiveFactor(material);
    m_Buffer.Unmap();

    // ベースカラーマップ生成.
    {
        auto path = MaterialProxy::GetBaseColorMap(material);
        if (!path.is_null_or_empty())
        { m_BaseColorMap = TextureManager::Instance().GetOrCreate(path.c_str()); }

        // デフォルトを設定.
        if (!m_BaseColorMap.IsValid())
        { m_BaseColorMap = TextureManager::Instance().GetOrCreate("default.OpaqueWhite"); }
        assert(m_BaseColorMap.IsValid());
    }

    // 法線マップ生成.
    {
        auto path = MaterialProxy::GetNormalMap(material);
        if (!path.is_null_or_empty())
        { m_NormalMap = TextureManager::Instance().GetOrCreate(path.c_str()); }

        // デフォルトを設定.
        if (!m_NormalMap.IsValid())
        { m_NormalMap = TextureManager::Instance().GetOrCreate("default.Normal"); }
        assert(m_NormalMap.IsValid());
   }

    // ORMマップ生成.
    {
        auto path = MaterialProxy::GetOrmMap(material);
        if (!path.is_null_or_empty())
        { m_OrmMap = TextureManager::Instance().GetOrCreate(path.c_str()); }

        // デフォルトを設定.
        if (!m_OrmMap.IsValid())
        { m_OrmMap = TextureManager::Instance().GetOrCreate("default.Orm"); }
        assert(m_OrmMap.IsValid());
    }

    // エミッシブマップ生成.
    {
        auto path = MaterialProxy::GetEmissiveMap(material);
        if (!path.is_null_or_empty())
        { m_EmissiveMap = TextureManager::Instance().GetOrCreate(path.c_str()); }

        // デフォルトを設定.
        if (!m_EmissiveMap.IsValid())
        { m_EmissiveMap = TextureManager::Instance().GetOrCreate("default.OpaqueBlack"); }
        assert(m_EmissiveMap.IsValid());
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Material::Term()
{
    m_Buffer      .Term();
    m_BaseColorMap.Reset();
    m_NormalMap   .Reset();
    m_OrmMap      .Reset();
    m_EmissiveMap .Reset();
}

//-----------------------------------------------------------------------------
//      定数バッファを取得します.
//-----------------------------------------------------------------------------
const ConstantBuffer& Material::GetBuffer() const
{ return m_Buffer; }

//-----------------------------------------------------------------------------
//      ベースカラーマップを取得します.
//-----------------------------------------------------------------------------
const TextureHolder& Material::GetBaseColorMap() const
{ return m_BaseColorMap; }

//-----------------------------------------------------------------------------
//      法線マップを取得します.
//-----------------------------------------------------------------------------
const TextureHolder& Material::GetNormalMap() const
{ return m_NormalMap; }

//-----------------------------------------------------------------------------
//      ORMマップを取得します.
//-----------------------------------------------------------------------------
const TextureHolder& Material::GetOrmMap() const
{ return m_OrmMap; }

//-----------------------------------------------------------------------------
//      エミッシブマップを取得します.
//-----------------------------------------------------------------------------
const TextureHolder& Material::GetEmissiveMap() const
{ return m_EmissiveMap; }


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
    m_Binary.Load(std::move(binary));

    // マテリアル生成.
    m_Materials.resize(m_Binary.GetMaterialCount());
    for(auto i=0u; i<m_Binary.GetMaterialCount(); ++i)
    {
        const auto& res = m_Binary.GetMaterial(i);
        if (!m_Materials[i].Init(res))
        {
            ELOG("Error : Material Initialize Failed. index = %u", i);
            return false;
        }
    }

    // メッシュ生成.
    m_Meshes.resize(m_Binary.GetMeshCount());
    for(auto i=0u; i<m_Binary.GetMeshCount(); ++i)
    {
        const auto& res = m_Binary.GetMesh(i);
        if (!m_Meshes[i].Init(res))
        {
            ELOG("Error : Mesh Initialize Failed. index = %u", i);
            return false;
        }
    }

    // バウンディングスフィア設定.
    m_BoundingSphere = m_Binary.GetBoundingSphere();

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

    // マテリアル解放.
    for(size_t i=0; i<m_Materials.size(); ++i)
    {
        m_Materials[i].Term();
    }
    m_Materials.clear();

    // バウンディングスフィア初期化.
    m_BoundingSphere = BoundingSphere3();

    // モデルバイナリ破棄.
    m_Binary.Term();
}

//-----------------------------------------------------------------------------
//      マテリアル数を取得します.
//-----------------------------------------------------------------------------
uint32_t Model::GetMaterialCount() const
{ return m_Binary.GetMaterialCount(); }

//-----------------------------------------------------------------------------
//      リソースマテリアルを取得します.
//-----------------------------------------------------------------------------
const res::Material& Model::GetResMaterial(uint32_t index) const
{ return m_Binary.GetMaterial(index); }

//-----------------------------------------------------------------------------
//      マテリアルを取得します.
//-----------------------------------------------------------------------------
const Material* Model::GetMaterial(uint32_t index) const
{
    assert(index < m_Materials.size());
    return &m_Materials[index];
}

//-----------------------------------------------------------------------------
//      ボーンを持つかどうかチェックします.
//-----------------------------------------------------------------------------
bool Model::HasBone() const
{ return m_Binary.GetBoneCount() > 0; }

//-----------------------------------------------------------------------------
//      ボーン数を取得します.
//-----------------------------------------------------------------------------
uint32_t Model::GetBoneCount() const
{ return m_Binary.GetBoneCount(); }

//-----------------------------------------------------------------------------
//      ボーン名を取得します.
//-----------------------------------------------------------------------------
const res::Bone& Model::GetBone(uint32_t index) const
{ return m_Binary.GetBone(index); }

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
{ return m_Binary.GetMesh(index); }

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
{ return m_Binary.FindBone(name, index); }

//-----------------------------------------------------------------------------
//      マテリアル名を検索します.
//-----------------------------------------------------------------------------
bool Model::FindMaterial(const char* name, uint32_t& index) const
{ return m_Binary.FindMaterial(name, index); }

//-----------------------------------------------------------------------------
//      メッシュ名を検索します.
//-----------------------------------------------------------------------------
bool Model::FindMesh(const char* name, uint32_t& index) const
{ return m_Binary.FindMesh(name, index); }

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
