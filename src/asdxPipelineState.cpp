//-----------------------------------------------------------------------------
// File : asdxPipelineState.cpp
// Desc : Pipeline State.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <vector>
#include <d3d12shader.h>
#include <asdxPipelineState.h>
#include <asdxHashString.h>
#include <asdxLogger.h>


#ifdef ASDX_ENABLE_DXC
    #include <dxcapi.h>
#else
    #include <d3dcompiler.h>
#endif//ASDX_ENABLE_DXC


namespace {

///////////////////////////////////////////////////////////////////////////////
// ASDX_SHADER_VERSION_TYPE
///////////////////////////////////////////////////////////////////////////////
enum ASDX_SHADER_VERSION_TYPE
{
    // https://github.com/microsoft/DirectXShaderCompiler/blob/master/include/dxc/DXIL/DxilConstants.h
    ASDX_SHVER_PIXEL = 0,
    ASDX_SHVER_VERTEX,
    ASDX_SHVER_GEOMETRY,
    ASDX_SHVER_HULL,
    ASDX_SHVER_DOMAIN,
    ASDX_SHVER_COMPUTE,
    ASDX_SHVER_LIBRARY,
    ASDX_SHVER_RAY_GENERATION,
    ASDX_SHVER_INTERSECTION,
    ASDX_SHVER_ANY_HIT,
    ASDX_SHVER_CLOSEST_HIT,
    ASDX_SHVER_MISS,
    ASDX_SHVER_CALLABLE,
    ASDX_SHVER_MESH,
    ASDX_SHVER_AMPLIFICATION,
    ASDX_SHVER_INVALID,
};

#define ASDX_FOURCC(ch0, ch1, ch2, ch3) (                            \
  (uint32_t)(uint8_t)(ch0)        | (uint32_t)(uint8_t)(ch1) << 8  | \
  (uint32_t)(uint8_t)(ch2) << 16  | (uint32_t)(uint8_t)(ch3) << 24   \
)

///////////////////////////////////////////////////////////////////////////////
// PSSubObject class
///////////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable : 4324)
template<typename InnerStructType, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type, typename DefaultArg = InnerStructType>
class alignas(void*) PSSubObject
{
public:
    PSSubObject() noexcept
    : m_Type    (Type)
    , m_Inner   (DefaultArg())
    { /* DO_NOTHING */ }

    PSSubObject(InnerStructType const& value) noexcept
    : m_Type    (Type)
    , m_Inner   (value)
    { /* DO_NOTHING */ }

    PSSubObject& operator = (InnerStructType const& value) noexcept
    {
        m_Type  = Type;
        m_Inner = value;
        return *this;
    }

    operator InnerStructType const&() const noexcept 
    { return m_Inner; }

    operator InnerStructType&() noexcept 
    { return m_Inner; }

    InnerStructType* operator&() noexcept
    { return &m_Inner; }

    InnerStructType const* operator&() const noexcept
    { return &m_Inner; }

private:
    D3D12_PIPELINE_STATE_SUBOBJECT_TYPE m_Type;
    InnerStructType                     m_Inner;
};
#pragma warning(pop)

using PSS_ROOT_SIGNATURE = PSSubObject< ID3D12RootSignature*,       D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE >;
#ifdef ASDX_ENABLE_MESH_SHADER
using PSS_AS             = PSSubObject< D3D12_SHADER_BYTECODE,      D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS >;
using PSS_MS             = PSSubObject< D3D12_SHADER_BYTECODE,      D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS >;
#endif//ASDX_ENABLE_MESH_SHADER
using PSS_PS             = PSSubObject< D3D12_SHADER_BYTECODE,      D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS >;
using PSS_BLEND          = PSSubObject< D3D12_BLEND_DESC,           D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND >;
using PSS_SAMPLE_MASK    = PSSubObject< UINT,                       D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK >;
using PSS_RASTERIZER     = PSSubObject< D3D12_RASTERIZER_DESC,      D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER >;
using PSS_DEPTH_STENCIL  = PSSubObject< D3D12_DEPTH_STENCIL_DESC,   D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL >;
using PSS_RTV_FORMATS    = PSSubObject< D3D12_RT_FORMAT_ARRAY,      D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS >;
using PSS_DSV_FORMAT     = PSSubObject< DXGI_FORMAT,                D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT >;
using PSS_SAMPLE_DESC    = PSSubObject< DXGI_SAMPLE_DESC,           D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC >;


