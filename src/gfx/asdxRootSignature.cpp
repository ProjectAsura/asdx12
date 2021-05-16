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

RangeCbv::RangeCbv(UINT baseRegister, UINT registerSpace)
{
    RangeType                           = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    NumDescriptors                      = 1;
    BaseShaderRegister                  = baseRegister;
    RegisterSpace                       = registerSpace;
    OffsetInDescriptorsFromTableStart   = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}

RangeSrv::RangeSrv(UINT baseRegister, UINT registerSpace)
{
    RangeType                           = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    NumDescriptors                      = 1;
    BaseShaderRegister                  = baseRegister;
    RegisterSpace                       = registerSpace;
    OffsetInDescriptorsFromTableStart   = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}

RangeUav::RangeUav(UINT baseRegister, UINT registerSpace)
{
    RangeType                           = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    NumDescriptors                      = 1;
    BaseShaderRegister                  = baseRegister;
    RegisterSpace                       = registerSpace;
    OffsetInDescriptorsFromTableStart   = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}

RangeSmp::RangeSmp(UINT baseRegister, UINT registerSpace)
{
    RangeType                           = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    NumDescriptors                      = 1;
    BaseShaderRegister                  = baseRegister;
    RegisterSpace                       = registerSpace;
    OffsetInDescriptorsFromTableStart   = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}

ParamTable::ParamTable
(
    D3D12_SHADER_VISIBILITY         visibility,
    UINT                            count,
    const D3D12_DESCRIPTOR_RANGE*   ranges
)
{
    ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    DescriptorTable.NumDescriptorRanges = count;
    DescriptorTable.pDescriptorRanges   = ranges;
    ShaderVisibility                    = visibility;
}

ParamConstant::ParamConstant
(
    D3D12_SHADER_VISIBILITY visibility,
    UINT                    count,
    UINT                    baseRegister,
    UINT                    registerSpace
)
{
    ParameterType               = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    Constants.Num32BitValues    = count;
    Constants.ShaderRegister    = baseRegister;
    Constants.RegisterSpace     = registerSpace;
    ShaderVisibility            = visibility;
}

ParamCbv::ParamCbv
(
    D3D12_SHADER_VISIBILITY visibility,
    UINT                    baseRegister,
    UINT                    registerSpace
)
{
    ParameterType               = D3D12_ROOT_PARAMETER_TYPE_CBV;
    Descriptor.ShaderRegister   = baseRegister;
    Descriptor.RegisterSpace    = registerSpace;
    ShaderVisibility            = visibility;
}

ParamSrv::ParamSrv
(
    D3D12_SHADER_VISIBILITY visibility,
    UINT                    baseRegister,
    UINT                    registerSpace
)
{
    ParameterType               = D3D12_ROOT_PARAMETER_TYPE_SRV;
    Descriptor.ShaderRegister   = baseRegister;
    Descriptor.RegisterSpace    = registerSpace;
    ShaderVisibility            = visibility;
}

ParamUav::ParamUav
(
    D3D12_SHADER_VISIBILITY visibility,
    UINT                    baseRegister,
    UINT                    registerSpace
)
{
    ParameterType               = D3D12_ROOT_PARAMETER_TYPE_UAV;
    Descriptor.ShaderRegister   = baseRegister;
    Descriptor.RegisterSpace    = registerSpace;
    ShaderVisibility            = visibility;
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
