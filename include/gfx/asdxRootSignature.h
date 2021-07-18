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
#include <vector>
#include <fnd/asdxRef.h>


namespace asdx {

constexpr D3D12_ROOT_SIGNATURE_FLAGS ROOT_SIGNATURE_FLAG_VS_PS =
    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS       |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS   |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

constexpr D3D12_ROOT_SIGNATURE_FLAGS ROOT_SIGNATURE_FLAG_MS_PS =
    D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS   |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
    D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

enum SHADER_VISIBILITY
{
    SV_ALL = 0,
    SV_VS  = 1,
    SV_HS  = 2,
    SV_DS  = 3,
    SV_GS  = 4,
    SV_PS  = 5,
    SV_AS  = 6,
    SV_MS  = 7,
};

void RangeCBV(D3D12_DESCRIPTOR_RANGE& range, UINT baseRegister, UINT registerSpace = 0);
void RangeSRV(D3D12_DESCRIPTOR_RANGE& range, UINT baseRegister, UINT registerSpace = 0);
void RangeUAV(D3D12_DESCRIPTOR_RANGE& range, UINT baseRegister, UINT registerSpace = 0);
void RangeSmp(D3D12_DESCRIPTOR_RANGE& range, UINT baseRegister, UINT registerSpace = 0);
void ParamCBV(D3D12_ROOT_PARAMETER& param, uint8_t shader, UINT baseRegister, uint32_t registerSpace = 0);
void ParamSRV(D3D12_ROOT_PARAMETER& param, uint8_t shader, UINT baseRegister, uint32_t registerSpace = 0);
void ParamUAV(D3D12_ROOT_PARAMETER& param, uint8_t shader, UINT baseRegister, uint32_t registerSpace = 0);
void ParamTable(D3D12_ROOT_PARAMETER& param, uint8_t shader, UINT count, const D3D12_DESCRIPTOR_RANGE* ranges);
void ParamConstants(D3D12_ROOT_PARAMETER& param, uint8_t shader, UINT count, UINT baseRegister, UINT registerSpace = 0);


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