///////////////////////////////////////////////////////////////////////////////
// GPS_DESC structure
///////////////////////////////////////////////////////////////////////////////
struct GPS_DESC
{
    PSS_ROOT_SIGNATURE  RootSignature;
#ifdef ASDX_ENABLE_MESH_SHADER
    PSS_AS              AS;
    PSS_MS              MS;
#endif//ASDX_ENABLE_MESH_SHADER
    PSS_PS              PS;
    PSS_BLEND           BlendState;
    PSS_SAMPLE_MASK     SampleMask;
    PSS_RASTERIZER      RasterizerState;
    PSS_DEPTH_STENCIL   DepthStencilState;
    PSS_RTV_FORMATS     RTVFormats;
    PSS_DSV_FORMAT      DSVFormat;
    PSS_SAMPLE_DESC     SampleDesc;

    GPS_DESC()
    { /* DO_NOTHING */ }

    GPS_DESC(const asdx::GEOMETRY_PIPELINE_STATE_DESC* pValue)
    {
        RootSignature       = pValue->pRootSignature;
    #ifdef ASDX_ENABLE_MESH_SHADER
        AS                  = pValue->AS;
        MS                  = pValue->MS;
    #endif
        PS                  = pValue->PS;
        BlendState          = pValue->BlendState;
        SampleMask          = pValue->SampleMask;
        RasterizerState     = pValue->RasterizerState;
        DepthStencilState   = pValue->DepthStencilState;
        RTVFormats          = pValue->RTVFormats;
        DSVFormat           = pValue->DSVFormat;
        SampleDesc          = pValue->SampleDesc;
    }
};

//-----------------------------------------------------------------------------
//      シェーダリフレクションを生成します.
//-----------------------------------------------------------------------------
HRESULT CreateReflection(const void* pBinary, size_t binarySize, ID3D12ShaderReflection** ppReflection)
{
#ifdef ASDX_ENABLE_DXC
    const uint32_t kDXC_CP_ACP = 0;
    const uint32_t kDFCC_DXIL  = ASDX_FOURCC('D', 'X', 'I', 'L');

    asdx::RefPtr<IDxcLibrary> pLibrary;
    auto hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(pLibrary.GetAddress()));
    if (FAILED(hr))
    { return hr; }

    asdx::RefPtr<IDxcBlobEncoding> blobEncoding;
    hr = pLibrary->CreateBlobWithEncodingFromPinned(pBinary, uint32_t(binarySize), kDXC_CP_ACP, blobEncoding.GetAddress());
    if (FAILED(hr))
    { return hr; }

    asdx::RefPtr<IDxcContainerReflection> containerReflection;
    hr = DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(containerReflection.GetAddress()));
    if (FAILED(hr))
    { return hr; }

    uint32_t shaderIdx = 0;
    hr = containerReflection->Load(blobEncoding.GetPtr());
    if (FAILED(hr))
    { return hr; }

    hr = containerReflection->FindFirstPartKind(kDFCC_DXIL, &shaderIdx);
    if (FAILED(hr))
    { return hr; }

    return containerReflection->GetPartReflection(shaderIdx, IID_PPV_ARGS(ppReflection));
#else
    return D3DReflect(pBinary, binarySize, IID_PPV_ARGS(ppReflection));
#endif
}

