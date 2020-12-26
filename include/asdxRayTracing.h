//-----------------------------------------------------------------------------
// File : asdxRayTracing.h
// Desc : DXR Helper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <vector>
#include <d3d12.h>
#include <asdxMath.h>
#include <asdxRef.h>


namespace asdx {

#define DXR_BUILD_FLAG_NONE                 D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE
#define DXR_BUILD_FLAG_ALLOW_UPDATE         D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE
#define DXR_BUILD_FLAG_ALLOW_COMPATION      D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION
#define DXR_BUILD_FLAG_PREFER_FAST_TRACE    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE
#define DXR_BUILD_FLAG_PREFER_FAST_BUILD    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD
#define DXR_BUILD_FLAG_MINIMIZE_MEMORY      D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY
#define DXR_BUILD_FLAG_PERFORM_UPDATE       D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE

#define DXR_SST_STATE_OBJECT_CONFIG                     D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG
#define DXR_SST_GLOBAL_ROOT_SIGNATURE                   D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE
#define DXR_SST_LOCAL_ROOT_SIGNATURE                    D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE
#define DXR_SST_NODE_MASK                               D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK
#define DXR_SST_DXIL_LIBRARY                            D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY
#define DXR_SST_EXISTING_COLLECTION                     D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION
#define DXR_SST_SUBOBJECT_TO_EXPORTS_ASSOCIATION        D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION
#define DXR_SST_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION   D3D12_STATE_SUBOBJECT_TYPE_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION
#define DXR_SST_RAYTRACING_SHADER_CONFIG                D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG
#define DXR_SST_RAYTRACING_PIPELINE_CONFIG              D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG
#define DXR_SST_HIT_GROUP                               D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP
#define DXR_SST_RAYTRACING_PIPELINE_CONFIG1             D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG1

using DXR_GEOMETRY_DESC = D3D12_RAYTRACING_GEOMETRY_DESC;
using DXR_BUILD_FLAGS   = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS;
using DXR_BUILD_DESC    = D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC;


//-----------------------------------------------------------------------------
//! @brief      DXRがサポートされているかどうかチェックします.
//! 
//! @param[in]      pDevice     デバイス.
//! @retval true    DXRサポート.
//! @retval false   DXR非サポート.
//-----------------------------------------------------------------------------
bool IsSupportDXR(ID3D12Device6* pDevice);

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
    Blas();

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
    //! @brief      ビルド設定を取得します.
    //-------------------------------------------------------------------------
    DXR_BUILD_DESC GetDesc() const;

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    RefPtr<ID3D12Resource>  m_Scratch;
    RefPtr<ID3D12Resource>  m_Structure;
    DXR_BUILD_DESC          m_BuildDesc;

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
    Tlas();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~Tlas();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //-------------------------------------------------------------------------
    bool Init(
        ID3D12Device6*              pDevice, 
        D3D12_GPU_VIRTUAL_ADDRESS   instanceDescs,
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
    //! @brief      ビルド設定を取得します.
    //-------------------------------------------------------------------------
    DXR_BUILD_DESC GetDesc() const;

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    RefPtr<ID3D12Resource>  m_Scratch;
    RefPtr<ID3D12Resource>  m_Structure;
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
    SubObjects();

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

    D3D12_STATE_OBJECT_CONFIG* GetStateObjectConfig()
    { return CreateAs<D3D12_STATE_OBJECT_CONFIG>(DXR_SST_STATE_OBJECT_CONFIG); }

    D3D12_GLOBAL_ROOT_SIGNATURE* CreateGlobalRootSignature()
    { return CreateAs<D3D12_GLOBAL_ROOT_SIGNATURE>(DXR_SST_GLOBAL_ROOT_SIGNATURE); }

    D3D12_LOCAL_ROOT_SIGNATURE* CreateLocalRootSignature()
    { return CreateAs<D3D12_LOCAL_ROOT_SIGNATURE>(DXR_SST_LOCAL_ROOT_SIGNATURE); }

    D3D12_NODE_MASK* CreateNodeMask()
    { return CreateAs<D3D12_NODE_MASK>(DXR_SST_NODE_MASK); }

    D3D12_DXIL_LIBRARY_DESC* CreateDXILLibrary()
    { return CreateAs<D3D12_DXIL_LIBRARY_DESC>(DXR_SST_DXIL_LIBRARY); }

    D3D12_EXISTING_COLLECTION_DESC* CreateExistingCollectionDesc()
    { return CreateAs<D3D12_EXISTING_COLLECTION_DESC>(DXR_SST_EXISTING_COLLECTION); }

    D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION* CreateSubObjectToExportsAssociation()
    { return CreateAs<D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION>(DXR_SST_SUBOBJECT_TO_EXPORTS_ASSOCIATION); }

    D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION* CreateDXILSubOBjectToExportAssociation()
    { return CreateAs<D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION>(DXR_SST_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION); }

    D3D12_HIT_GROUP_DESC* CreateHitGroupDesc()
    { return CreateAs<D3D12_HIT_GROUP_DESC>(DXR_SST_HIT_GROUP); }

    D3D12_RAYTRACING_SHADER_CONFIG* CreateShaderConfig()
    { return CreateAs<D3D12_RAYTRACING_SHADER_CONFIG>(DXR_SST_RAYTRACING_SHADER_CONFIG); }

    D3D12_RAYTRACING_PIPELINE_CONFIG* CreatePipelineConfig()
    { return CreateAs<D3D12_RAYTRACING_PIPELINE_CONFIG>(DXR_SST_RAYTRACING_PIPELINE_CONFIG); }

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
    { return reinterpret_cast<T*>(Create(type, sizeof(T))); }
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
