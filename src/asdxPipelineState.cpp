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


namespace {

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
using PSS_NODE_MASK      = PSSubObject< UINT,                       D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK >;
using PSS_CACHED_PSO     = PSSubObject< D3D12_CACHED_PIPELINE_STATE,D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO >;
using PSS_FLAGS          = PSSubObject< D3D12_PIPELINE_STATE_FLAGS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS >;


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
    PSS_NODE_MASK       NodeMask;
    PSS_CACHED_PSO      CachedPSO;
    PSS_FLAGS           Flags;

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
        NodeMask            = pValue->NodeMask;
        CachedPSO           = pValue->CachedPSO;
        Flags               = pValue->Flags;
    }
};

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

    // パイプラインステート生成.
    {
        auto hr = pDevice->CreateGraphicsPipelineState(pDesc, IID_PPV_ARGS(m_pPSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_pPSO->SetName(L"asdxGraphicsPipelineState");
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

    // パイプラインステート生成.
    {
        auto hr = pDevice->CreateComputePipelineState(pDesc, IID_PPV_ARGS(m_pPSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_pPSO->SetName(L"asdxComputePipelineState");
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

        m_pPSO->SetName(L"asdxGeometryPipelineState");
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
{ m_pPSO.Reset(); }

//-----------------------------------------------------------------------------
//      パイプラインステートを取得します.
//-----------------------------------------------------------------------------
ID3D12PipelineState* PipelineState::GetPtr() const
{ return m_pPSO.GetPtr(); }

//-----------------------------------------------------------------------------
//      パイプラインタイプを取得します.
//-----------------------------------------------------------------------------
PIPELINE_TYPE PipelineState::GetType() const
{ return m_Type; }

//-----------------------------------------------------------------------------
//      深度ステンシルステートを取得します.
//-----------------------------------------------------------------------------
D3D12_DEPTH_STENCIL_DESC PipelineState::GetDSS(DEPTH_STATE_TYPE type, D3D12_COMPARISON_FUNC func)
{
    D3D12_DEPTH_STENCIL_DESC result = {};

    result.StencilEnable                = FALSE;
    result.StencilReadMask              = D3D12_DEFAULT_STENCIL_READ_MASK;
    result.StencilWriteMask             = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    result.FrontFace.StencilFailOp      = D3D12_STENCIL_OP_KEEP;
    result.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    result.FrontFace.StencilPassOp      = D3D12_STENCIL_OP_KEEP;
    result.FrontFace.StencilFunc        = D3D12_COMPARISON_FUNC_ALWAYS;
    result.BackFace.StencilFailOp       = D3D12_STENCIL_OP_KEEP;
    result.BackFace.StencilDepthFailOp  = D3D12_STENCIL_OP_KEEP;
    result.BackFace.StencilPassOp       = D3D12_STENCIL_OP_KEEP;
    result.BackFace.StencilFunc         = D3D12_COMPARISON_FUNC_ALWAYS;

    switch(type)
    {
    case DEPTH_STATE_DEFAULT:
        {
            result.DepthEnable      = TRUE;
            result.DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ALL;
            result.DepthFunc        = func;
        }
        break;

    case DEPTH_STATE_NONE:
        {
            result.DepthEnable      = FALSE;
            result.DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ZERO;
            result.DepthFunc        = D3D12_COMPARISON_FUNC_ALWAYS;
        }
        break;

    case DEPTH_STATE_READ_ONLY:
        {
            result.DepthEnable      = TRUE;
            result.DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ZERO;
            result.DepthFunc        = func;
        }
        break;

    case DEPTH_STATE_WRITE_ONLY:
        {
            result.DepthEnable      = FALSE;
            result.DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ALL;
            result.DepthFunc        = func;
        }
        break;
    }

    return result;
}

//-----------------------------------------------------------------------------
//      ラスタライザーステートを取得します.
//-----------------------------------------------------------------------------
D3D12_RASTERIZER_DESC PipelineState::GetRS(RASTERIZER_STATE_TYPE type)
{
    D3D12_RASTERIZER_DESC result = {};

    result.FrontCounterClockwise    = FALSE;
    result.DepthBias                = 0;
    result.DepthBiasClamp           = 0.0f;
    result.DepthClipEnable          = TRUE;
    result.MultisampleEnable        = FALSE;
    result.AntialiasedLineEnable    = FALSE;
    result.ForcedSampleCount        = 0;
    result.ConservativeRaster       = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    switch(type)
    {
    case RASTERIZER_STATE_CULL_NONE:
        {
            result.FillMode = D3D12_FILL_MODE_SOLID;
            result.CullMode = D3D12_CULL_MODE_NONE;
        }
        break;

    case RASTERIZER_STATE_CULL_BACK:
        {
            result.FillMode = D3D12_FILL_MODE_SOLID;
            result.CullMode = D3D12_CULL_MODE_BACK;
        }
        break;

    case RASTERIZER_STATE_CULL_FRONT:
        {
            result.FillMode = D3D12_FILL_MODE_SOLID;
            result.CullMode = D3D12_CULL_MODE_FRONT;
        }
        break;

    case RASTERIZER_STATE_WIREFRAME:
        {
            result.FillMode = D3D12_FILL_MODE_WIREFRAME;
            result.CullMode = D3D12_CULL_MODE_NONE;
        }
        break;
    }

    return result;
}

//-----------------------------------------------------------------------------
//      ブレンドステートを取得します.
//-----------------------------------------------------------------------------
D3D12_BLEND_DESC PipelineState::GetBS(BLEND_STATE_TYPE type)
{
    D3D12_BLEND_DESC result = {};

    result.AlphaToCoverageEnable    = FALSE;
    result.IndependentBlendEnable   = FALSE;
    result.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    switch(type)
    {
    case BLEND_STATE_OPAQUE:
        {
            result.RenderTarget[0].BlendEnable = FALSE;
            result.RenderTarget[0].SrcBlend    = result.RenderTarget[0].SrcBlendAlpha    = D3D12_BLEND_ONE;
            result.RenderTarget[0].DestBlend   = result.RenderTarget[0].DestBlendAlpha   = D3D12_BLEND_ZERO;
            result.RenderTarget[0].BlendOp     = result.RenderTarget[0].BlendOpAlpha     = D3D12_BLEND_OP_ADD;
        }
        break;

    case BLEND_STATE_ALPHABLEND:
        {
            result.RenderTarget[0].BlendEnable = TRUE;
            result.RenderTarget[0].SrcBlend    = result.RenderTarget[0].SrcBlendAlpha    = D3D12_BLEND_SRC_ALPHA;
            result.RenderTarget[0].DestBlend   = result.RenderTarget[0].DestBlendAlpha   = D3D12_BLEND_INV_SRC_ALPHA;
            result.RenderTarget[0].BlendOp     = result.RenderTarget[0].BlendOpAlpha     = D3D12_BLEND_OP_ADD;
        }
        break;

    case BLEND_STATE_ADDITIVE:
        {
            result.RenderTarget[0].BlendEnable = TRUE;
            result.RenderTarget[0].SrcBlend    = result.RenderTarget[0].SrcBlendAlpha    = D3D12_BLEND_SRC_ALPHA;
            result.RenderTarget[0].DestBlend   = result.RenderTarget[0].DestBlendAlpha   = D3D12_BLEND_ONE;
            result.RenderTarget[0].BlendOp     = result.RenderTarget[0].BlendOpAlpha     = D3D12_BLEND_OP_ADD;
        }
        break;

    case BLEND_STATE_SUBTRACT:
        {
            result.RenderTarget[0].BlendEnable = TRUE;
            result.RenderTarget[0].SrcBlend    = result.RenderTarget[0].SrcBlendAlpha    = D3D12_BLEND_SRC_ALPHA;
            result.RenderTarget[0].DestBlend   = result.RenderTarget[0].DestBlendAlpha   = D3D12_BLEND_ONE;
            result.RenderTarget[0].BlendOp     = result.RenderTarget[0].BlendOpAlpha     = D3D12_BLEND_OP_REV_SUBTRACT;
        }
        break;

    case BLEND_STATE_PREMULTIPLIED:
        {
            result.RenderTarget[0].BlendEnable = TRUE;
            result.RenderTarget[0].SrcBlend    = result.RenderTarget[0].SrcBlendAlpha    = D3D12_BLEND_ONE;
            result.RenderTarget[0].DestBlend   = result.RenderTarget[0].DestBlendAlpha   = D3D12_BLEND_INV_SRC_ALPHA;
            result.RenderTarget[0].BlendOp     = result.RenderTarget[0].BlendOpAlpha     = D3D12_BLEND_OP_ADD;
        }
        break;

    case BLEND_STATE_MULTIPLY:
        {
            result.RenderTarget[0].BlendEnable    = TRUE;
            result.RenderTarget[0].SrcBlend       = result.RenderTarget[0].SrcBlendAlpha    = D3D12_BLEND_ZERO;
            result.RenderTarget[0].DestBlend      = D3D12_BLEND_SRC_COLOR;
            result.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_SRC_ALPHA;
            result.RenderTarget[0].BlendOp        = result.RenderTarget[0].BlendOpAlpha     = D3D12_BLEND_OP_ADD;
        }
        break;

    case BLEND_STATE_SCREEN:
        {
            result.RenderTarget[0].BlendEnable    = TRUE;
            result.RenderTarget[0].SrcBlend       = D3D12_BLEND_DEST_COLOR;
            result.RenderTarget[0].SrcBlendAlpha  = D3D12_BLEND_DEST_ALPHA;
            result.RenderTarget[0].DestBlend      = result.RenderTarget[0].DestBlendAlpha   = D3D12_BLEND_ONE;
            result.RenderTarget[0].BlendOp        = result.RenderTarget[0].BlendOpAlpha     = D3D12_BLEND_OP_ADD;
        }
        break;
    }

    return result;
}

} // namespace asdx
