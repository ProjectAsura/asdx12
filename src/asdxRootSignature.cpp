//-----------------------------------------------------------------------------
// File : asdxRootSignature.cpp
// Desc : Root Signature.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <asdxRootSignature.h>
#include <asdxLogger.h>
#include <asdxShaderReflection.h>


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

    m_Tags.clear();
}

//-----------------------------------------------------------------------------
//      定数バッファビューを追加します.
//-----------------------------------------------------------------------------
uint32_t RootSignatureDesc::AddCBV
(
    const char*         tag,
    SHADER_VISIBILITY   visibility,
    uint32_t            shaderRegister,
    uint32_t            registerSpace
)
{
    uint32_t result;
    if (!Contains(tag, result))
    {
        D3D12_ROOT_PARAMETER param = {};
        param.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
        param.Descriptor.ShaderRegister = shaderRegister;
        param.Descriptor.RegisterSpace  = registerSpace;
        param.ShaderVisibility          = D3D12_SHADER_VISIBILITY(visibility);

        result = uint32_t(m_Params.size());
        m_Params.push_back(param);
        m_Tags.push_back(tag);
    }
    else
    {
        assert(m_Params[result].Descriptor.ShaderRegister == shaderRegister);
        assert(m_Params[result].Descriptor.RegisterSpace == registerSpace);

        m_Params[result].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    }

    return result;
}

//-----------------------------------------------------------------------------
//      シェーダリソースビューを追加します.
//-----------------------------------------------------------------------------
uint32_t RootSignatureDesc::AddSRV
(
    const char*         tag,
    SHADER_VISIBILITY   visibility,
    uint32_t            shaderRegister,
    uint32_t            registerSpace
)
{
    uint32_t result;
    if (!Contains(tag, result))
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
        m_Tags.push_back(tag);
    }
    else
    {
        assert(m_Params[result].DescriptorTable.pDescriptorRanges->BaseShaderRegister == shaderRegister);
        assert(m_Params[result].DescriptorTable.pDescriptorRanges->RegisterSpace == registerSpace);

        m_Params[result].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    }

    return result;
}

//-----------------------------------------------------------------------------
//      アンオーダードアクセスビューを追加します.
//-----------------------------------------------------------------------------
uint32_t RootSignatureDesc::AddUAV
(
    const char*         tag,
    SHADER_VISIBILITY   visibility,
    uint32_t            shaderRegister,
    uint32_t            registerSpace
)
{
    uint32_t result;
    if (!Contains(tag, result))
    {
        D3D12_ROOT_PARAMETER param = {};
        param.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_UAV;
        param.Descriptor.ShaderRegister = shaderRegister;
        param.Descriptor.RegisterSpace  = registerSpace;
        param.ShaderVisibility          = D3D12_SHADER_VISIBILITY(visibility);

        auto result = uint32_t(m_Params.size());
        m_Params.push_back(param);
        m_Tags.push_back(tag);
    }
    else
    {
        assert(m_Params[result].Descriptor.ShaderRegister == shaderRegister);
        assert(m_Params[result].Descriptor.RegisterSpace == registerSpace);

        m_Params[result].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    }

    return result;
}

//-----------------------------------------------------------------------------
//      サンプラーを追加します.
//-----------------------------------------------------------------------------
uint32_t RootSignatureDesc::AddSampler
(
    const char*         tag,
    SHADER_VISIBILITY   visibility,
    uint32_t            shaderRegister,
    uint32_t            registerSpace
)
{
    uint32_t result;
    if (!Contains(tag, result))
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
        m_Tags.push_back(tag);
    }
    else
    {
        assert(m_Params[result].DescriptorTable.pDescriptorRanges->BaseShaderRegister == shaderRegister);
        assert(m_Params[result].DescriptorTable.pDescriptorRanges->RegisterSpace == registerSpace);

        m_Params[result].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    }

    return result;
}

