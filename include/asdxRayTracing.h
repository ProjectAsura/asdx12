//-----------------------------------------------------------------------------
// File : asdxRayTracing.h
// Desc : DirectX Ray Tracing Helper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <vector>
#include <d3d12.h>
#include <asdxMath.h>
#include <asdxView.h>


namespace asdx {

constexpr D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DXR_BUILD_FLAG_NONE               = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
constexpr D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DXR_BUILD_FLAG_ALLOW_UPDATE       = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
constexpr D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DXR_BUILD_FLAG_ALLOW_COMPATION    = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION;
constexpr D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DXR_BUILD_FLAG_PREFER_FAST_TRACE  = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
constexpr D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DXR_BUILD_FLAG_PREFER_FAST_BUILD  = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
constexpr D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DXR_BUILD_FLAG_MINIMIZE_MEMORY    = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
constexpr D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DXR_BUILD_FLAG_PERFORM_UPDATE     = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;

constexpr D3D12_STATE_SUBOBJECT_TYPE DXR_SST_STATE_OBJECT_CONFIG                   =  D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG;
constexpr D3D12_STATE_SUBOBJECT_TYPE DXR_SST_GLOBAL_ROOT_SIGNATURE                 =  D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
constexpr D3D12_STATE_SUBOBJECT_TYPE DXR_SST_LOCAL_ROOT_SIGNATURE                  =  D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
constexpr D3D12_STATE_SUBOBJECT_TYPE DXR_SST_NODE_MASK                             =  D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK;
constexpr D3D12_STATE_SUBOBJECT_TYPE DXR_SST_DXIL_LIBRARY                          =  D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
constexpr D3D12_STATE_SUBOBJECT_TYPE DXR_SST_EXISTING_COLLECTION                   =  D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION;
constexpr D3D12_STATE_SUBOBJECT_TYPE DXR_SST_SUBOBJECT_TO_EXPORTS_ASSOCIATION      =  D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
constexpr D3D12_STATE_SUBOBJECT_TYPE DXR_SST_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION =  D3D12_STATE_SUBOBJECT_TYPE_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
constexpr D3D12_STATE_SUBOBJECT_TYPE DXR_SST_RAYTRACING_SHADER_CONFIG              =  D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
constexpr D3D12_STATE_SUBOBJECT_TYPE DXR_SST_RAYTRACING_PIPELINE_CONFIG            =  D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
constexpr D3D12_STATE_SUBOBJECT_TYPE DXR_SST_HIT_GROUP                             =  D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
constexpr D3D12_STATE_SUBOBJECT_TYPE DXR_SST_RAYTRACING_PIPELINE_CONFIG1           =  D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG1;

using DXR_GEOMETRY_DESC = D3D12_RAYTRACING_GEOMETRY_DESC;
using DXR_BUILD_FLAGS   = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS;
using DXR_BUILD_DESC    = D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC;
using DXR_INSTANCE_DESC = D3D12_RAYTRACING_INSTANCE_DESC;


//-----------------------------------------------------------------------------
//! @brief      DXRがサポートされているかどうかチェックします.
//! 
//! @param[in]      pDevice     デバイス.
//! @retval true    DXRサポート.
//! @retval false   DXR非サポート.
//-----------------------------------------------------------------------------
bool IsSupportDXR(ID3D12Device6* pDevice);

//-----------------------------------------------------------------------------
//! @brief      UAVバッファを生成します.
//! 
//! @param[in]      pDevice         デバイス
//! @param[in]      bufferSize      バッファサイズ.
//! @param[out]     ppResource      リソース格納先.
//! @param[in]      initState       初期ステート.
//! @retval true    生成に成功.
//! @retval false   生成に失敗.
//-----------------------------------------------------------------------------
bool CreateBufferUAV(
    ID3D12Device*           pDevice,
    UINT64                  bufferSize,
    ID3D12Resource**        ppResource,
    D3D12_RESOURCE_STATES   initState = D3D12_RESOURCE_STATE_COMMON);

//-----------------------------------------------------------------------------
//! @brief      SRVバッファを生成します.
//!
//! @param[in]      pDevice         デバイス
//! @param[in]      pResource       リソース
//! @param[in]      elementCount    要素数.
//! @param[in]      elementSize     要素サイズ.
//! @param[out]     ppSRV           ビュー格納先.
//! @retval true    生成に成功.
//! @retval false   生成に失敗.
//-----------------------------------------------------------------------------
bool CreateBufferSRV(
    ID3D12Device*           pDevice,
    ID3D12Resource*         pResource,
    UINT                    elementCount,
    UINT                    elementSize,
    IShaderResourceView**   ppSRV);

//-----------------------------------------------------------------------------
//! @brief      アップロードバッファを生成します.
//! 
//! @param[in]      pDevice         デバイス.
//! @param[in]      bufferSize      バッファサイズ
//! @param[out]     ppResource      リソース格納先
//! @retval true    生成に成功.
//! @retval false   生成に失敗.
//-----------------------------------------------------------------------------
bool CreateUploadBuffer(
    ID3D12Device*           pDevice,
    UINT64                  bufferSize,
    ID3D12Resource**        ppResource);


///////////////////////////////////////////////////////////////////////////////
// Transform3x4 structure
///////////////////////////////////////////////////////////////////////////////
struct Transform3x4
{
    float m[3][4] = {};