//-----------------------------------------------------------------------------
//      エントリーを登録します.
//-----------------------------------------------------------------------------
void EnumerateEntries
(
    ID3D12ShaderReflection*                         pReflection,
    std::map<uint32_t, asdx::PipelineState::Entry>& rangeTable,
    uint32_t&                                       rootParamIndex
)
{
    D3D12_SHADER_DESC shaderDesc = {};
    auto hr = pReflection->GetDesc(&shaderDesc);
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12ShaderReflection::GetDesc() Failed.");
        return;
    }

    D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL;
    auto shaderType = D3D12_SHVER_GET_TYPE(shaderDesc.Version);
    switch(shaderType)
    {
        case ASDX_SHVER_PIXEL           : { visibility = D3D12_SHADER_VISIBILITY_PIXEL;         } break;
        case ASDX_SHVER_VERTEX          : { visibility = D3D12_SHADER_VISIBILITY_VERTEX;        } break;
        case ASDX_SHVER_GEOMETRY        : { visibility = D3D12_SHADER_VISIBILITY_GEOMETRY;      } break;
        case ASDX_SHVER_HULL            : { visibility = D3D12_SHADER_VISIBILITY_HULL;          } break;
        case ASDX_SHVER_DOMAIN          : { visibility = D3D12_SHADER_VISIBILITY_DOMAIN;        } break;
        case ASDX_SHVER_COMPUTE         : { visibility = D3D12_SHADER_VISIBILITY_ALL;           } break;
    #ifdef ASDX_ENABLE_MESH_SHADER
        case ASDX_SHVER_AMPLIFICATION   : { visibility = D3D12_SHADER_VISIBILITY_AMPLIFICATION; } break;
        case ASDX_SHVER_MESH            : { visibility = D3D12_SHADER_VISIBILITY_MESH;          } break;
    #endif
        default:
            break;
    }

    for(auto i=0u; i<shaderDesc.BoundResources; ++i)
    {
        D3D12_SHADER_INPUT_BIND_DESC bindDesc = {};

        hr = pReflection->GetResourceBindingDesc(i, &bindDesc);
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12ShaderReflection::GetResourceBindingDesc() Failed.");
            continue;
        }

        asdx::PipelineState::Entry entry = {};
        entry.Name                  = bindDesc.Name;
        entry.Type                  = bindDesc.Type;
        entry.BindPoint             = bindDesc.BindPoint;
        entry.BindCount             = bindDesc.BindCount;
        entry.RegisterSpace         = bindDesc.Space;
        entry.Visibility            = visibility;
        entry.RootParameterIndex    = rootParamIndex++;

        if (bindDesc.Type == D3D_SIT_CBUFFER)
        {
            auto pReflectionCB = pReflection->GetConstantBufferByName(bindDesc.Name);
            if (pReflectionCB != nullptr)
            {
                D3D12_SHADER_BUFFER_DESC bufDesc = {};
                hr = pReflectionCB->GetDesc(&bufDesc);
                if (SUCCEEDED(hr))
                { entry.BufferSize = bufDesc.Size; }
            }
        }

        asdx::HashString key(bindDesc.Name);
        if (rangeTable.find(key.hash()) != rangeTable.end())
        { rangeTable[key.hash()] = entry; }
    }
}

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// PipelineState class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
PipelineState::PipelineState()
: m_Type(PIPELINE_TYPE_GRAPHICS)
, m_ThreadX(0)
, m_ThreadY(0)
, m_ThreadZ(0)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
PipelineState::~PipelineState()
{ Term(); }

