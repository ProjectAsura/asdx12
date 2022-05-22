//-----------------------------------------------------------------------------
// File : asdxRootSignature.cpp
// Desc : Root Signature.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxRootSignature.h>
#include <fnd/asdxLogger.h>


namespace asdx {

//-----------------------------------------------------------------------------
//      DynamicResourcesをサポートしているかどうかチェックします.
//-----------------------------------------------------------------------------
bool CheckSupportDynamicResources(ID3D12Device8* pDevice)
{
    // D3D12_RESOURCE_BINDING_TIER3 と D3D_SHADER_MODEL_6_6 以上であることが必須.
    // また、シェーダコンパイルする側で
    // D3D_SHADER_REQUIRES_RESOURCE_HEAP_INDEXING(0x02000000)
    // D3D_SHADER_REQUIRES_SAMPLER_HEAP_INDEXING(0x04000000)
    // のフラグを設定しておく必要がある.

    D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
    if (FAILED(pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options))))
    { return false; }

    if (options.ResourceBindingTier != D3D12_RESOURCE_BINDING_TIER_3)
    { return false; }

    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = {};
    if (FAILED(pDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))))
    { return false; }

    if (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_6)
    { return false; }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// DescriptorSetLayoutBase class
///////////////////////////////////////////////////////////////////////////////
void DescriptorSetLayoutBase::SetCBV(uint32_t slot, uint8_t shader, uint32_t baseRegister, uint32_t registerSpace)
{
    m_Params[slot].ParameterType                = D3D12_ROOT_PARAMETER_TYPE_CBV;
    m_Params[slot].ShaderVisibility             = D3D12_SHADER_VISIBILITY(shader);
    m_Params[slot].Descriptor.ShaderRegister    = baseRegister;
    m_Params[slot].Descriptor.RegisterSpace     = registerSpace;
}

void DescriptorSetLayoutBase::SetSRV(uint32_t slot, uint8_t shader, uint32_t baseRegister, uint32_t registerSpace)
{
    m_Params[slot].ParameterType                = D3D12_ROOT_PARAMETER_TYPE_SRV;
    m_Params[slot].ShaderVisibility             = D3D12_SHADER_VISIBILITY(shader);
    m_Params[slot].Descriptor.ShaderRegister    = baseRegister;
    m_Params[slot].Descriptor.RegisterSpace     = registerSpace;
}

void DescriptorSetLayoutBase::SetUAV(uint32_t slot, uint8_t shader, uint32_t baseRegister, uint32_t registerSpace)
{
    m_Params[slot].ParameterType                = D3D12_ROOT_PARAMETER_TYPE_UAV;
    m_Params[slot].ShaderVisibility             = D3D12_SHADER_VISIBILITY(shader);
    m_Params[slot].Descriptor.ShaderRegister    = baseRegister;
    m_Params[slot].Descriptor.RegisterSpace     = registerSpace;
}

void DescriptorSetLayoutBase::SetTableCBV(uint32_t slot, uint8_t shader, uint32_t baseRegister, uint32_t registerSpace)
{
    m_Ranges[slot].RangeType                            = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    m_Ranges[slot].NumDescriptors                       = 1;
    m_Ranges[slot].BaseShaderRegister                   = baseRegister;
    m_Ranges[slot].RegisterSpace                        = registerSpace;
    m_Ranges[slot].OffsetInDescriptorsFromTableStart    = 0;

    m_Params[slot].ParameterType                        = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    m_Params[slot].ShaderVisibility                     = D3D12_SHADER_VISIBILITY(shader);
    m_Params[slot].DescriptorTable.NumDescriptorRanges  = 1;
    m_Params[slot].DescriptorTable.pDescriptorRanges    = &m_Ranges[slot];
}

void DescriptorSetLayoutBase::SetTableSRV(uint32_t slot, uint8_t shader, uint32_t baseRegister, uint32_t registerSpace)
{
    m_Ranges[slot].RangeType                            = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    m_Ranges[slot].NumDescriptors                       = 1;
    m_Ranges[slot].BaseShaderRegister                   = baseRegister;
    m_Ranges[slot].RegisterSpace                        = registerSpace;
    m_Ranges[slot].OffsetInDescriptorsFromTableStart    = 0;

    m_Params[slot].ParameterType                        = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    m_Params[slot].ShaderVisibility                     = D3D12_SHADER_VISIBILITY(shader);
    m_Params[slot].DescriptorTable.NumDescriptorRanges  = 1;
    m_Params[slot].DescriptorTable.pDescriptorRanges    = &m_Ranges[slot];
}