    Transform3x4()
    {
        m[0][0] = 1.0f; m[0][1] = 0.0f, m[0][2] = 0.0f; m[0][3] = 0.0f;
        m[1][0] = 0.0f; m[1][1] = 1.0f, m[1][2] = 0.0f; m[1][3] = 0.0f;
        m[2][0] = 0.0f; m[2][1] = 0.0f, m[2][2] = 1.0f; m[2][3] = 0.0f;
    }

    explicit Transform3x4(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23)
    {
        m[0][0] = m00, m[0][1] = m01, m[0][2] = m02, m[0][3] = m03;
        m[1][0] = m10, m[1][1] = m11, m[1][2] = m12, m[1][3] = m13;
        m[2][0] = m20, m[2][1] = m21, m[2][2] = m22, m[2][3] = m23;
    }
};

//-----------------------------------------------------------------------------
//! @brief      Matrix型に変換します.
//-----------------------------------------------------------------------------
inline Matrix ToMatrix(const Transform3x4& value)
{
    // 転置しながら格納.
    return Matrix(
        value.m[0][0], value.m[1][0], value.m[2][0], 0.0f,
        value.m[0][1], value.m[1][1], value.m[2][1], 0.0f,
        value.m[0][2], value.m[1][2], value.m[2][2], 0.0f,
        value.m[0][3], value.m[1][3], value.m[2][3], 1.0f );
}

//-----------------------------------------------------------------------------
//! @brief      Transform3x4型に変換します.
//-----------------------------------------------------------------------------
inline Transform3x4 FromMatrix(const Matrix& value)
{
    // 転置しながら格納.
    return Transform3x4(
        value.m[0][0], value.m[1][0], value.m[2][0], value.m[3][0],
        value.m[0][1], value.m[1][1], value.m[2][1], value.m[3][1],
        value.m[0][2], value.m[1][2], value.m[2][2], value.m[3][2]);
}

///////////////////////////////////////////////////////////////////////////////
// Blas class
///////////////////////////////////////////////////////////////////////////////
class Blas
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    Blas() = default;

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~Blas();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //-------------------------------------------------------------------------
    bool Init(
        ID3D12Device6*              pDevice,
        uint32_t                    count,
        const DXR_GEOMETRY_DESC*    pDescs,
        DXR_BUILD_FLAGS             flags);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      ビルドします.
    //-------------------------------------------------------------------------
    void Build(ID3D12GraphicsCommandList6* pCmd);

    //-------------------------------------------------------------------------
    //! @brief      ジオメトリ数を取得します.
    //-------------------------------------------------------------------------
    uint32_t GetGeometryCount() const;

    //-------------------------------------------------------------------------
    //! @brief      ジオメトリ構成を取得します.
    //-------------------------------------------------------------------------
    const DXR_GEOMETRY_DESC& GetGeometry(uint32_t index) const;

    //-------------------------------------------------------------------------
    //! @brief      ジオメトリ構成を設定します.
    //-------------------------------------------------------------------------
    void SetGeometry(uint32_t index, const DXR_GEOMETRY_DESC& desc);

