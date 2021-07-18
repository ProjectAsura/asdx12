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

void RangeCBV(D3D12_DESCRIPTOR_RANGE& range, UINT baseRegister, UINT registerSpace)
{
    range.RangeType                           = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    range.NumDescriptors                      = 1;
    range.BaseShaderRegister                  = baseRegister;
    range.RegisterSpace                       = registerSpace;
    range.OffsetInDescriptorsFromTableStart   = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}

void RangeSRV(D3D12_DESCRIPTOR_RANGE& range, UINT baseRegister, UINT registerSpace)
{
    range.RangeType                           = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range.NumDescriptors                      = 1;
    range.BaseShaderRegister                  = baseRegister;
    range.RegisterSpace                       = registerSpace;
    range.OffsetInDescriptorsFromTableStart   = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}

void RangeUAV(D3D12_DESCRIPTOR_RANGE& range, UINT baseRegister, UINT registerSpace)
{
    range.RangeType                           = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    range.NumDescriptors                      = 1;
    range.BaseShaderRegister                  = baseRegister;
    range.RegisterSpace                       = registerSpace;
    range.OffsetInDescriptorsFromTableStart   = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}

void RangeSmp(D3D12_DESCRIPTOR_RANGE& range, UINT baseRegister, UINT registerSpace)
{
    range.RangeType                           = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    range.NumDescriptors                      = 1;
    range.BaseShaderRegister                  = baseRegister;
    range.RegisterSpace                       = registerSpace;
    range.OffsetInDescriptorsFromTableStart   = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}

void ParamCBV
(
    D3D12_ROOT_PARAMETER&   param,
    uint8_t                 visibility,
    UINT                    baseRegister,
    UINT                    registerSpace
)
{
    param.ParameterType               = D3D12_ROOT_PARAMETER_TYPE_CBV;
    param.Descriptor.ShaderRegister   = baseRegister;
    param.Descriptor.RegisterSpace    = registerSpace;
    param.ShaderVisibility            = D3D12_SHADER_VISIBILITY(visibility);
}

void ParamSRV
(
    D3D12_ROOT_PARAMETER&   param,
    uint8_t                 visibility,
    UINT                    baseRegister,
    UINT                    registerSpace
)
{
    param.ParameterType               = D3D12_ROOT_PARAMETER_TYPE_SRV;
    param.Descriptor.ShaderRegister   = baseRegister;
    param.Descriptor.RegisterSpace    = registerSpace;
    param.ShaderVisibility            = D3D12_SHADER_VISIBILITY(visibility);
}

void ParamUAV
(
    D3D12_ROOT_PARAMETER&   param,
    uint8_t                 visibility,
    UINT                    baseRegister,
    UINT                    registerSpace
)
{
    param.ParameterType               = D3D12_ROOT_PARAMETER_TYPE_UAV;
    param.Descriptor.ShaderRegister   = baseRegister;
    param.Descriptor.RegisterSpace    = registerSpace;
    param.ShaderVisibility            = D3D12_SHADER_VISIBILITY(visibility);
}

void ParamTable
(
    D3D12_ROOT_PARAMETER&           param,
    uint8_t                         visibility,
    UINT                            count,
    const D3D12_DESCRIPTOR_RANGE*   ranges
)
{
    param.ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param.DescriptorTable.NumDescriptorRanges = count;
    param.DescriptorTable.pDescriptorRanges   = ranges;
    param.ShaderVisibility                    = D3D12_SHADER_VISIBILITY(visibility);
}

void ParamConstants
(
    D3D12_ROOT_PARAMETER&   param,
    uint8_t                 visibility,
    UINT                    count,
    UINT                    baseRegister,
    UINT                    registerSpace
)
{
    param.ParameterType               = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    param.Constants.Num32BitValues    = count;
    param.Constants.ShaderRegister    = baseRegister;
    param.Constants.RegisterSpace     = registerSpace;
    param.ShaderVisibility            = D3D12_SHADER_VISIBILITY(visibility);
}


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