void DescriptorSetLayoutBase::SetTableUAV(uint32_t slot, uint8_t shader, uint32_t baseRegister, uint32_t registerSpace)
{
    m_Ranges[slot].RangeType                            = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    m_Ranges[slot].NumDescriptors                       = 1;
    m_Ranges[slot].BaseShaderRegister                   = baseRegister;
    m_Ranges[slot].RegisterSpace                        = registerSpace;
    m_Ranges[slot].OffsetInDescriptorsFromTableStart    = 0;

    m_Params[slot].ParameterType                        = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    m_Params[slot].ShaderVisibility                     = D3D12_SHADER_VISIBILITY(shader);
    m_Params[slot].DescriptorTable.NumDescriptorRanges  = 1;
    m_Params[slot].DescriptorTable.pDescriptorRanges    = &m_Ranges[slot];
}

void DescriptorSetLayoutBase::SetTableSmp(uint32_t slot, uint8_t shader, uint32_t baseRegister, uint32_t registerSpace)
{
    m_Ranges[slot].RangeType                            = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    m_Ranges[slot].NumDescriptors                       = 1;
    m_Ranges[slot].BaseShaderRegister                   = baseRegister;
    m_Ranges[slot].RegisterSpace                        = registerSpace;
    m_Ranges[slot].OffsetInDescriptorsFromTableStart    = 0;

    m_Params[slot].ParameterType                        = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    m_Params[slot].ShaderVisibility                     = D3D12_SHADER_VISIBILITY(shader);
    m_Params[slot].DescriptorTable.NumDescriptorRanges  = 1;
    m_Params[slot].DescriptorTable.pDescriptorRanges    = &m_Ranges[slot];
}

void DescriptorSetLayoutBase::SetContants(uint32_t slot, uint8_t shader, uint32_t count, uint32_t baseRegister, uint32_t registerSpace)
{
    m_Params[slot].ParameterType            = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    m_Params[slot].ShaderVisibility         = D3D12_SHADER_VISIBILITY(shader);
    m_Params[slot].Constants.ShaderRegister = baseRegister;
    m_Params[slot].Constants.RegisterSpace  = registerSpace;
    m_Params[slot].Constants.Num32BitValues = count;
}

void DescriptorSetLayoutBase::SetStaticSampler(uint32_t slot, uint8_t shader, uint32_t type, uint32_t baseRegister, uint32_t registerSpace)
{
    m_Samplers[slot].MipLODBias         = 0;
    m_Samplers[slot].MaxAnisotropy      = 0;
    m_Samplers[slot].ComparisonFunc     = D3D12_COMPARISON_FUNC_ALWAYS;
    m_Samplers[slot].BorderColor        = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
    m_Samplers[slot].MinLOD             = 0;
    m_Samplers[slot].MaxLOD             = D3D12_FLOAT32_MAX;
    m_Samplers[slot].ShaderRegister     = baseRegister;
    m_Samplers[slot].RegisterSpace      = registerSpace;
    m_Samplers[slot].ShaderVisibility   = D3D12_SHADER_VISIBILITY(shader);

    switch(type)
    {
    case STATIC_SAMPLER_POINT_CLAMP:
        {
            m_Samplers[slot].Filter     = D3D12_FILTER_MIN_MAG_MIP_POINT;
            m_Samplers[slot].AddressU   = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            m_Samplers[slot].AddressV   = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            m_Samplers[slot].AddressW   = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        }
        break;

    case STATIC_SAMPLER_POINT_WRAP:
        {
            m_Samplers[slot].Filter     = D3D12_FILTER_MIN_MAG_MIP_POINT;
            m_Samplers[slot].AddressU   = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            m_Samplers[slot].AddressV   = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            m_Samplers[slot].AddressW   = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
        break;

    case STATIC_SAMPLER_POINT_MIRROR:
        {
            m_Samplers[slot].Filter     = D3D12_FILTER_MIN_MAG_MIP_POINT;
            m_Samplers[slot].AddressU   = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            m_Samplers[slot].AddressV   = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            m_Samplers[slot].AddressW   = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        }
        break;

    case STATIC_SAMPLER_LINEAR_CLAMP:
        {
            m_Samplers[slot].Filter     = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            m_Samplers[slot].AddressU   = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            m_Samplers[slot].AddressV   = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            m_Samplers[slot].AddressW   = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        }
        break;

    case STATIC_SAMPLER_LINEAR_WRAP:
        {
            m_Samplers[slot].Filter     = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            m_Samplers[slot].AddressU   = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            m_Samplers[slot].AddressV   = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            m_Samplers[slot].AddressW   = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
        break;

    case STATIC_SAMPLER_LINEAR_MIRROR:
        {
            m_Samplers[slot].Filter     = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            m_Samplers[slot].AddressU   = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            m_Samplers[slot].AddressV   = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            m_Samplers[slot].AddressW   = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        }
        break;

    case STATIC_SAMPLER_ANISOTROPIC_CLAMP:
        {
            m_Samplers[slot].Filter         = D3D12_FILTER_ANISOTROPIC;
            m_Samplers[slot].AddressU       = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            m_Samplers[slot].AddressV       = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            m_Samplers[slot].AddressW       = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            m_Samplers[slot].MaxAnisotropy  = 16;
        }
        break;

    case STATIC_SAMPLER_ANISOTROPIC_WRAP:
        {
            m_Samplers[slot].Filter         = D3D12_FILTER_ANISOTROPIC;
            m_Samplers[slot].AddressU       = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            m_Samplers[slot].AddressV       = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            m_Samplers[slot].AddressW       = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            m_Samplers[slot].MaxAnisotropy  = 16;
        }
        break;

    case STATIC_SAMPLER_ANISOTROPIC_MIRROR:
        {
            m_Samplers[slot].Filter         = D3D12_FILTER_ANISOTROPIC;
            m_Samplers[slot].AddressU       = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            m_Samplers[slot].AddressV       = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            m_Samplers[slot].AddressW       = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            m_Samplers[slot].MaxAnisotropy  = 16;
        }
        break;
    }
}

void DescriptorSetLayoutBase::SetFlags(D3D12_ROOT_SIGNATURE_FLAGS flags)
{ m_Desc.Flags = flags; }

D3D12_ROOT_SIGNATURE_DESC* DescriptorSetLayoutBase::GetDesc()
{ return &m_Desc; }


///////////////////////////////////////////////////////////////////////////////
// RootSignature class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
RootSignature::RootSignature()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
RootSignature::~RootSignature()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool RootSignature::Init(ID3D12Device* pDevice, const D3D12_ROOT_SIGNATURE_DESC* pDesc)
{
    asdx::RefPtr<ID3DBlob> pBlob;
    asdx::RefPtr<ID3DBlob> pErrorBlob;

    auto hr = D3D12SerializeRootSignature(
        pDesc,
        D3D_ROOT_SIGNATURE_VERSION_1_0,
        pBlob.GetAddress(),
        pErrorBlob.GetAddress());
    if (FAILED(hr))
    {
        const char* msg = "";
        if (pErrorBlob != nullptr)
        { msg = static_cast<const char*>(pErrorBlob->GetBufferPointer()); }

        ELOGA("Error : D3D12SerializeRootSignature() Failed. errcode = 0x%x, msg = %s", hr, msg);
        return false;
    }

    hr = pDevice->CreateRootSignature(
        0,
        pBlob->GetBufferPointer(),
        pBlob->GetBufferSize(),
        IID_PPV_ARGS(m_RootSignature.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateRootSignature() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void RootSignature::Term()
{ m_RootSignature.Reset(); }

//-----------------------------------------------------------------------------
//      ルートシグニチャを取得します.
//-----------------------------------------------------------------------------
ID3D12RootSignature* RootSignature::GetPtr() const
{ return m_RootSignature.GetPtr(); }

} // namespace asdx