    //-------------------------------------------------------------------------
    //! @brief      GPU仮想アドレスを取得します.
    //-------------------------------------------------------------------------
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    RefPtr<ID3D12Resource>          m_Scratch;
    RefPtr<ID3D12Resource>          m_Structure;
    DXR_BUILD_DESC                  m_BuildDesc;
    std::vector<DXR_GEOMETRY_DESC>  m_GeometryDesc;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////
// Tlas class
///////////////////////////////////////////////////////////////////////////////
class Tlas
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    Tlas() = default;

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~Tlas();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //-------------------------------------------------------------------------
    bool Init(
        ID3D12Device6*              pDevice, 
        uint32_t                    instanceCount,
        const DXR_INSTANCE_DESC*    pInstanceDescs,
        DXR_BUILD_FLAGS             flags);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      ビルドします.
    //-------------------------------------------------------------------------
    void Build(ID3D12GraphicsCommandList6* pCmd);

    //-------------------------------------------------------------------------
    //! @brief      メモリマッピングを行います.
    //-------------------------------------------------------------------------
    DXR_INSTANCE_DESC* Map();

    //-------------------------------------------------------------------------
    //! @brief      メモリマッピングを解除します.
    //-------------------------------------------------------------------------
    void Unmap();

    //-------------------------------------------------------------------------
    //! @brief      GPU仮想アドレスを取得します.
    //-------------------------------------------------------------------------
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    RefPtr<ID3D12Resource>  m_Scratch;
    RefPtr<ID3D12Resource>  m_Structure;
    RefPtr<ID3D12Resource>  m_Instances;
    DXR_BUILD_DESC          m_BuildDesc;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////
// SubObjects class
///////////////////////////////////////////////////////////////////////////////
class SubObjects
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    SubObjects() = default;

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~SubObjects();

    //-------------------------------------------------------------------------
    //! @brief      メモリを解放します.
    //-------------------------------------------------------------------------
    void Clear();

    //-------------------------------------------------------------------------
    //! @brief      ステートオブジェクトの構成設定を取得します.
    //-------------------------------------------------------------------------
    D3D12_STATE_OBJECT_DESC GetDesc() const;

    //-------------------------------------------------------------------------
    //! @brief      ステートオブジェクト設定を生成します.
    //-------------------------------------------------------------------------
    D3D12_STATE_OBJECT_CONFIG* CreateObjectConfig()
    { return CreateAs<D3D12_STATE_OBJECT_CONFIG>(DXR_SST_STATE_OBJECT_CONFIG); }

    //-------------------------------------------------------------------------
    //! @brief      グローバルルートシグニチャを生成します.
    //-------------------------------------------------------------------------
    D3D12_GLOBAL_ROOT_SIGNATURE* CreateGlobalRootSignature()
    { return CreateAs<D3D12_GLOBAL_ROOT_SIGNATURE>(DXR_SST_GLOBAL_ROOT_SIGNATURE); }

    //-------------------------------------------------------------------------
    //! @brief      ローカルルートシグニチャを生成します.
    //-------------------------------------------------------------------------
    D3D12_LOCAL_ROOT_SIGNATURE* CreateLocalRootSignature()
    { return CreateAs<D3D12_LOCAL_ROOT_SIGNATURE>(DXR_SST_LOCAL_ROOT_SIGNATURE); }

    //-------------------------------------------------------------------------
    //! @brief      ノードマスクを生成します.
    //-------------------------------------------------------------------------
    D3D12_NODE_MASK* CreateNodeMask()
    { return CreateAs<D3D12_NODE_MASK>(DXR_SST_NODE_MASK); }

    //-------------------------------------------------------------------------
    //! @brief      DXILライブラリ設定を生成します.
    //-------------------------------------------------------------------------
    D3D12_DXIL_LIBRARY_DESC* CreateDXILLibrary()
    { return CreateAs<D3D12_DXIL_LIBRARY_DESC>(DXR_SST_DXIL_LIBRARY); }

    //-------------------------------------------------------------------------
    //! @brief      既存コレクション設定を生成します.
    //-------------------------------------------------------------------------
    D3D12_EXISTING_COLLECTION_DESC* CreateExistingCollectionDesc()
    { return CreateAs<D3D12_EXISTING_COLLECTION_DESC>(DXR_SST_EXISTING_COLLECTION); }

