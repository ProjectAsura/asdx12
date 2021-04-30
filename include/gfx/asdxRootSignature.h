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
#include <core/asdxRef.h>


namespace asdx {

struct RANGE_CBV : public D3D12_DESCRIPTOR_RANGE
{
    RANGE_CBV(UINT baseRegister, UINT registerSpace = 0);
};

struct RANGE_SRV : public D3D12_DESCRIPTOR_RANGE
{
    RANGE_SRV(UINT baseRegister, UINT registerSpace = 0);
};

struct RANGE_UAV : public D3D12_DESCRIPTOR_RANGE
{
    RANGE_UAV(UINT baseRegister, UINT registerSpace = 0);
};

struct RANGE_SMP : public D3D12_DESCRIPTOR_RANGE
{
    RANGE_SMP(UINT baseRegister, UINT registerSpace = 0);
};

struct PARAM_TABLE : public D3D12_ROOT_PARAMETER
{
    PARAM_TABLE(
        D3D12_SHADER_VISIBILITY         visibility,
        UINT                            count,
        const D3D12_DESCRIPTOR_RANGE*   ranges);
};

struct PARAM_CONSTANT : public D3D12_ROOT_PARAMETER
{
    PARAM_CONSTANT(
        D3D12_SHADER_VISIBILITY visibility,
        UINT                    count,
        UINT                    baseRegister,
        UINT                    registerSpace = 0);
};

struct PARAM_CBV : public D3D12_ROOT_PARAMETER
{
    PARAM_CBV(
        D3D12_SHADER_VISIBILITY visibility,
        UINT                    baseRegister,
        UINT                    registerSpace = 0);
};

struct PARAM_SRV : public D3D12_ROOT_PARAMETER
{
    PARAM_SRV(
        D3D12_SHADER_VISIBILITY visibility,
        UINT                    baseRegister,
        UINT                    registerSpace = 0);
};

struct PARAM_UAV : public D3D12_ROOT_PARAMETER
{
    PARAM_UAV(
        D3D12_SHADER_VISIBILITY visibility,
        UINT                    baseRegister,
        UINT                    registerSpace = 0);
};


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
