﻿//-----------------------------------------------------------------------------
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
#include <fnd/asdxMath.h>
#include <gfx/asdxView.h>


namespace asdx {

constexpr D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DXR_BUILD_FLAG_NONE               = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
constexpr D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DXR_BUILD_FLAG_ALLOW_UPDATE       = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
constexpr D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DXR_BUILD_FLAG_ALLOW_COMPATION    = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION;
constexpr D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DXR_BUILD_FLAG_PREFER_FAST_TRACE  = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
constexpr D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DXR_BUILD_FLAG_PREFER_FAST_BUILD  = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
constexpr D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DXR_BUILD_FLAG_MINIMIZE_MEMORY    = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
constexpr D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DXR_BUILD_FLAG_PERFORM_UPDATE     = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;

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
    //! @brief      リソースを取得します.
    //-------------------------------------------------------------------------
    ID3D12Resource* GetResource() const;

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
    //! @brief      リソースを取得します.
    //-------------------------------------------------------------------------
    ID3D12Resource* GetResource() const;

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
    ~SubObjects() = default;

    //-------------------------------------------------------------------------
    //! @brief      クリアします.
    //-------------------------------------------------------------------------
    void Clear();

    //-------------------------------------------------------------------------
    //! @brief      ステートオブジェクトの構成設定を取得します.
    //-------------------------------------------------------------------------
    D3D12_STATE_OBJECT_DESC GetDesc() const;

    //-------------------------------------------------------------------------
    //! @brief      ステートオブジェクト設定を追加します.
    //-------------------------------------------------------------------------
    void Push(const D3D12_STATE_OBJECT_CONFIG* pDesc);

    //-------------------------------------------------------------------------
    //! @brief      グローバルルートシグニチャを追加します.
    //-------------------------------------------------------------------------
    void Push(const D3D12_GLOBAL_ROOT_SIGNATURE* pDesc);

    //-------------------------------------------------------------------------
    //! @brief      ローカルルートシグニチャを追加します.
    //-------------------------------------------------------------------------
    void Push(const D3D12_LOCAL_ROOT_SIGNATURE* pDesc);

    //-------------------------------------------------------------------------
    //! @brief      ノードマスクを追加します.
    //-------------------------------------------------------------------------
    void Push(const D3D12_NODE_MASK* pDesc);

    //-------------------------------------------------------------------------
    //! @brief      DXILライブラリ設定を追加します.
    //-------------------------------------------------------------------------
    void Push(const D3D12_DXIL_LIBRARY_DESC* pDesc);

    //-------------------------------------------------------------------------
    //! @brief      既存コレクション設定を追加します.
    //-------------------------------------------------------------------------
    void Push(const D3D12_EXISTING_COLLECTION_DESC* pDesc);

    //-------------------------------------------------------------------------
    //! @brief      エクスポートを関連付けるサブオブジェクト設定を追加します.
    //-------------------------------------------------------------------------
    void Push(const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION* pDesc);

    //-------------------------------------------------------------------------
    //! @brief      エクスポートを関連付けるDXILサブオブジェクト設定を追加します.
    //-------------------------------------------------------------------------
    void Push(const D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION* pDesc);

    //-------------------------------------------------------------------------
    //! @brief      ヒットグループ設定を追加します.
    //-------------------------------------------------------------------------
    void Push(const D3D12_HIT_GROUP_DESC* pDesc);

    //-------------------------------------------------------------------------
    //! @brief      シェーダ設定を追加します.
    //-------------------------------------------------------------------------
    void Push(const D3D12_RAYTRACING_SHADER_CONFIG* pDesc);

    //-------------------------------------------------------------------------
    //! @brief      パイプライン設定を追加します.
    //-------------------------------------------------------------------------
    void Push(const D3D12_RAYTRACING_PIPELINE_CONFIG* pDesc);

    //-------------------------------------------------------------------------
    //! @brief      パイプライン設定を追加します.
    //-------------------------------------------------------------------------
    void Push(const D3D12_RAYTRACING_PIPELINE_CONFIG1* pDesc);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    static const uint32_t  kMaxCount = 32;
    D3D12_STATE_SUBOBJECT  m_Objects[kMaxCount] = {};
    uint32_t               m_Count              = 0;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
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
    ///////////////////////////////////////////////////////////////////////////
    // Desc structure
    ///////////////////////////////////////////////////////////////////////////
    struct Desc
    {
        uint32_t            RecordCount             = 0;
        uint32_t            LocalRootArgumentSize   = 0;
        const ShaderRecord* pRecords                = nullptr;
    };

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
    bool Init(ID3D12Device* pDevice, const Desc* pDesc);

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