    //-------------------------------------------------------------------------
    //! @brief      エクスポートを関連付けるサブジェクトを生成します.
    //-------------------------------------------------------------------------
    D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION* CreateSubObjectToExportsAssociation()
    { return CreateAs<D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION>(DXR_SST_SUBOBJECT_TO_EXPORTS_ASSOCIATION); }

    //-------------------------------------------------------------------------
    //! @brief      エクスポートを関連付けるDXILサブオブジェクトを生成します.
    //-------------------------------------------------------------------------
    D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION* CreateDXILSubOBjectToExportAssociation()
    { return CreateAs<D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION>(DXR_SST_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION); }

    //-------------------------------------------------------------------------
    //! @brief      ヒットグループ設定を生成します.
    //-------------------------------------------------------------------------
    D3D12_HIT_GROUP_DESC* CreateHitGroupDesc()
    { return CreateAs<D3D12_HIT_GROUP_DESC>(DXR_SST_HIT_GROUP); }

    //-------------------------------------------------------------------------
    //! @brief      シェーダ設定を生成します.
    //-------------------------------------------------------------------------
    D3D12_RAYTRACING_SHADER_CONFIG* CreateShaderConfig()
    { return CreateAs<D3D12_RAYTRACING_SHADER_CONFIG>(DXR_SST_RAYTRACING_SHADER_CONFIG); }

    //-------------------------------------------------------------------------
    //! @brief      パイプライン設定を生成します.
    //-------------------------------------------------------------------------
    D3D12_RAYTRACING_PIPELINE_CONFIG* CreatePipelineConfig()
    { return CreateAs<D3D12_RAYTRACING_PIPELINE_CONFIG>(DXR_SST_RAYTRACING_PIPELINE_CONFIG); }

    //-------------------------------------------------------------------------
    //! @brief      パイプライン設定1を生成します.
    //-------------------------------------------------------------------------
    D3D12_RAYTRACING_PIPELINE_CONFIG1* CreatePipelineConfig1()
    { return CreateAs<D3D12_RAYTRACING_PIPELINE_CONFIG1>(DXR_SST_RAYTRACING_PIPELINE_CONFIG1); }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::vector<D3D12_STATE_SUBOBJECT>  m_Objects;

    //=========================================================================
    // private methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      サブオブジェクトを生成します.
    //! 
    //! @param[in]      type        サブオブジェクトタイプ.
    //! @param[in]      size        構造体サイズ.
    //! @return     アロケートしたメモリを返却します.
    //-------------------------------------------------------------------------
    void* Create(D3D12_STATE_SUBOBJECT_TYPE type, size_t size);

    //-------------------------------------------------------------------------
    //! @brief      型を指定してサブオブジェクトを生成します.
    //-------------------------------------------------------------------------
    template<typename T>
    T* CreateAs(D3D12_STATE_SUBOBJECT_TYPE type)
    {
        auto buf = Create(type, sizeof(T));
        return new(buf) T();
    }
};

///////////////////////////////////////////////////////////////////////////////
// ShaderRecord structure
///////////////////////////////////////////////////////////////////////////////
struct ShaderRecord
{
    void* ShaderIdentifier      = nullptr;
    void* LocalRootArguments    = nullptr;
};

///////////////////////////////////////////////////////////////////////////////
// ShaderTable class
///////////////////////////////////////////////////////////////////////////////
class ShaderTable
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    ShaderTable();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~ShaderTable();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //-------------------------------------------------------------------------
    bool Init(
        ID3D12Device*       pDevice,
        uint32_t            recordCount,
        const ShaderRecord* records,
        uint32_t            localRootArgumentSize = 0);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      GPU仮想アドレスと範囲を取得します.
    //-------------------------------------------------------------------------
    D3D12_GPU_VIRTUAL_ADDRESS_RANGE GetRecordView() const;

    //-------------------------------------------------------------------------
    //! @brief      GPU仮想アドレスと範囲とストライドを取得します.
    //-------------------------------------------------------------------------
    D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE GetTableView() const;

private:
    //=========================================================================
    // private varables.
    //=========================================================================
    RefPtr<ID3D12Resource>  m_Resource;
    UINT                    m_RecordSize;
};

} // namespace asdx
