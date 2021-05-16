//-----------------------------------------------------------------------------
// File : asdxRootSignature.h
// Desc : Root Signature.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <d3d12.h>
#include <fnd/asdxRef.h>


namespace asdx {

enum ROOT_SIGNATURE_FLAGS
{
    ROOT_SIGNATURE_FLAG_NONE = 0,
    ROOT_SIGNATURE_FLAG_ALLOW_IL = 0x01,
    ROOT_SIGNATURE_FLAG_DENY_VS = 0x02,
    ROOT_SIGNATURE_FLAG_DENY_HS = 0x04,
    ROOT_SIGNATURE_FLAG_DENY_DS = 0x08,
    ROOT_SIGNATURE_FLAG_DENY_GS = 0x10,
    ROOT_SIGNATURE_FLAG_DENY_PS = 0x20,
    ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT = 0x40,
    ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE = 0x80,
    ROOT_SIGNATURE_FLAG_DENY_AS = 0x100,
    ROOT_SIGNATURE_FLAG_DENY_MS = 0x200,
    ROOT_SIGNATURE_FLAG_HEAP_DIRECTLY_INDEXED_RES = 0x400,
    ROOT_SIGNATURE_FLAG_HEAP_DIRECTLY_INDEXED_SMP = 0x800,
};

struct RangeCbv : D3D12_DESCRIPTOR_RANGE
{
    RangeCbv(UINT baseRegister, UINT registerSpace = 0);
};

struct RangeSrv : D3D12_DESCRIPTOR_RANGE
{
    RangeSrv(UINT baseRegister, UINT registerSpace = 0);
};

struct RangeUav : D3D12_DESCRIPTOR_RANGE
{
    RangeUav(UINT baseRegister, UINT registerSpace = 0);
};

struct RangeSmp : D3D12_DESCRIPTOR_RANGE
{
    RangeSmp(UINT baseRegister, UINT registerSpace = 0);
};

struct ParamTable : D3D12_ROOT_PARAMETER
{
    ParamTable(
        D3D12_SHADER_VISIBILITY         visibility,
        UINT                            count,
        const D3D12_DESCRIPTOR_RANGE*   ranges);
};

struct ParamConstant : D3D12_ROOT_PARAMETER
{
    ParamConstant(
        D3D12_SHADER_VISIBILITY visibility,
        UINT                    count,
        UINT                    baseRegister,
        UINT                    registerSpace = 0);
};

struct ParamCbv : public D3D12_ROOT_PARAMETER
{
    ParamCbv(
        D3D12_SHADER_VISIBILITY visibility,
        UINT                    baseRegister,
        UINT                    registerSpace = 0);
};

struct ParamSrv : public D3D12_ROOT_PARAMETER
{
    ParamSrv(
        D3D12_SHADER_VISIBILITY visibility,
        UINT                    baseRegister,
        UINT                    registerSpace = 0);
};

struct ParamUav : public D3D12_ROOT_PARAMETER
{
    ParamUav(
        D3D12_SHADER_VISIBILITY visibility,
        UINT                    baseRegister,
        UINT                    registerSpace = 0);
};

//-----------------------------------------------------------------------------
//! @brief  DynamicResourcesをサポートしているかどうかチェックします.
//
//! @param[in]      pDevice     デバイス
//! @retval true    サポートしています.
//! @retval false   非サポートです.
//-----------------------------------------------------------------------------
bool CheckSupportDynamicResources(ID3D12Device8* pDevice);

///////////////////////////////////////////////////////////////////////////////
// RootSignature class
///////////////////////////////////////////////////////////////////////////////
class RootSignature
{
    //=========================================================================
    // friend classes and methods.
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
    RootSignature();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~RootSignature();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice     デバイスです.
    //! @param[in]      pDesc       ルートシグニチャ設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(ID3D12Device* pDevice, const D3D12_ROOT_SIGNATURE_DESC* pDesc);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      ルートシグニチャを取得します.
    //!
    //! @return     ルートシグニチャを返却します.
    //-------------------------------------------------------------------------
    ID3D12RootSignature* GetPtr() const;

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    asdx::RefPtr<ID3D12RootSignature>   m_RootSignature;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx
