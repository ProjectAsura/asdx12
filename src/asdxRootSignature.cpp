//-----------------------------------------------------------------------------
// File : asdxRootSignature.cpp
// Desc : Root Signature.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxRootSignature.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// RootSignatureDesc class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
RootSignatureDesc::RootSignatureDesc()
: m_Flags(D3D12_ROOT_SIGNATURE_FLAG_NONE)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
RootSignatureDesc::~RootSignatureDesc()
{
    m_Params.clear();
    m_Params.shrink_to_fit();

    m_Samplers.clear();
    m_Samplers.shrink_to_fit();

    {
        auto itr = m_pRange.begin();
        while(itr != m_pRange.end())
        {
            auto ptr = *itr;
            if (ptr != nullptr)
            {
                delete ptr;
                ptr = nullptr;
            }

            itr = m_pRange.erase(itr);
        }
    }
    m_pRange.clear();

}

//-----------------------------------------------------------------------------
//      定数バッファビューを追加します.
//-----------------------------------------------------------------------------
uint32_t RootSignatureDesc::AddCBV
(
    SHADER_VISIBILITY   visibility,
    uint32_t            shaderRegister,
    uint32_t            registerSpace
)
{
    D3D12_ROOT_PARAMETER param = {};
    param.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    param.Descriptor.ShaderRegister = shaderRegister;
    param.Descriptor.RegisterSpace  = registerSpace;
    param.ShaderVisibility          = D3D12_SHADER_VISIBILITY(visibility);

    auto result = uint32_t(m_Params.size());
    m_Params.push_back(param);

    return result;
}

//-----------------------------------------------------------------------------
//      シェーダリソースビューを追加します.
//-----------------------------------------------------------------------------
uint32_t RootSignatureDesc::AddSRV
(
    SHADER_VISIBILITY   visibility,
    uint32_t            shaderRegister,
    uint32_t            registerSpace
)
{
    auto range = new D3D12_DESCRIPTOR_RANGE;
    range->BaseShaderRegister                   = shaderRegister;
    range->NumDescriptors                       = 1;
    range->OffsetInDescriptorsFromTableStart    = 0;
    range->RangeType                            = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range->RegisterSpace                        = registerSpace;

    D3D12_ROOT_PARAMETER param = {};
    param.ParameterType                         = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param.DescriptorTable.NumDescriptorRanges   = 1;
    param.DescriptorTable.pDescriptorRanges     = range;
    param.ShaderVisibility                      = D3D12_SHADER_VISIBILITY(visibility);

    auto result = uint32_t(m_Params.size());
    m_Params.push_back(param);
    m_pRange.push_back(range);

    return result;
}

//-----------------------------------------------------------------------------
//      アンオーダードアクセスビューを追加します.
//-----------------------------------------------------------------------------
uint32_t RootSignatureDesc::AddUAV
(
    SHADER_VISIBILITY   visibility,
    uint32_t            shaderRegister,
    uint32_t            registerSpace
)
{
    D3D12_ROOT_PARAMETER param = {};
    param.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_UAV;
    param.Descriptor.ShaderRegister = shaderRegister;
    param.Descriptor.RegisterSpace  = registerSpace;
    param.ShaderVisibility          = D3D12_SHADER_VISIBILITY(visibility);

    auto result = uint32_t(m_Params.size());
    m_Params.push_back(param);

    return result;
}

//-----------------------------------------------------------------------------
//      サンプラーを追加します.
//-----------------------------------------------------------------------------
uint32_t RootSignatureDesc::AddSampler
(
    SHADER_VISIBILITY   visibility,
    uint32_t            shaderRegister,
    uint32_t            registerSpace
)
{
    auto range = new D3D12_DESCRIPTOR_RANGE;
    range->BaseShaderRegister                   = shaderRegister;
    range->NumDescriptors                       = 1;
    range->OffsetInDescriptorsFromTableStart    = 0;
    range->RangeType                            = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    range->RegisterSpace                        = registerSpace;

    D3D12_ROOT_PARAMETER param = {};
    param.ParameterType                         = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param.DescriptorTable.NumDescriptorRanges   = 1;
    param.DescriptorTable.pDescriptorRanges     = range;
    param.ShaderVisibility                      = D3D12_SHADER_VISIBILITY(visibility);

    auto result = uint32_t(m_Params.size());
    m_Params.push_back(param);
    m_pRange.push_back(range);

    return result;
}

//-----------------------------------------------------------------------------
//      32bit定数を追加します.
//-----------------------------------------------------------------------------
uint32_t RootSignatureDesc::AddConstant
(
    SHADER_VISIBILITY   visibility,
    uint32_t            count32BitValues,
    uint32_t            shaderRegister,
    uint32_t            registerSpace
)
{
    D3D12_ROOT_PARAMETER param = {};
    param.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    param.Constants.Num32BitValues  = count32BitValues;
    param.Constants.ShaderRegister  = shaderRegister;
    param.Constants.RegisterSpace   = registerSpace;
    param.ShaderVisibility          = D3D12_SHADER_VISIBILITY(visibility);

    auto result = uint32_t(m_Params.size());
    m_Params.push_back(param);

    return result;
}

//-----------------------------------------------------------------------------
//      静的サンプラーを追加します.
//-----------------------------------------------------------------------------
uint32_t RootSignatureDesc::AddStaticSampler
(
    SHADER_VISIBILITY   visibility,
    STATIC_SAMPLER_TYPE type,
    uint32_t            shaderRegister,
    uint32_t            registerSpace
)
{
    D3D12_STATIC_SAMPLER_DESC desc = {};

    desc.MipLODBias         = 0;
    desc.MaxAnisotropy      = 0;
    desc.ComparisonFunc     = D3D12_COMPARISON_FUNC_ALWAYS;
    desc.BorderColor        = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
    desc.MinLOD             = 0;
    desc.MaxLOD             = D3D12_FLOAT32_MAX;
    desc.ShaderRegister     = shaderRegister;
    desc.RegisterSpace      = registerSpace;
    desc.ShaderVisibility   = D3D12_SHADER_VISIBILITY(visibility);

    switch(type)
    {
    case SS_POINT_CLAMP:
        {
            desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        }
        break;

    case SS_POINT_WRAP:
        {
            desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
        break;

    case SS_POINT_MIRROR:
        {
            desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        }
        break;

    case SS_LINEAR_CLAMP:
        {
            desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        }
        break;

    case SS_LINEAR_WRAP:
        {
            desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
        break;

    case SS_LINEAR_MIRROR:
        {
            desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        }
        break;

    case SS_ANISOTROPIC_CLAMP:
        {
            desc.Filter = D3D12_FILTER_ANISOTROPIC;
            desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.MaxAnisotropy = 16;
        }
        break;

    case SS_ANISOTROPIC_WRAP:
        {
            desc.Filter = D3D12_FILTER_ANISOTROPIC;
            desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.MaxAnisotropy = 16;
        }
        break;

    case SS_ANISOTROPIC_MIRROR:
        {
            desc.Filter = D3D12_FILTER_ANISOTROPIC;
            desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            desc.MaxAnisotropy = 16;
        }
        break;
    }

    return AddStaticSampler(desc);
}

//-----------------------------------------------------------------------------
//      静的サンプラーを追加します.
//-----------------------------------------------------------------------------
uint32_t RootSignatureDesc::AddStaticSampler(D3D12_STATIC_SAMPLER_DESC desc)
{
    auto result = uint32_t(m_Samplers.size());
    m_Samplers.push_back(desc);

    return result;
}

//-----------------------------------------------------------------------------
//      フラグを設定します.
//-----------------------------------------------------------------------------
void RootSignatureDesc::SetFlag(D3D12_ROOT_SIGNATURE_FLAGS flags)
{ m_Flags = flags; }


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
bool RootSignature::Init(ID3D12Device* pDevice, RootSignatureDesc& desc)
{
    D3D12_ROOT_SIGNATURE_DESC sigDesc = {};
    sigDesc.NumParameters       = UINT(desc.m_Params.size());
    sigDesc.pParameters         = desc.m_Params.data();
    sigDesc.NumStaticSamplers   = UINT(desc.m_Samplers.size());
    sigDesc.pStaticSamplers     = desc.m_Samplers.data();
    sigDesc.Flags               = desc.m_Flags;

    asdx::RefPtr<ID3DBlob> pBlob;
    asdx::RefPtr<ID3DBlob> pErrorBlob;

    auto hr = D3D12SerializeRootSignature(
        &sigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1_0,
        pBlob.GetAddress(),
        pErrorBlob.GetAddress());
    if (FAILED(hr))
    {
        const char* msg = "";
        if (pErrorBlob != nullptr)
        { msg = static_cast<const char*>(pErrorBlob->GetBufferPointer()); }

        ELOG("Error : D3D12SerializeRootSignature() Failed. errcode = 0x%x, msg = %s", hr, msg);
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