//-----------------------------------------------------------------------------
//      グラフィックスパイプラインとして初期化します.
//-----------------------------------------------------------------------------
bool PipelineState::Init(ID3D12Device* pDevice, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    {
        ELOG("Error : Inavlid Argument.");
        return false;
    }

    m_ThreadX = 0;
    m_ThreadY = 0;
    m_ThreadZ = 0;
    m_Entries.clear();
    uint32_t rootParamIndex = 0;

    // シェーダリフレクション生成.
    if (pDesc->VS.pShaderBytecode != nullptr && pDesc->VS.BytecodeLength > 0)
    {
        RefPtr<ID3D12ShaderReflection> pReflectionVS;
        auto hr = CreateReflection(
            pDesc->VS.pShaderBytecode,
            pDesc->VS.BytecodeLength,
            pReflectionVS.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : CreateReflection() Failed. errcode = 0x%x", hr);
            return false;
        }

        EnumerateEntries(pReflectionVS.GetPtr(), m_Entries, rootParamIndex);
    }

    if (pDesc->HS.pShaderBytecode != nullptr && pDesc->HS.BytecodeLength > 0)
    {
        RefPtr<ID3D12ShaderReflection> pReflectionHS;
        auto hr = CreateReflection(
            pDesc->HS.pShaderBytecode,
            pDesc->HS.BytecodeLength,
            pReflectionHS.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : CreateReflection() Failed. errcode = 0x%x", hr);
            return false;
        }

        EnumerateEntries(pReflectionHS.GetPtr(), m_Entries, rootParamIndex);
    }

    if (pDesc->DS.pShaderBytecode != nullptr && pDesc->DS.BytecodeLength > 0)
    {
        RefPtr<ID3D12ShaderReflection> pReflectionDS;
        auto hr = CreateReflection(
            pDesc->DS.pShaderBytecode,
            pDesc->DS.BytecodeLength,
            pReflectionDS.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : CreateReflection() Failed. errcode = 0x%x", hr);
            return false;
        }

        EnumerateEntries(pReflectionDS.GetPtr(), m_Entries, rootParamIndex);
    }

    if (pDesc->GS.pShaderBytecode != nullptr && pDesc->GS.BytecodeLength > 0)
    {
        RefPtr<ID3D12ShaderReflection> pReflectionGS;
        auto hr = CreateReflection(
            pDesc->GS.pShaderBytecode,
            pDesc->GS.BytecodeLength,
            pReflectionGS.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : CreateReflection() Failed. errcode = 0x%x", hr);
            return false;
        }

        EnumerateEntries(pReflectionGS.GetPtr(), m_Entries, rootParamIndex);
    }

    if (pDesc->PS.pShaderBytecode != nullptr && pDesc->PS.BytecodeLength > 0)
    {
        RefPtr<ID3D12ShaderReflection> pReflectionPS;
        auto hr = CreateReflection(
            pDesc->PS.pShaderBytecode,
            pDesc->PS.BytecodeLength,
            pReflectionPS.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : CreateReflection() Failed. errcode = 0x%x", hr);
            return false;
        }

        EnumerateEntries(pReflectionPS.GetPtr(), m_Entries, rootParamIndex);
    }


    // ルートシグニチャ生成.
    std::vector<D3D12_ROOT_PARAMETER>   params;
    std::vector<D3D12_DESCRIPTOR_RANGE> ranges;
    params.resize(m_Entries.size());
    ranges.resize(m_Entries.size());

    auto denyVS = true;
    auto denyHS = true;
    auto denyDS = true;
    auto denyGS = true;
    auto denyPS = true;

    for(auto& itr : m_Entries)
    {
        auto idx = itr.second.RootParameterIndex;
        switch(itr.second.Visibility)
        {
        case D3D12_SHADER_VISIBILITY_VERTEX:
            denyVS = false;
            break;

        case D3D12_SHADER_VISIBILITY_HULL:
            denyHS = false;
            break;

        case D3D12_SHADER_VISIBILITY_DOMAIN:
            denyDS = false;
            break;

        case D3D12_SHADER_VISIBILITY_GEOMETRY:
            denyGS = false;
            break;

        case D3D12_SHADER_VISIBILITY_PIXEL:
            denyPS = false;
            break;
        }

        switch(itr.second.Type)
        {
        case D3D_SIT_CBUFFER:
            {
                if (itr.second.BufferSize < 16)
                {
                    params[idx].ParameterType               = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
                    params[idx].Constants.Num32BitValues    = itr.second.BufferSize / 4;
                    params[idx].Constants.RegisterSpace     = itr.second.RegisterSpace;
                    params[idx].Constants.ShaderRegister    = itr.second.BindPoint;
                    params[idx].ShaderVisibility            = itr.second.Visibility;
                }
                else
                {
                    params[idx].ParameterType               = D3D12_ROOT_PARAMETER_TYPE_CBV;
                    params[idx].Descriptor.RegisterSpace    = itr.second.RegisterSpace;
                    params[idx].Descriptor.ShaderRegister   = itr.second.BindPoint;
                    params[idx].ShaderVisibility            = itr.second.Visibility;
                }
            }
            break;

        case D3D_SIT_TBUFFER:
        case D3D_SIT_TEXTURE:
            {
                params[idx].ParameterType               = D3D12_ROOT_PARAMETER_TYPE_SRV;
                params[idx].Descriptor.RegisterSpace    = itr.second.RegisterSpace;
                params[idx].Descriptor.ShaderRegister   = itr.second.BindPoint;
                params[idx].ShaderVisibility            = itr.second.Visibility;
            }
            break;

        case D3D_SIT_SAMPLER:
            {
                ranges[idx].RangeType           = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                ranges[idx].NumDescriptors      = itr.second.BindCount;
                ranges[idx].BaseShaderRegister  = itr.second.BindPoint;
                ranges[idx].RegisterSpace       = itr.second.RegisterSpace;
                ranges[idx].OffsetInDescriptorsFromTableStart = 0;

                params[idx].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                params[idx].DescriptorTable.NumDescriptorRanges = 1;
                params[idx].DescriptorTable.pDescriptorRanges   = &ranges[idx];
                params[idx].ShaderVisibility                    = itr.second.Visibility;
            }
            break;

        case D3D_SIT_UAV_RWTYPED:
        case D3D_SIT_UAV_RWSTRUCTURED:
        case D3D_SIT_UAV_RWBYTEADDRESS:
        case D3D_SIT_UAV_APPEND_STRUCTURED:
        case D3D_SIT_UAV_CONSUME_STRUCTURED:
        case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
            {
                params[idx].ParameterType               = D3D12_ROOT_PARAMETER_TYPE_UAV;
                params[idx].Descriptor.RegisterSpace    = itr.second.RegisterSpace;
                params[idx].Descriptor.ShaderRegister   = itr.second.BindPoint;
                params[idx].ShaderVisibility            = itr.second.Visibility;
            }
            break;
        }
    }

    D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

    if (!denyVS)
    { flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; }
    else
    { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS; }

    if (denyHS)
    { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS; }

    if (denyDS)
    { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS; }

    if (denyGS)
    { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS; }

    if (denyPS)
    { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS; }


    D3D12_ROOT_SIGNATURE_DESC sigDesc = {};
    sigDesc.NumParameters       = uint32_t(m_Entries.size());
    sigDesc.pParameters         = params.data();
    sigDesc.NumStaticSamplers   = 0;
    sigDesc.pStaticSamplers     = nullptr;
    sigDesc.Flags               = flags;

    RefPtr<ID3DBlob> pBlob;
    RefPtr<ID3DBlob> pErrorBlob;
    auto hr = D3D12SerializeRootSignature(
        &sigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, pBlob.GetAddress(), pErrorBlob.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : D3D12SerializeRootSignature() Failed. errcode = 0x%x, msg = %s",
            hr, static_cast<char*>(pErrorBlob->GetBufferPointer()));
        return false;
    }

    hr = pDevice->CreateRootSignature(
        0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(m_pRootSig.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateRootSignature() Failed. errcode = 0x%x", hr);
        return false;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = *pDesc;
    psoDesc.pRootSignature = m_pRootSig.GetPtr();

    // パイプラインステート生成.
    hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pPSO.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      コンピュートパイプラインとして初期化します.
//-----------------------------------------------------------------------------
bool PipelineState::Init(ID3D12Device* pDevice, const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc)
{
    if (pDesc == nullptr)
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    m_ThreadX = 0;
    m_ThreadY = 0;
    m_ThreadZ = 0;
    m_Entries.clear();

    uint32_t rootParamIndex = 0;

    // シェーダリフレクション生成.
    if (pDesc->CS.pShaderBytecode != nullptr && pDesc->CS.BytecodeLength > 0)
    {
        RefPtr<ID3D12ShaderReflection> pReflectionCS;
        auto hr = CreateReflection(
            pDesc->CS.pShaderBytecode,
            pDesc->CS.BytecodeLength,
            pReflectionCS.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : CreateReflection() Failed. errcode = 0x%x", hr);
            return false;
        }

        EnumerateEntries(pReflectionCS.GetPtr(), m_Entries, rootParamIndex);
        pReflectionCS->GetThreadGroupSize(&m_ThreadX, &m_ThreadY, &m_ThreadZ);
    }

    // ルートシグニチャ生成.
    std::vector<D3D12_ROOT_PARAMETER>   params;
    std::vector<D3D12_DESCRIPTOR_RANGE> ranges;
    params.resize(m_Entries.size());
    ranges.resize(m_Entries.size());

    for(auto& itr : m_Entries)
    {
        auto idx = itr.second.RootParameterIndex;

        switch(itr.second.Type)
        {
        case D3D_SIT_CBUFFER:
            {
                if (itr.second.BufferSize < 16)
                {
                    params[idx].ParameterType               = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
                    params[idx].Constants.Num32BitValues    = itr.second.BufferSize / 4;
                    params[idx].Constants.RegisterSpace     = itr.second.RegisterSpace;
                    params[idx].Constants.ShaderRegister    = itr.second.BindPoint;
                    params[idx].ShaderVisibility            = itr.second.Visibility;
                }
                else
                {
                    params[idx].ParameterType               = D3D12_ROOT_PARAMETER_TYPE_CBV;
                    params[idx].Descriptor.RegisterSpace    = itr.second.RegisterSpace;
                    params[idx].Descriptor.ShaderRegister   = itr.second.BindPoint;
                    params[idx].ShaderVisibility            = itr.second.Visibility;
                }
            }
            break;

        case D3D_SIT_TBUFFER:
        case D3D_SIT_TEXTURE:
            {
                params[idx].ParameterType               = D3D12_ROOT_PARAMETER_TYPE_SRV;
                params[idx].Descriptor.RegisterSpace    = itr.second.RegisterSpace;
                params[idx].Descriptor.ShaderRegister   = itr.second.BindPoint;
                params[idx].ShaderVisibility            = itr.second.Visibility;
            }
            break;

        case D3D_SIT_SAMPLER:
            {
                ranges[idx].RangeType           = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                ranges[idx].NumDescriptors      = itr.second.BindCount;
                ranges[idx].BaseShaderRegister  = itr.second.BindPoint;
                ranges[idx].RegisterSpace       = itr.second.RegisterSpace;
                ranges[idx].OffsetInDescriptorsFromTableStart = 0;

                params[idx].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                params[idx].DescriptorTable.NumDescriptorRanges = 1;
                params[idx].DescriptorTable.pDescriptorRanges   = &ranges[idx];
                params[idx].ShaderVisibility                    = itr.second.Visibility;
            }
            break;

        case D3D_SIT_UAV_RWTYPED:
        case D3D_SIT_UAV_RWSTRUCTURED:
        case D3D_SIT_UAV_RWBYTEADDRESS:
        case D3D_SIT_UAV_APPEND_STRUCTURED:
        case D3D_SIT_UAV_CONSUME_STRUCTURED:
        case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
            {
                params[idx].ParameterType               = D3D12_ROOT_PARAMETER_TYPE_UAV;
                params[idx].Descriptor.RegisterSpace    = itr.second.RegisterSpace;
                params[idx].Descriptor.ShaderRegister   = itr.second.BindPoint;
                params[idx].ShaderVisibility            = itr.second.Visibility;
            }
            break;
        }
    }

    D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

    D3D12_ROOT_SIGNATURE_DESC sigDesc = {};
    sigDesc.NumParameters       = uint32_t(m_Entries.size());
    sigDesc.pParameters         = params.data();
    sigDesc.NumStaticSamplers   = 0;
    sigDesc.pStaticSamplers     = nullptr;
    sigDesc.Flags               = flags;

    RefPtr<ID3DBlob> pBlob;
    RefPtr<ID3DBlob> pErrorBlob;
    auto hr = D3D12SerializeRootSignature(
        &sigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, pBlob.GetAddress(), pErrorBlob.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : D3D12SerializeRootSignature() Failed. errcode = 0x%x, msg = %s",
            hr, static_cast<char*>(pErrorBlob->GetBufferPointer()));
        return false;
    }

    hr = pDevice->CreateRootSignature(
        0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(m_pRootSig.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateRootSignature() Failed. errcode = 0x%x", hr);
        return false;
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = *pDesc;
    psoDesc.pRootSignature = m_pRootSig.GetPtr();

    // パイプラインステート生成.
    hr = pDevice->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(m_pPSO.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      ジオメトリパイプラインとして初期化します.
//-----------------------------------------------------------------------------
bool PipelineState::Init(ID3D12Device2* pDevice, const GEOMETRY_PIPELINE_STATE_DESC* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

#ifdef ASDX_ENABLE_MESH_SHADER
    // シェーダモデルをチェック.
    {
        D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_5 };
        auto hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel));
        if (FAILED(hr) || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_5))
        {
            ELOG("Error : Shader Model 6.5 is not supported.");
            return false;
        }
    }

    // メッシュシェーダをサポートしているかどうかチェック.
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS7 features = {};
        auto hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &features, sizeof(features));
        if (FAILED(hr) || (features.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED))
        {
            ELOG("Error : Mesh Shaders aren't supported.");
            return false;
        }
    }

    uint32_t rootParamIndex = 0;

    // シェーダリフレクション生成.
    if (pDesc->AS.pShaderBytecode != nullptr && pDesc->AS.BytecodeLength > 0)
    {
        RefPtr<ID3D12ShaderReflection> pReflectionAS;
        auto hr = CreateReflection(
            pDesc->AS.pShaderBytecode,
            pDesc->AS.BytecodeLength,
            pReflectionAS.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : CreateReflection() Failed. errcode = 0x%x", hr);
            return false;
        }

        EnumerateEntries(pReflectionAS.GetPtr(), m_Entries, rootParamIndex);
    }

    if (pDesc->MS.pShaderBytecode != nullptr && pDesc->MS.BytecodeLength > 0)
    {
        RefPtr<ID3D12ShaderReflection> pReflectionMS;
        auto hr = CreateReflection(
            pDesc->MS.pShaderBytecode,
            pDesc->MS.BytecodeLength,
            pReflectionMS.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : CreateReflection() Failed. errcode = 0x%x", hr);
            return false;
        }

        EnumerateEntries(pReflectionMS.GetPtr(), m_Entries, rootParamIndex);
        pReflectionMS->GetThreadGroupSize(&m_ThreadX, &m_ThreadY, &m_ThreadZ);
    }

    // ルートシグニチャ生成.
    std::vector<D3D12_ROOT_PARAMETER>   params;
    std::vector<D3D12_DESCRIPTOR_RANGE> ranges;
    params.resize(m_Entries.size());
    ranges.resize(m_Entries.size());

    auto denyAS = true;
    auto denyMS = true;
    auto denyPS = true;

    for(auto& itr : m_Entries)
    {
        auto idx = itr.second.RootParameterIndex;
        switch(itr.second.Visibility)
        {
        case D3D12_SHADER_VISIBILITY_AMPLIFICATION:
            denyAS = false;
            break;

        case D3D12_SHADER_VISIBILITY_MESH:
            denyMS = false;
            break;

        case D3D12_SHADER_VISIBILITY_PIXEL:
            denyPS = false;
            break;
        }

        switch(itr.second.Type)
        {
        case D3D_SIT_CBUFFER:
            {
                if (itr.second.BufferSize < 16)
                {
                    params[idx].ParameterType               = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
                    params[idx].Constants.Num32BitValues    = itr.second.BufferSize / 4;
                    params[idx].Constants.RegisterSpace     = itr.second.RegisterSpace;
                    params[idx].Constants.ShaderRegister    = itr.second.BindPoint;
                    params[idx].ShaderVisibility            = itr.second.Visibility;
                }
                else
                {
                    params[idx].ParameterType               = D3D12_ROOT_PARAMETER_TYPE_CBV;
                    params[idx].Descriptor.RegisterSpace    = itr.second.RegisterSpace;
                    params[idx].Descriptor.ShaderRegister   = itr.second.BindPoint;
                    params[idx].ShaderVisibility            = itr.second.Visibility;
                }
            }
            break;

        case D3D_SIT_TBUFFER:
        case D3D_SIT_TEXTURE:
            {
                params[idx].ParameterType               = D3D12_ROOT_PARAMETER_TYPE_SRV;
                params[idx].Descriptor.RegisterSpace    = itr.second.RegisterSpace;
                params[idx].Descriptor.ShaderRegister   = itr.second.BindPoint;
                params[idx].ShaderVisibility            = itr.second.Visibility;
            }
            break;

        case D3D_SIT_SAMPLER:
            {
                ranges[idx].RangeType           = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                ranges[idx].NumDescriptors      = itr.second.BindCount;
                ranges[idx].BaseShaderRegister  = itr.second.BindPoint;
                ranges[idx].RegisterSpace       = itr.second.RegisterSpace;
                ranges[idx].OffsetInDescriptorsFromTableStart = 0;

                params[idx].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                params[idx].DescriptorTable.NumDescriptorRanges = 1;
                params[idx].DescriptorTable.pDescriptorRanges   = &ranges[idx];
                params[idx].ShaderVisibility                    = itr.second.Visibility;
            }
            break;

        case D3D_SIT_UAV_RWTYPED:
        case D3D_SIT_UAV_RWSTRUCTURED:
        case D3D_SIT_UAV_RWBYTEADDRESS:
        case D3D_SIT_UAV_APPEND_STRUCTURED:
        case D3D_SIT_UAV_CONSUME_STRUCTURED:
        case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
            {
                params[idx].ParameterType               = D3D12_ROOT_PARAMETER_TYPE_UAV;
                params[idx].Descriptor.RegisterSpace    = itr.second.RegisterSpace;
                params[idx].Descriptor.ShaderRegister   = itr.second.BindPoint;
                params[idx].ShaderVisibility            = itr.second.Visibility;
            }
            break;
        }
    }

    D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

    if (denyAS)
    { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS; }

    if (denyMS)
    { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS; }

    if (denyPS)
    { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS; }


    D3D12_ROOT_SIGNATURE_DESC sigDesc = {};
    sigDesc.NumParameters       = uint32_t(m_Entries.size());
    sigDesc.pParameters         = params.data();
    sigDesc.NumStaticSamplers   = 0;
    sigDesc.pStaticSamplers     = nullptr;
    sigDesc.Flags               = flags;

    // ルートシグニチャを生成.
    {
        RefPtr<ID3DBlob> pBlob;
        RefPtr<ID3DBlob> pErrorBlob;
        auto hr = D3D12SerializeRootSignature(
            &sigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, pBlob.GetAddress(), pErrorBlob.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : D3D12SerializeRootSignature() Failed. errcode = 0x%x, msg = %s",
                hr, static_cast<char*>(pErrorBlob->GetBufferPointer()));
            return false;
        }

        hr = pDevice->CreateRootSignature(
            0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(m_pRootSig.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateRootSignature() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // ジオメトリパイプラインステートを生成.
    {
        GPS_DESC gpsDesc(pDesc);

        D3D12_PIPELINE_STATE_STREAM_DESC pssDesc = {};
        pssDesc.SizeInBytes = sizeof(gpsDesc);
        pssDesc.pPipelineStateSubobjectStream = &gpsDesc;

        // パイプラインステート生成.
        auto hr = pDevice->CreatePipelineState(&pssDesc, IID_PPV_ARGS(m_pPSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    return true;
#else
    ELOG("Error : Not Support Geometry Pipeline.");
    return false;
#endif
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void PipelineState::Term()
{
    m_pPSO.Reset();
    m_pRootSig.Reset();
    m_Entries.clear();
}

//-----------------------------------------------------------------------------
//      インデックスを検索します.
//-----------------------------------------------------------------------------
uint32_t PipelineState::FindIndex(const char* tag) const
{
    HashString input(tag);
    return FindIndex(input.hash());
}

//-----------------------------------------------------------------------------
//      インデックスを検索します.
//-----------------------------------------------------------------------------
uint32_t PipelineState::FindIndex(uint32_t hash) const
{
    auto itr = m_Entries.find(hash);
    if (itr == m_Entries.end())
    { return UINT32_MAX; }

    return itr->second.RootParameterIndex;
}

//-----------------------------------------------------------------------------
//      エントリーを検索します.
//-----------------------------------------------------------------------------
bool PipelineState::FindEntry(const char* tag, Entry* result) const
{
    HashString input(tag);
    return FindEntry(input.hash(), result);
}

//-----------------------------------------------------------------------------
//      エントリーを検索します.
//-----------------------------------------------------------------------------
bool PipelineState::FindEntry(uint32_t hash, Entry* result) const
{
    auto itr = m_Entries.find(hash);
    if (itr == m_Entries.end())
    { return false; }

    if (result != nullptr)
    { *result = itr->second; }

    return true;
}

//-----------------------------------------------------------------------------
//      描画コマンドを生成します.
//-----------------------------------------------------------------------------
void PipelineState::MakeCommand(ID3D12GraphicsCommandList* pCmd)
{
    switch(m_Type)
    {
        case PIPELINE_TYPE_GRAPHICS:
            {
                pCmd->SetGraphicsRootSignature(m_pRootSig.GetPtr());
                pCmd->SetPipelineState(m_pPSO.GetPtr());
            }
            break;

        case PIPELINE_TYPE_COMPUTE:
            {
                pCmd->SetComputeRootSignature(m_pRootSig.GetPtr());
                pCmd->SetPipelineState(m_pPSO.GetPtr());
            }
            break;

    #ifdef ASDX_ENABLE_MESH_SHADER
        case PIPELINE_TYPE_GEOMETRY:
            {
                pCmd->SetGraphicsRootSignature(m_pRootSig.GetPtr());
                pCmd->SetPipelineState(m_pPSO.GetPtr());
            }
            break;
    #endif
    }
}

//-----------------------------------------------------------------------------
//      ルートシグニチャを取得します.
//-----------------------------------------------------------------------------
ID3D12RootSignature* PipelineState::GetRootSignature() const
{ return m_pRootSig.GetPtr(); }

//-----------------------------------------------------------------------------
//      パイプラインステートを取得します.
//-----------------------------------------------------------------------------
ID3D12PipelineState* PipelineState::GetPipelineState() const
{ return m_pPSO.GetPtr(); }

//-----------------------------------------------------------------------------
//      パイプラインタイプを取得します.
//-----------------------------------------------------------------------------
PIPELINE_TYPE PipelineState::GetType() const
{ return m_Type; }

//-----------------------------------------------------------------------------
//      X方向のスレッドサイズを取得します.
//-----------------------------------------------------------------------------
uint32_t PipelineState::GetThreadX() const
{ return m_ThreadX; }

//-----------------------------------------------------------------------------
//      Y方向のスレッドサイズを取得します.
//-----------------------------------------------------------------------------
uint32_t PipelineState::GetThreadY() const
{ return m_ThreadY; }

//-----------------------------------------------------------------------------
//      Z方向のスレッドサイズを取得します.
//-----------------------------------------------------------------------------
uint32_t PipelineState::GetThreadZ() const
{ return m_ThreadZ; }


} // namespace asdx
