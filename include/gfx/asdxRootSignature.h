﻿//-----------------------------------------------------------------------------
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

///////////////////////////////////////////////////////////////////////////////
// STATIC_SAMPLER_TYPE enum
///////////////////////////////////////////////////////////////////////////////
enum STATIC_SAMPLER_TYPE
{
    STATIC_SAMPLER_POINT_CLAMP,
    STATIC_SAMPLER_POINT_WRAP,
    STATIC_SAMPLER_POINT_MIRROR,
    STATIC_SAMPLER_LINEAR_CLAMP,
    STATIC_SAMPLER_LINEAR_WRAP,
    STATIC_SAMPLER_LINEAR_MIRROR,
    STATIC_SAMPLER_ANISOTROPIC_CLAMP,
    STATIC_SAMPLER_ANISOTROPIC_WRAP,
    STATIC_SAMPLER_ANISOTROPIC_MIRROR,
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
// DescriptorSetLayoutBase class
///////////////////////////////////////////////////////////////////////////////
class DescriptorSetLayoutBase
{
public:
    DescriptorSetLayoutBase() = default;
    void SetCBV(uint32_t slot, uint8_t shader, uint32_t baseRegister, uint32_t registerSpace = 0);
    void SetSRV(uint32_t slot, uint8_t shader, uint32_t baseRegister, uint32_t registerSpace = 0);
    void SetUAV(uint32_t slot, uint8_t shader, uint32_t baseRegister, uint32_t registerSpace = 0);
    void SetTableCBV(uint32_t slot, uint8_t shader, uint32_t baseRegister, uint32_t registerSpace = 0);
    void SetTableSRV(uint32_t slot, uint8_t shader, uint32_t baseRegister, uint32_t registerSpace = 0);
    void SetTableUAV(uint32_t slot, uint8_t shader, uint32_t baseRegister, uint32_t registerSpace = 0);
    void SetTableSmp(uint32_t slot, uint8_t shader, uint32_t baseRegister, uint32_t registerSpace = 0);
    void SetContants(uint32_t slot, uint8_t shader, uint32_t count, uint32_t baseRegister, uint32_t registerSpace = 0);
    void SetStaticSampler(uint32_t slot, uint8_t shader, uint32_t type, uint32_t baseRegister, uint32_t registerSpace = 0);
    void SetFlags(D3D12_ROOT_SIGNATURE_FLAGS flags);
    D3D12_ROOT_SIGNATURE_DESC* GetDesc();

protected:
    D3D12_ROOT_SIGNATURE_DESC   m_Desc      = {};
    D3D12_ROOT_PARAMETER*       m_Params    = nullptr;
    D3D12_DESCRIPTOR_RANGE*     m_Ranges    = nullptr;
    D3D12_STATIC_SAMPLER_DESC*  m_Samplers  = nullptr;
};

///////////////////////////////////////////////////////////////////////////////
// DescriptorSetLayout class
///////////////////////////////////////////////////////////////////////////////
template<size_t ParamCount, size_t SamplerCount>
class DescriptorSetLayout : public DescriptorSetLayoutBase
{
public:
    DescriptorSetLayout()
    {
        m_Params    = m_StorageParams;
        m_Ranges    = m_StorageRanges;
        m_Samplers  = m_StorageSamplers;

        m_Desc.NumParameters        = ParamCount;
        m_Desc.pParameters          = m_Params;
        m_Desc.NumStaticSamplers    = SamplerCount;
        m_Desc.pStaticSamplers      = m_Samplers;
    }

private:
    D3D12_ROOT_PARAMETER        m_StorageParams  [ParamCount]   = {};
    D3D12_DESCRIPTOR_RANGE      m_StorageRanges  [ParamCount]   = {};
    D3D12_STATIC_SAMPLER_DESC   m_StorageSamplers[SamplerCount] = {};
};

template<size_t ParamCount>
class DescriptorSetLayout<ParamCount, 0> : public DescriptorSetLayoutBase
{
public:
    DescriptorSetLayout()
    {
        m_Params = m_StorageParams;
        m_Ranges = m_StorageRanges;

        m_Desc.NumParameters        = ParamCount;
        m_Desc.pParameters          = m_Params;
        m_Desc.NumStaticSamplers    = 0;
        m_Desc.pStaticSamplers      = nullptr;
    }

private:
    D3D12_ROOT_PARAMETER        m_StorageParams[ParamCount]     = {};
    D3D12_DESCRIPTOR_RANGE      m_StorageRanges[ParamCount]     = {};
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