//-----------------------------------------------------------------------------
//      32bit定数を追加します.
//-----------------------------------------------------------------------------
uint32_t RootSignatureDesc::AddConstant
(
    const char*         tag,
    SHADER_VISIBILITY   visibility,
    uint32_t            count32BitValues,
    uint32_t            shaderRegister,
    uint32_t            registerSpace
)
{
    uint32_t result;
    if (!Contains(tag, result))
    {
        D3D12_ROOT_PARAMETER param = {};
        param.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param.Constants.Num32BitValues  = count32BitValues;
        param.Constants.ShaderRegister  = shaderRegister;
        param.Constants.RegisterSpace   = registerSpace;
        param.ShaderVisibility          = D3D12_SHADER_VISIBILITY(visibility);

        auto result = uint32_t(m_Params.size());
        m_Params.push_back(param);
        m_Tags.push_back(tag);
    }
    else
    {
        assert(m_Params[result].Constants.Num32BitValues == count32BitValues);
        assert(m_Params[result].Constants.ShaderRegister == shaderRegister);
        assert(m_Params[result].Constants.RegisterSpace == registerSpace);

        m_Params[result].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    }

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
    case SST_POINT_CLAMP:
        {
            desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        }
        break;

    case SST_POINT_WRAP:
        {
            desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
        break;

    case SST_POINT_MIRROR:
        {
            desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        }
        break;

    case SST_LINEAR_CLAMP:
        {
            desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        }
        break;

    case SST_LINEAR_WRAP:
        {
            desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
        break;

    case SST_LINEAR_MIRROR:
        {
            desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        }
        break;

    case SST_ANISOTROPIC_CLAMP:
        {
            desc.Filter = D3D12_FILTER_ANISOTROPIC;
            desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            desc.MaxAnisotropy = 16;
        }
        break;

    case SST_ANISOTROPIC_WRAP:
        {
            desc.Filter = D3D12_FILTER_ANISOTROPIC;
            desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc.MaxAnisotropy = 16;
        }
        break;

    case SST_ANISOTROPIC_MIRROR:
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
void RootSignatureDesc::SetFlag(uint32_t flags)
{ m_Flags = D3D12_ROOT_SIGNATURE_FLAGS(flags); }

//-----------------------------------------------------------------------------
//      シェーダからルートパラメータを追加します.
//-----------------------------------------------------------------------------
RootSignatureDesc& RootSignatureDesc::AddFromShader
(
    const void* pByteCode,
    size_t      byteCodeSize
)
{
    ShaderReflection wrapper;
    if (!wrapper.Init(pByteCode, byteCodeSize))
    { return *this; }

    D3D12_SHADER_DESC shaderDesc;
    auto hr = wrapper->GetDesc(&shaderDesc);
    if (FAILED(hr))
    { return *this; }

    auto shaderType = D3D12_SHVER_GET_TYPE(shaderDesc.Version);
    SHADER_VISIBILITY visibility = SV_ALL;
    switch(shaderType)
    {
    case D3D12_SHVER_PIXEL_SHADER:
        visibility = SV_PS;
        break;

    case D3D12_SHVER_VERTEX_SHADER:
        visibility = SV_VS;
        break;

    case D3D12_SHVER_GEOMETRY_SHADER:
        visibility = SV_GS;
        break;

    case D3D12_SHVER_HULL_SHADER:
        visibility = SV_HS;
        break;

    case D3D12_SHVER_DOMAIN_SHADER:
        visibility = SV_DS;
        break;

    case D3D12_SHVER_COMPUTE_SHADER:
        visibility = SV_ALL;
        break;

    case SHVER_LIBRARY:
    case SHVER_RAY_GENERATION:
    case SHVER_INTERSECTION:
    case SHVER_ANY_HIT:
    case SHVER_CLOSEST_HIT:
    case SHVER_MISS:
    case SHVER_CALLABLE:
        visibility = SV_ALL;
        break;

    case SHVER_MS:
        visibility = SV_MS;
        break;

    case SHVER_AS:
        visibility = SV_AS;
        break;
    }

    for(auto i=0u; i<shaderDesc.BoundResources; ++i)
    {
        D3D12_SHADER_INPUT_BIND_DESC bindDesc = {};
        hr = wrapper->GetResourceBindingDesc(i, &bindDesc);
        if (FAILED(hr))
        { continue; }

        switch(bindDesc.Type)
        {
        case D3D_SIT_CBUFFER:
            {
                auto cbv = wrapper->GetConstantBufferByName(bindDesc.Name);
                D3D12_SHADER_BUFFER_DESC bufDesc;
                cbv->GetDesc(&bufDesc);

                if (bufDesc.Size <= 16u)
                { AddConstant(bindDesc.Name, visibility, bufDesc.Size / 4, bindDesc.BindPoint, bindDesc.Space); }
                else
                { AddCBV(bindDesc.Name, visibility, bindDesc.BindPoint, bindDesc.Space); }
            }
            break;

        case D3D_SIT_TBUFFER:
        case D3D_SIT_TEXTURE:
        case D3D_SIT_STRUCTURED:
        case D3D_SIT_BYTEADDRESS:
            AddSRV(bindDesc.Name, visibility, bindDesc.BindPoint, bindDesc.Space);
            break;

        case D3D_SIT_SAMPLER:
            AddSampler(bindDesc.Name, visibility, bindDesc.BindPoint, bindDesc.Space);
            break;

        case D3D_SIT_UAV_RWTYPED:
        case D3D_SIT_UAV_RWSTRUCTURED:
        case D3D_SIT_UAV_RWBYTEADDRESS:
        case D3D_SIT_UAV_APPEND_STRUCTURED:
        case D3D_SIT_UAV_CONSUME_STRUCTURED:
        case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
            AddUAV(bindDesc.Name, visibility, bindDesc.BindPoint, bindDesc.Space);
            break;
        }
    }

    return *this;
}

//-----------------------------------------------------------------------------
//      指定したタグが含まれるかチェックします.
//-----------------------------------------------------------------------------
bool RootSignatureDesc::Contains(const std::string& value, uint32_t& index)
{
    for(size_t i=0; i<m_Tags.size(); ++i)
    {
        if (m_Tags[i] == value)
        {
            index = uint32_t(i);
            return true;
        }
    }

    index = UINT32_MAX;
    return false;
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

    for(size_t i=0; i<desc.m_Tags.size(); ++i)
    { m_RootParamIndex[desc.m_Tags[i]] = uint32_t(i); }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void RootSignature::Term()
{ m_RootSignature.Reset(); }

//-----------------------------------------------------------------------------
//      ルートパラメータ番号を探索します.
//-----------------------------------------------------------------------------
uint32_t RootSignature::Find(const char* tag) const
{
    auto itr = m_RootParamIndex.find(tag);
    if (itr == m_RootParamIndex.end())
    { return UINT32_MAX; }

    return itr->second;
}

//-----------------------------------------------------------------------------
//      ルートシグニチャを取得します.
//-----------------------------------------------------------------------------
ID3D12RootSignature* RootSignature::GetPtr() const
{ return m_RootSignature.GetPtr(); }

//-----------------------------------------------------------------------------
//      ルートパラメータをログに表示します.
//-----------------------------------------------------------------------------
void RootSignature::Dump()
{
    for(auto& itr : m_RootParamIndex)
    { ILOGA("RootParam index = %3d, name = %s", itr.second, itr.first.c_str()); }
}

} // namespace asdx
