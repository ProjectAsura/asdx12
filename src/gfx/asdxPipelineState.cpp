//-----------------------------------------------------------------------------
// File : asdxPipelineState.cpp
// Desc : Pipeline State.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <d3d12shader.h>
#include <fnd/asdxLogger.h>
#include <gfx/asdxPipelineState.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxShaderCompiler.h>


namespace {

enum ROOT_PARAM_TYPE
{
    ROOT_PARAM_VAR = 0,
    ROOT_PARAM_CBV = 1,
    ROOT_PARAM_SRV = 2,
    ROOT_PARAM_UAV = 3,
    ROOT_PARAM_SMP = 4,
    ROOT_PARAM_AS  = 5,
};

///////////////////////////////////////////////////////////////////////////////
// PSSubObject class
///////////////////////////////////////////////////////////////////////////////
template<typename StructType, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type>
class alignas(void*) SubObject
{
public:
    SubObject() noexcept
    : m_Type    (Type)
    , m_Value   (StructType())
    { /* DO_NOTHING */ }

    SubObject(StructType const& value) noexcept
    : m_Type    (Type)
    , m_Value   (value)
    { /* DO_NOTHING */ }

    SubObject& operator = (StructType const& value) noexcept
    {
        m_Type  = Type;
        m_Value = value;
        return *this;
    }

    operator StructType const&() const noexcept 
    { return m_Value; }

    operator StructType&() noexcept 
    { return m_Value; }

    StructType* operator&() noexcept
    { return &m_Value; }

    StructType const* operator&() const noexcept
    { return &m_Value; }

private:
    D3D12_PIPELINE_STATE_SUBOBJECT_TYPE m_Type;
    StructType                          m_Value;
};

#define PSST(x) D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_##x

using PSS_ROOT_SIGNATURE = SubObject< ID3D12RootSignature*,        PSST(ROOT_SIGNATURE) >;
#ifdef ASDX_ENABLE_MESH_SHADER
using PSS_AS             = SubObject< D3D12_SHADER_BYTECODE,       PSST(AS) >;
using PSS_MS             = SubObject< D3D12_SHADER_BYTECODE,       PSST(MS) >;
#endif//ASDX_ENABLE_MESH_SHADER
using PSS_PS             = SubObject< D3D12_SHADER_BYTECODE,       PSST(PS) >;
using PSS_BLEND          = SubObject< D3D12_BLEND_DESC,            PSST(BLEND) >;
using PSS_SAMPLE_MASK    = SubObject< UINT,                        PSST(SAMPLE_MASK) >;
using PSS_RASTERIZER     = SubObject< D3D12_RASTERIZER_DESC,       PSST(RASTERIZER) >;
using PSS_DEPTH_STENCIL  = SubObject< D3D12_DEPTH_STENCIL_DESC,    PSST(DEPTH_STENCIL) >;
using PSS_RTV_FORMATS    = SubObject< D3D12_RT_FORMAT_ARRAY,       PSST(RENDER_TARGET_FORMATS) >;
using PSS_DSV_FORMAT     = SubObject< DXGI_FORMAT,                 PSST(DEPTH_STENCIL_FORMAT) >;
using PSS_SAMPLE_DESC    = SubObject< DXGI_SAMPLE_DESC,            PSST(SAMPLE_DESC) >;
using PSS_NODE_MASK      = SubObject< UINT,                        PSST(NODE_MASK) >;
using PSS_CACHED_PSO     = SubObject< D3D12_CACHED_PIPELINE_STATE, PSST(CACHED_PSO) >;
using PSS_FLAGS          = SubObject< D3D12_PIPELINE_STATE_FLAGS,  PSST(FLAGS) >;

#undef PSST


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


uint16_t MakeKey(uint8_t shaderType, uint8_t paramType, uint8_t registerIndex)
{
    return uint16_t(((shaderType & 0xf) << 12) | ((paramType & 0xf) << 8) | registerIndex);
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

} // namespace


namespace asdx {

//-----------------------------------------------------------------------------
//      深度ステンシルステートを取得します.
//-----------------------------------------------------------------------------
DEPTH_STENCIL_DESC::DEPTH_STENCIL_DESC(DEPTH_STATE_TYPE type, D3D12_COMPARISON_FUNC depthFunc)
{
    StencilEnable                = FALSE;
    StencilReadMask              = D3D12_DEFAULT_STENCIL_READ_MASK;
    StencilWriteMask             = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    FrontFace.StencilFailOp      = D3D12_STENCIL_OP_KEEP;
    FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    FrontFace.StencilPassOp      = D3D12_STENCIL_OP_KEEP;
    FrontFace.StencilFunc        = D3D12_COMPARISON_FUNC_ALWAYS;
    BackFace.StencilFailOp       = D3D12_STENCIL_OP_KEEP;
    BackFace.StencilDepthFailOp  = D3D12_STENCIL_OP_KEEP;
    BackFace.StencilPassOp       = D3D12_STENCIL_OP_KEEP;
    BackFace.StencilFunc         = D3D12_COMPARISON_FUNC_ALWAYS;

    switch(type)
    {
    case DEPTH_STATE_DEFAULT:
        {
            DepthEnable      = TRUE;
            DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ALL;
            DepthFunc        = depthFunc;
        }
        break;

    case DEPTH_STATE_NONE:
        {
            DepthEnable      = FALSE;
            DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ZERO;
            DepthFunc        = D3D12_COMPARISON_FUNC_ALWAYS;
        }
        break;

    case DEPTH_STATE_READ_ONLY:
        {
            DepthEnable      = TRUE;
            DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ZERO;
            DepthFunc        = depthFunc;
        }
        break;

    case DEPTH_STATE_WRITE_ONLY:
        {
            DepthEnable      = FALSE;
            DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ALL;
            DepthFunc        = depthFunc;
        }
        break;
    }
}

//-----------------------------------------------------------------------------
//      ラスタライザーステートを取得します.
//-----------------------------------------------------------------------------
RASTERIZER_DESC::RASTERIZER_DESC(RASTERIZER_STATE_TYPE type)
{
    FrontCounterClockwise    = FALSE;
    DepthBias                = D3D12_DEFAULT_DEPTH_BIAS;
    DepthBiasClamp           = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    SlopeScaledDepthBias     = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    DepthClipEnable          = TRUE;
    MultisampleEnable        = FALSE;
    AntialiasedLineEnable    = FALSE;
    ForcedSampleCount        = 0;
    ConservativeRaster       = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    switch(type)
    {
    case RASTERIZER_STATE_CULL_NONE:
        {
            FillMode = D3D12_FILL_MODE_SOLID;
            CullMode = D3D12_CULL_MODE_NONE;
        }
        break;

    case RASTERIZER_STATE_CULL_BACK:
        {
            FillMode = D3D12_FILL_MODE_SOLID;
            CullMode = D3D12_CULL_MODE_BACK;
        }
        break;

    case RASTERIZER_STATE_CULL_FRONT:
        {
            FillMode = D3D12_FILL_MODE_SOLID;
            CullMode = D3D12_CULL_MODE_FRONT;
        }
        break;

    case RASTERIZER_STATE_WIREFRAME:
        {
            FillMode = D3D12_FILL_MODE_WIREFRAME;
            CullMode = D3D12_CULL_MODE_NONE;
        }
        break;
    }
}

//-----------------------------------------------------------------------------
//      ブレンドステートを取得します.
//-----------------------------------------------------------------------------
BLEND_DESC::BLEND_DESC(BLEND_STATE_TYPE type)
{
    AlphaToCoverageEnable    = FALSE;
    IndependentBlendEnable   = FALSE;
    RenderTarget[0].RenderTargetWriteMask    = D3D12_COLOR_WRITE_ENABLE_ALL;
    RenderTarget[0].LogicOpEnable            = FALSE;
    RenderTarget[0].LogicOp                  = D3D12_LOGIC_OP_NOOP;

    switch(type)
    {
    case BLEND_STATE_OPAQUE:
        {
            RenderTarget[0].BlendEnable = FALSE;
            RenderTarget[0].SrcBlend    = RenderTarget[0].SrcBlendAlpha    = D3D12_BLEND_ONE;
            RenderTarget[0].DestBlend   = RenderTarget[0].DestBlendAlpha   = D3D12_BLEND_ZERO;
            RenderTarget[0].BlendOp     = RenderTarget[0].BlendOpAlpha     = D3D12_BLEND_OP_ADD;
        }
        break;

    case BLEND_STATE_ALPHABLEND:
        {
            RenderTarget[0].BlendEnable = TRUE;
            RenderTarget[0].SrcBlend    = RenderTarget[0].SrcBlendAlpha    = D3D12_BLEND_SRC_ALPHA;
            RenderTarget[0].DestBlend   = RenderTarget[0].DestBlendAlpha   = D3D12_BLEND_INV_SRC_ALPHA;
            RenderTarget[0].BlendOp     = RenderTarget[0].BlendOpAlpha     = D3D12_BLEND_OP_ADD;
        }
        break;

    case BLEND_STATE_ADDITIVE:
        {
            RenderTarget[0].BlendEnable = TRUE;
            RenderTarget[0].SrcBlend    = RenderTarget[0].SrcBlendAlpha    = D3D12_BLEND_SRC_ALPHA;
            RenderTarget[0].DestBlend   = RenderTarget[0].DestBlendAlpha   = D3D12_BLEND_ONE;
            RenderTarget[0].BlendOp     = RenderTarget[0].BlendOpAlpha     = D3D12_BLEND_OP_ADD;
        }
        break;

    case BLEND_STATE_SUBTRACT:
        {
            RenderTarget[0].BlendEnable = TRUE;
            RenderTarget[0].SrcBlend    = RenderTarget[0].SrcBlendAlpha    = D3D12_BLEND_SRC_ALPHA;
            RenderTarget[0].DestBlend   = RenderTarget[0].DestBlendAlpha   = D3D12_BLEND_ONE;
            RenderTarget[0].BlendOp     = RenderTarget[0].BlendOpAlpha     = D3D12_BLEND_OP_REV_SUBTRACT;
        }
        break;

    case BLEND_STATE_PREMULTIPLIED:
        {
            RenderTarget[0].BlendEnable = TRUE;
            RenderTarget[0].SrcBlend    = RenderTarget[0].SrcBlendAlpha    = D3D12_BLEND_ONE;
            RenderTarget[0].DestBlend   = RenderTarget[0].DestBlendAlpha   = D3D12_BLEND_INV_SRC_ALPHA;
            RenderTarget[0].BlendOp     = RenderTarget[0].BlendOpAlpha     = D3D12_BLEND_OP_ADD;
        }
        break;

    case BLEND_STATE_MULTIPLY:
        {
            RenderTarget[0].BlendEnable    = TRUE;
            RenderTarget[0].SrcBlend       = RenderTarget[0].SrcBlendAlpha    = D3D12_BLEND_ZERO;
            RenderTarget[0].DestBlend      = D3D12_BLEND_SRC_COLOR;
            RenderTarget[0].DestBlendAlpha = D3D12_BLEND_SRC_ALPHA;
            RenderTarget[0].BlendOp        = RenderTarget[0].BlendOpAlpha     = D3D12_BLEND_OP_ADD;
        }
        break;

    case BLEND_STATE_SCREEN:
        {
            RenderTarget[0].BlendEnable    = TRUE;
            RenderTarget[0].SrcBlend       = D3D12_BLEND_DEST_COLOR;
            RenderTarget[0].SrcBlendAlpha  = D3D12_BLEND_DEST_ALPHA;
            RenderTarget[0].DestBlend      = RenderTarget[0].DestBlendAlpha   = D3D12_BLEND_ONE;
            RenderTarget[0].BlendOp        = RenderTarget[0].BlendOpAlpha     = D3D12_BLEND_OP_ADD;
        }
        break;
    }
}


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
bool PipelineState::Init(ID3D12Device8* pDevice, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    {
        ELOG("Error : Inavlid Argument.");
        return false;
    }

    m_VS.resize(pDesc->VS.BytecodeLength);
    memcpy(m_VS.data(), pDesc->VS.pShaderBytecode, m_VS.size());

    m_PS.resize(pDesc->PS.BytecodeLength);
    memcpy(m_PS.data(), pDesc->PS.pShaderBytecode, m_PS.size());

    m_Type = PIPELINE_TYPE_GRAPHICS;
    m_Desc.Graphics = *pDesc;
    m_Desc.Graphics.VS.pShaderBytecode = m_VS.data();
    m_Desc.Graphics.PS.pShaderBytecode = m_PS.data();

    if (pDesc->pRootSignature == nullptr)
    {
        if (!CreateGraphicsRootSignature(pDevice, m_pRootSig.GetAddress()))
        {
            ELOG("Error : CreateGraphicsRootSignature() Failed");
            return false;
        }

        m_Desc.Graphics.pRootSignature = m_pRootSig.GetPtr();
    }

    // パイプラインステート生成.
    {
        auto hr = pDevice->CreateGraphicsPipelineState(&m_Desc.Graphics, IID_PPV_ARGS(m_pPSO.GetAddress()));
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
bool PipelineState::Init(ID3D12Device8* pDevice, const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc)
{
    if (pDesc == nullptr)
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    m_CS.resize(pDesc->CS.BytecodeLength);
    memcpy(m_CS.data(), pDesc->CS.pShaderBytecode, m_CS.size());

    m_Type = PIPELINE_TYPE_COMPUTE;
    m_Desc.Compute = *pDesc;
    m_Desc.Compute.CS.pShaderBytecode = m_CS.data();

    if (pDesc->pRootSignature == nullptr)
    {
        if (!CreateComputeRootSignature(pDevice, m_pRootSig.GetAddress()))
        {
            ELOG("Error : CreateComputeRootSignature() Failed.");
            return false;
        }

        m_Desc.Compute.pRootSignature = m_pRootSig.GetPtr();
    }

    // パイプラインステート生成.
    {
        auto hr = pDevice->CreateComputePipelineState(&m_Desc.Compute, IID_PPV_ARGS(m_pPSO.GetAddress()));
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
bool PipelineState::Init(ID3D12Device8* pDevice, const GEOMETRY_PIPELINE_STATE_DESC* pDesc)
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

    m_MS.resize(pDesc->MS.BytecodeLength);
    memcpy(m_MS.data(), pDesc->MS.pShaderBytecode, m_MS.size());

    if (pDesc->AS.BytecodeLength > 0)
    {
        m_AS.resize(pDesc->AS.BytecodeLength);
        memcpy(m_AS.data(), pDesc->AS.pShaderBytecode, m_AS.size());
    }
    else
    {
        m_AS.clear();
    }

    m_PS.resize(pDesc->PS.BytecodeLength);
    memcpy(m_PS.data(), pDesc->PS.pShaderBytecode, m_PS.size());

    m_Type = PIPELINE_TYPE_GEOMETRY;
    m_Desc.Geometry = *pDesc;

    m_Desc.Geometry.MS.pShaderBytecode = m_MS.data();
    m_Desc.Geometry.PS.pShaderBytecode = m_PS.data();

    if (pDesc->AS.BytecodeLength > 0)
    { m_Desc.Geometry.AS.pShaderBytecode = m_AS.data(); }

    if (pDesc->pRootSignature == nullptr)
    {
        if (!CreateGeometryRootSignature(pDevice, m_pRootSig.GetAddress()))
        {
            ELOG("Error : CreateGeometryRootSignature() Failed.");
            return false;
        }

        m_Desc.Geometry.pRootSignature = m_pRootSig.GetPtr();
    }

    // ジオメトリパイプラインステートを生成.
    {
        GPS_DESC gpsDesc(&m_Desc.Geometry);

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
{
    m_pRecreatePSO.Reset();
    m_pRecreateRootSig.Reset();
    m_pRootSig.Reset();
    m_pPSO.Reset();
    m_VS.clear();
    m_PS.clear();
    m_CS.clear();
    m_MS.clear();
    m_AS.clear();
}

//-----------------------------------------------------------------------------
//      頂点シェーダを差し替えます.
//-----------------------------------------------------------------------------
void PipelineState::ReplaceShader(SHADER_TYPE type, const void* pBinary, size_t binarySize)
{
    switch(type)
    {
    case SHADER_TYPE_VS:
        {
            if (m_Type != PIPELINE_TYPE_GRAPHICS)
            { return; }

            m_VS.resize(binarySize);
            memcpy(m_VS.data(), pBinary, binarySize);

            m_Desc.Graphics.VS.pShaderBytecode = m_VS.data();
            m_Desc.Graphics.VS.BytecodeLength  = m_VS.size();
        }
        break;

    case SHADER_TYPE_PS:
        {
            if (m_Type != PIPELINE_TYPE_GRAPHICS)
            { return; }

            m_PS.resize(binarySize);
            memcpy(m_PS.data(), pBinary, binarySize);

            m_Desc.Graphics.PS.pShaderBytecode = m_PS.data();
            m_Desc.Graphics.PS.BytecodeLength  = m_PS.size();
        }
        break;

    case SHADER_TYPE_AS:
        {
            if (m_Type != PIPELINE_TYPE_GEOMETRY)
            { return; }

            m_AS.resize(binarySize);
            memcpy(m_AS.data(), pBinary, binarySize);

            m_Desc.Geometry.AS.pShaderBytecode = m_AS.data();
            m_Desc.Geometry.AS.BytecodeLength  = m_AS.size();
        }
        break;

    case SHADER_TYPE_MS:
        {
            if (m_Type != PIPELINE_TYPE_GEOMETRY)
            { return; }

            m_MS.resize(binarySize);
            memcpy(m_MS.data(), pBinary, binarySize);

            m_Desc.Geometry.MS.pShaderBytecode = m_MS.data();
            m_Desc.Geometry.MS.BytecodeLength  = m_MS.size();
        }
        break;

    case SHADER_TYPE_CS:
        {
            if (m_Type != PIPELINE_TYPE_COMPUTE)
            { return; }

            m_CS.resize(binarySize);
            memcpy(m_CS.data(), pBinary, binarySize);

            m_Desc.Compute.CS.pShaderBytecode = m_CS.data();
            m_Desc.Compute.CS.BytecodeLength  = m_CS.size();
        }
        break;

    default:
        break;
    }
}

//-----------------------------------------------------------------------------
//      パイプラインステートを再生成します.
//-----------------------------------------------------------------------------
void PipelineState::Rebuild()
{
    if (!m_pRecreatePSO.GetPtr())
    {
        auto pso = m_pRecreatePSO.Detach();
        Dispose(pso);
    }

    if (!m_pRecreateRootSig.GetPtr())
    {
        auto rootSig = m_pRecreateRootSig.Detach();
        Dispose(rootSig);
    }

    if (m_Type == PIPELINE_TYPE_GRAPHICS)
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = m_Desc.Graphics;

        if (m_pRootSig.GetPtr() != nullptr)
        {
            if (!CreateGraphicsRootSignature(GetD3D12Device(), m_pRecreateRootSig.GetAddress()))
            {
                ELOGA("Error : CreateGraphicsRootSignature() Failed.");
                return;
            }

            desc.pRootSignature = m_pRecreateRootSig.GetPtr();
        }

        auto hr = GetD3D12Device()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_pRecreatePSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
            return;
        }

         m_pRecreatePSO->SetName(L"asdxGraphicsPipelineState_Reload");
    }
    else if (m_Type == PIPELINE_TYPE_COMPUTE)
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = m_Desc.Compute;

        if (m_pRootSig.GetPtr() != nullptr)
        {
            if (!CreateComputeRootSignature(GetD3D12Device(), m_pRecreateRootSig.GetAddress()))
            {
                ELOGA("Error : CreateGraphicsRootSignature() Failed.");
                return;
            }

            desc.pRootSignature = m_pRecreateRootSig.GetPtr();
        }

        auto hr = GetD3D12Device()->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_pRecreatePSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return;
        }

        m_pRecreatePSO->SetName(L"asdxComputePipelineState_Reload");
    }
    else
    {
        GPS_DESC gpsDesc(&m_Desc.Geometry);

        if (m_pRootSig.GetPtr() != nullptr)
        {
            if (!CreateGeometryRootSignature(GetD3D12Device(), m_pRecreateRootSig.GetAddress()))
            {
                ELOGA("Error : CreateGeometryRootSignature() Failed.");
                return;
            }

            gpsDesc.RootSignature = m_pRecreateRootSig.GetPtr();
        }

        D3D12_PIPELINE_STATE_STREAM_DESC pssDesc = {};
        pssDesc.SizeInBytes = sizeof(gpsDesc);
        pssDesc.pPipelineStateSubobjectStream = &gpsDesc;

        // パイプラインステート生成.
        auto hr = GetD3D12Device()->CreatePipelineState(&pssDesc, IID_PPV_ARGS(m_pRecreatePSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
            return;
        }

        m_pRecreatePSO->SetName(L"asdxGeometryPipelineState_Reload");
    }
}

//-----------------------------------------------------------------------------
//      パイプラインタイプを取得します.
//-----------------------------------------------------------------------------
PIPELINE_TYPE PipelineState::GetType() const
{ return m_Type; }

//-----------------------------------------------------------------------------
//      パイプラインステートを設定します.
//-----------------------------------------------------------------------------
void PipelineState::SetState(ID3D12GraphicsCommandList* pCmdList)
{
    if (m_Type == PIPELINE_TYPE_COMPUTE)
    {
        if (m_pRootSig.GetPtr() != nullptr)
        {
            auto sig = (m_pRecreateRootSig.GetPtr() != nullptr)
                ? m_pRecreateRootSig.GetPtr()
                : m_pRootSig.GetPtr();
            pCmdList->SetComputeRootSignature(sig);
        }
    }
    else
    {
        if (m_pRootSig.GetPtr() != nullptr)
        {
            auto sig = (m_pRecreateRootSig.GetPtr() != nullptr)
                ? m_pRecreateRootSig.GetPtr()
                : m_pRootSig.GetPtr();
            pCmdList->SetGraphicsRootSignature(sig);
        }
    }

    auto pso = (m_pRecreatePSO.GetPtr() != nullptr) ? m_pRecreatePSO.GetPtr() : m_pPSO.GetPtr();
    pCmdList->SetPipelineState(pso);
}

//-----------------------------------------------------------------------------
//      ルート定数を設定します.
//-----------------------------------------------------------------------------
void PipelineState::SetConstants
(
    ID3D12GraphicsCommandList*  pCmdList,
    SHADER_TYPE                 type,
    uint32_t                    registerIndex,
    uint32_t                    paramCount,
    const void*                 params,
    uint32_t                    offset
)
{
    auto index = FindIndex(type, ROOT_PARAM_UAV, registerIndex);
    if (index == UINT32_MAX)
    { return; }

    if (type == SHADER_TYPE_CS)
    { pCmdList->SetComputeRoot32BitConstants(index, paramCount, params, offset); }
    else
    { pCmdList->SetGraphicsRoot32BitConstants(index, paramCount, params, offset); }
}

//-----------------------------------------------------------------------------
//      定数バッファを設定します.
//-----------------------------------------------------------------------------
void PipelineState::SetCBV
(
    ID3D12GraphicsCommandList*  pCmdList,
    SHADER_TYPE                 type,
    uint32_t                    registerIndex,
    IConstantBufferView*        pView
)
{
    auto index = FindIndex(type, ROOT_PARAM_CBV, registerIndex);
    if (index == UINT32_MAX)
    { return; }

    if (type == SHADER_TYPE_CS)
    { pCmdList->SetComputeRootDescriptorTable(index, pView->GetHandleGPU()); }
    else
    { pCmdList->SetGraphicsRootDescriptorTable(index, pView->GetHandleGPU()); }
}

//-----------------------------------------------------------------------------
//      シェーダリソースビューを設定します.
//-----------------------------------------------------------------------------
void PipelineState::SetSRV
(
    ID3D12GraphicsCommandList*  pCmdList,
    SHADER_TYPE                 type,
    uint32_t                    registerIndex,
    IShaderResourceView*        pView
)
{
    auto index = FindIndex(type, ROOT_PARAM_SRV, registerIndex);
    if (index == UINT32_MAX)
    { return; }

    if (type == SHADER_TYPE_CS)
    { pCmdList->SetComputeRootDescriptorTable(index, pView->GetHandleGPU()); }
    else
    { pCmdList->SetGraphicsRootDescriptorTable(index, pView->GetHandleGPU()); }
}

//-----------------------------------------------------------------------------
//      アンオーダードアクセスビューを設定します.
//-----------------------------------------------------------------------------
void PipelineState::SetUAV
(
    ID3D12GraphicsCommandList*  pCmdList,
    SHADER_TYPE                 type,
    uint32_t                    registerIndex,
    IUnorderedAccessView*       pView
)
{
    auto index = FindIndex(type, ROOT_PARAM_UAV, registerIndex);
    if (index == UINT32_MAX)
    { return; }

    if (type == SHADER_TYPE_CS)
    { pCmdList->SetComputeRootDescriptorTable(index, pView->GetHandleGPU()); }
    else
    { pCmdList->SetGraphicsRootDescriptorTable(index, pView->GetHandleGPU()); }
}

//-----------------------------------------------------------------------------
//      ルートパラメータ番号を検索します.
//-----------------------------------------------------------------------------
uint32_t PipelineState::FindIndex(SHADER_TYPE type, uint8_t kind, uint32_t registerIndex) const
{
    auto key = MakeKey(uint8_t(type), kind, registerIndex);

    auto itr = m_RootParameterIndices.find(key);
    if (itr == m_RootParameterIndices.end())
    { return UINT32_MAX; }

    return itr->second;
}

//-----------------------------------------------------------------------------
//      グラフィックスパイプライン用ルートシグニチャを生成します.
//-----------------------------------------------------------------------------
bool PipelineState::CreateGraphicsRootSignature(ID3D12Device8* pDevice, ID3D12RootSignature** ppRootSig)
{
    m_RootParameterIndices.clear();

    std::vector<D3D12_DESCRIPTOR_RANGE*>    ranges;
    std::vector<D3D12_ROOT_PARAMETER>       params;
    std::vector<D3D12_STATIC_SAMPLER_DESC>  samplers;

    bool hasVSInput = false;
    if (!m_VS.empty())
    { hasVSInput = EnumerateRootParameter(SHADER_TYPE_VS, m_VS.data(), m_VS.size(), ranges, params, samplers); }

    if (!m_PS.empty())
    { EnumerateRootParameter(SHADER_TYPE_PS, m_PS.data(), m_PS.size(), ranges, params, samplers); }

    D3D12_ROOT_SIGNATURE_FLAGS flags = {};
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;
    if (hasVSInput)
    { flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; }
    if (CheckSupportDynamicResources(pDevice))
    {
        flags |= D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
        flags |= D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;
    }

    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters      = UINT(params.size());
    desc.pParameters        = params.data();
    desc.NumStaticSamplers  = UINT(samplers.size());
    desc.pStaticSamplers    = samplers.data();
    desc.Flags              = flags;

    asdx::RefPtr<ID3DBlob> blob;
    asdx::RefPtr<ID3DBlob> errorBlob;

    auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, blob.GetAddress(), errorBlob.GetAddress());
    auto ret = false;

    if (SUCCEEDED(hr))
    {
        hr = pDevice->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(ppRootSig));
        if (SUCCEEDED(hr))
        { ret = true; }
    }

    for(size_t i=0; i<ranges.size(); ++i) {
        auto ptr = ranges[i];
        if (ptr)
        { delete ptr; }
        ranges[i] = nullptr;
    }
    ranges.clear();
    params.clear();

    return ret;
}

//-----------------------------------------------------------------------------
//      ジオメトリパイプライン用ルートシグニチャを生成します.
//-----------------------------------------------------------------------------
bool PipelineState::CreateGeometryRootSignature(ID3D12Device8* pDevice, ID3D12RootSignature** ppRootSig)
{
    m_RootParameterIndices.clear();

    std::vector<D3D12_DESCRIPTOR_RANGE*>    ranges;
    std::vector<D3D12_ROOT_PARAMETER>       params;
    std::vector<D3D12_STATIC_SAMPLER_DESC>  samplers;

    if (!m_AS.empty())
    { EnumerateRootParameter(SHADER_TYPE_AS, m_AS.data(), m_AS.size(), ranges, params, samplers); }

    if (!m_MS.empty())
    { EnumerateRootParameter(SHADER_TYPE_MS, m_MS.data(), m_MS.size(), ranges, params, samplers); }

    if (!m_PS.empty())
    { EnumerateRootParameter(SHADER_TYPE_PS, m_PS.data(), m_PS.size(), ranges, params, samplers); }

    D3D12_ROOT_SIGNATURE_FLAGS flags = {};
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
    if (CheckSupportDynamicResources(pDevice))
    {
        flags |= D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
        flags |= D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;
    }

    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters      = UINT(params.size());
    desc.pParameters        = params.data();
    desc.NumStaticSamplers  = UINT(samplers.size());
    desc.pStaticSamplers    = samplers.data();
    desc.Flags              = flags;

    asdx::RefPtr<ID3DBlob> blob;
    asdx::RefPtr<ID3DBlob> errorBlob;

    auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, blob.GetAddress(), errorBlob.GetAddress());
    auto ret = false;

    if (SUCCEEDED(hr))
    {
        hr = pDevice->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(ppRootSig));
        if (SUCCEEDED(hr))
        { ret = true; }
    }

    for(size_t i=0; i<ranges.size(); ++i) {
        auto ptr = ranges[i];
        if (ptr)
        { delete ptr; }
        ranges[i] = nullptr;
    }
    ranges.clear();
    params.clear();

    return ret;
}

//-----------------------------------------------------------------------------
//      コンピュートパイプライン用ルートシグニチャを生成します.
//-----------------------------------------------------------------------------
bool PipelineState::CreateComputeRootSignature(ID3D12Device8* pDevice, ID3D12RootSignature** ppRootSig)
{
    m_RootParameterIndices.clear();

    std::vector<D3D12_DESCRIPTOR_RANGE*>    ranges;
    std::vector<D3D12_ROOT_PARAMETER>       params;
    std::vector<D3D12_STATIC_SAMPLER_DESC>  samplers;

    if (!m_CS.empty())
    { EnumerateRootParameter(SHADER_TYPE_AS, m_CS.data(), m_CS.size(), ranges, params, samplers); }

    D3D12_ROOT_SIGNATURE_FLAGS flags = {};
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;
    if (CheckSupportDynamicResources(pDevice))
    {
        flags |= D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
        flags |= D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;
    }

    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters      = UINT(params.size());
    desc.pParameters        = params.data();
    desc.NumStaticSamplers  = UINT(samplers.size());
    desc.pStaticSamplers    = samplers.data();
    desc.Flags              = flags;

    asdx::RefPtr<ID3DBlob> blob;
    asdx::RefPtr<ID3DBlob> errorBlob;

    auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, blob.GetAddress(), errorBlob.GetAddress());
    auto ret = false;

    if (SUCCEEDED(hr))
    {
        hr = pDevice->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(ppRootSig));
        if (SUCCEEDED(hr))
        { ret = true; }
    }

    for(size_t i=0; i<ranges.size(); ++i) {
        auto ptr = ranges[i];
        if (ptr)
        { delete ptr; }
        ranges[i] = nullptr;
    }
    ranges.clear();
    params.clear();

    return ret;
}

//-----------------------------------------------------------------------------
//      ルートパラメータを列挙します.
//-----------------------------------------------------------------------------
bool PipelineState::EnumerateRootParameter
(
    SHADER_TYPE                             type,
    const void*                             binary,
    size_t                                  binarySize,
    std::vector<D3D12_DESCRIPTOR_RANGE*>&   ranges,
    std::vector<D3D12_ROOT_PARAMETER>&      params,
    std::vector<D3D12_STATIC_SAMPLER_DESC>& samplers
)
{
    bool hasVSInput = false;

    ShaderReflection reflection;

    if (!reflection.Init(binary, binarySize))
    { return hasVSInput; }

    D3D12_SHADER_DESC shaderDesc = {};
    auto hr = reflection->GetDesc(&shaderDesc);
    if (FAILED(hr))
    { return hasVSInput; }

    D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL;
    switch(type)
    {
    case SHADER_TYPE_VS:
        {
            visibility = D3D12_SHADER_VISIBILITY_VERTEX;

            for(auto i=0u; i<shaderDesc.InputParameters; ++i)
            {
                D3D12_SIGNATURE_PARAMETER_DESC sigDesc = {};
                hr = reflection->GetInputParameterDesc(i, &sigDesc);
                if (FAILED(hr))
                { continue; }

                if (sigDesc.SystemValueType == D3D_NAME_POSITION ||
                    sigDesc.SystemValueType == D3D_NAME_UNDEFINED)
                {
                    hasVSInput = true;
                    break;
                }
            }
        }
        break;

    case SHADER_TYPE_PS:
        visibility = D3D12_SHADER_VISIBILITY_PIXEL;
        break;

    case SHADER_TYPE_AS:
        visibility = D3D12_SHADER_VISIBILITY_AMPLIFICATION;
        break;

    case SHADER_TYPE_MS:
        visibility = D3D12_SHADER_VISIBILITY_MESH;
        break;
    }

    auto count = shaderDesc.BoundResources;
    for(auto i=0u; i<count; ++i)
    {
        D3D12_SHADER_INPUT_BIND_DESC inputDesc = {};
        hr = reflection->GetResourceBindingDesc(i, &inputDesc);
        if (FAILED(hr))
        { continue; }

        switch(inputDesc.Type)
        {
        // CBV.
        case D3D_SIT_CBUFFER:
            {
                if (strstr(inputDesc.Name, "Constants") != nullptr)
                {
                    auto cbReflection = reflection->GetConstantBufferByName(inputDesc.Name);
                    assert(cbReflection != nullptr);

                    D3D12_SHADER_BUFFER_DESC bufferDesc = {};
                    hr = cbReflection->GetDesc(&bufferDesc);
                    if (SUCCEEDED(hr))
                    {
                        auto size       = bufferDesc.Size;
                        auto varCount   = size / 4;

                        auto paramIndex = uint16_t(params.size());

                        D3D12_ROOT_PARAMETER param = {};
                        param.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
                        param.Constants.Num32BitValues  = varCount;
                        param.Constants.ShaderRegister  = inputDesc.BindPoint;
                        param.Constants.RegisterSpace   = inputDesc.Space;
                        param.ShaderVisibility          = visibility;
                        params.push_back(param);

                        auto key = MakeKey(type, ROOT_PARAM_VAR, inputDesc.BindPoint);
                        m_RootParameterIndices[key] = paramIndex;
                    }
                }
                else
                {
                    auto paramIndex = uint16_t(params.size());

                    D3D12_ROOT_PARAMETER param = {};
                    param.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
                    param.Descriptor.ShaderRegister = inputDesc.BindPoint;
                    param.Descriptor.RegisterSpace  = inputDesc.Space;
                    param.ShaderVisibility          = visibility;
                    params.push_back(param);

                    auto key = MakeKey(type, ROOT_PARAM_CBV, inputDesc.BindPoint);
                    m_RootParameterIndices[key] = paramIndex;
                }
            }
            break;

        // SRV.
        case D3D_SIT_TBUFFER:
        case D3D_SIT_TEXTURE:
        case D3D_SIT_STRUCTURED:
        case D3D_SIT_BYTEADDRESS:
        case D3D_SIT_RTACCELERATIONSTRUCTURE:
            {
                auto paramIndex = uint16_t(params.size());

                auto range = new D3D12_DESCRIPTOR_RANGE();
                range->BaseShaderRegister                   = inputDesc.BindPoint;
                range->RegisterSpace                        = inputDesc.Space;
                range->NumDescriptors                       = 1;
                range->OffsetInDescriptorsFromTableStart    = 0;
                range->RangeType                            = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

                D3D12_ROOT_PARAMETER param = {};
                param.ParameterType                         = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                param.DescriptorTable.NumDescriptorRanges   = 1;
                param.DescriptorTable.pDescriptorRanges     = range;
                param.ShaderVisibility                      = visibility;

                params.push_back(param);
                ranges.push_back(range);

                auto key = MakeKey(type, ROOT_PARAM_SRV, inputDesc.BindPoint);
                m_RootParameterIndices[key] = paramIndex;
            }
            break;

        // UAV.
        case D3D_SIT_UAV_RWTYPED:
        case D3D_SIT_UAV_RWSTRUCTURED:
        case D3D_SIT_UAV_RWBYTEADDRESS:
        case D3D_SIT_UAV_APPEND_STRUCTURED:
        case D3D_SIT_UAV_CONSUME_STRUCTURED:
        case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
        case D3D_SIT_UAV_FEEDBACKTEXTURE:
            {
                auto paramIndex = uint16_t(params.size());

                auto range = new D3D12_DESCRIPTOR_RANGE();
                range->BaseShaderRegister                   = inputDesc.BindCount;
                range->RegisterSpace                        = inputDesc.Space;
                range->NumDescriptors                       = 1;
                range->OffsetInDescriptorsFromTableStart    = 0;
                range->RangeType                            = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;

                D3D12_ROOT_PARAMETER param = {};
                param.ParameterType                         = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                param.DescriptorTable.NumDescriptorRanges   = 1;
                param.DescriptorTable.pDescriptorRanges     = range;
                param.ShaderVisibility                      = visibility;

                params.push_back(param);
                ranges.push_back(range);

                auto key = MakeKey(type, ROOT_PARAM_UAV, inputDesc.BindPoint);
                m_RootParameterIndices[key] = paramIndex;
            }
            break;

        case D3D_SIT_SAMPLER:
            {
                D3D12_STATIC_SAMPLER_DESC smpDesc = {};
                smpDesc.MipLODBias          = 0;
                smpDesc.MinLOD              = 0;
                smpDesc.MaxLOD              = D3D12_FLOAT32_MAX;
                smpDesc.ComparisonFunc      = D3D12_COMPARISON_FUNC_NEVER;
                smpDesc.BorderColor         = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
                smpDesc.ShaderRegister      = inputDesc.BindPoint;
                smpDesc.RegisterSpace       = inputDesc.Space;
                smpDesc.ShaderVisibility    = visibility;

                if (strcmp(inputDesc.Name, "PointClamp") == 0)
                {
                    smpDesc.AddressU    = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                    smpDesc.AddressV    = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                    smpDesc.AddressW    = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                    smpDesc.Filter      = D3D12_FILTER_MIN_MAG_MIP_POINT;

                    samplers.push_back(smpDesc);
                }
                else if (strcmp(inputDesc.Name, "PointWrap") == 0)
                {
                    smpDesc.AddressU    = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                    smpDesc.AddressV    = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                    smpDesc.AddressW    = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                    smpDesc.Filter      = D3D12_FILTER_MIN_MAG_MIP_POINT;

                    samplers.push_back(smpDesc);
                }
                else if (strcmp(inputDesc.Name, "PointMirror") == 0)
                {
                    smpDesc.AddressU    = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                    smpDesc.AddressV    = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                    smpDesc.AddressW    = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                    smpDesc.Filter      = D3D12_FILTER_MIN_MAG_MIP_POINT;

                    samplers.push_back(smpDesc);
                }
                else if (strcmp(inputDesc.Name, "LinearClamp") == 0)
                {
                    smpDesc.AddressU    = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                    smpDesc.AddressV    = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                    smpDesc.AddressW    = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                    smpDesc.Filter      = D3D12_FILTER_MIN_MAG_MIP_LINEAR;

                    samplers.push_back(smpDesc);
                }
                else if (strcmp(inputDesc.Name, "LinearWrap") == 0)
                {
                    smpDesc.AddressU    = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                    smpDesc.AddressV    = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                    smpDesc.AddressW    = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                    smpDesc.Filter      = D3D12_FILTER_MIN_MAG_MIP_LINEAR;

                    samplers.push_back(smpDesc);
                }
                else if (strcmp(inputDesc.Name, "LinearMirror") == 0)
                {
                    smpDesc.AddressU    = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                    smpDesc.AddressV    = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                    smpDesc.AddressW    = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                    smpDesc.Filter      = D3D12_FILTER_MIN_MAG_MIP_LINEAR;

                    samplers.push_back(smpDesc);
                }
                else if (strcmp(inputDesc.Name, "AnisotropicClamp") == 0)
                {
                    smpDesc.AddressU        = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                    smpDesc.AddressV        = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                    smpDesc.AddressW        = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                    smpDesc.Filter          = D3D12_FILTER_ANISOTROPIC;
                    smpDesc.MaxAnisotropy   = 16;

                    samplers.push_back(smpDesc);
                }
                else if (strcmp(inputDesc.Name, "AnisotropicWrap") == 0)
                {
                    smpDesc.AddressU        = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                    smpDesc.AddressV        = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                    smpDesc.AddressW        = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                    smpDesc.Filter          = D3D12_FILTER_ANISOTROPIC;
                    smpDesc.MaxAnisotropy   = 16;

                    samplers.push_back(smpDesc);
                }
                else if (strcmp(inputDesc.Name, "AnisotropicMirror") == 0)
                {
                    smpDesc.AddressU        = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                    smpDesc.AddressV        = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                    smpDesc.AddressW        = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                    smpDesc.Filter          = D3D12_FILTER_ANISOTROPIC;
                    smpDesc.MaxAnisotropy   = 16;

                    samplers.push_back(smpDesc);
                }
                else if (strcmp(inputDesc.Name, "LessEqualSampler") == 0)
                {
                    smpDesc.AddressU        = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
                    smpDesc.AddressV        = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
                    smpDesc.AddressW        = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
                    smpDesc.Filter          = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
                    smpDesc.ComparisonFunc  = D3D12_COMPARISON_FUNC_LESS_EQUAL;
                    smpDesc.BorderColor     = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;

                    samplers.push_back(smpDesc);
                }
                else if (strcmp(inputDesc.Name, "GreaterSampler") == 0)
                {
                    smpDesc.AddressU        = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
                    smpDesc.AddressV        = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
                    smpDesc.AddressW        = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
                    smpDesc.Filter          = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
                    smpDesc.ComparisonFunc  = D3D12_COMPARISON_FUNC_GREATER;
                    smpDesc.BorderColor     = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;

                    samplers.push_back(smpDesc);
                }
            }
            break;
        }
    }

    return hasVSInput;
}

void InitRangeAsSRV(D3D12_DESCRIPTOR_RANGE& range, UINT registerIndex, UINT count)
{
    range.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range.NumDescriptors                    = count;
    range.BaseShaderRegister                = registerIndex;
    range.RegisterSpace                     = 0;
    range.OffsetInDescriptorsFromTableStart = 0;
}

void InitRangeAsUAV(D3D12_DESCRIPTOR_RANGE& range, UINT registerIndex, UINT count)
{
    range.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    range.NumDescriptors                    = count;
    range.BaseShaderRegister                = registerIndex;
    range.RegisterSpace                     = 0;
    range.OffsetInDescriptorsFromTableStart = 0;
}

void InitAsConstants(D3D12_ROOT_PARAMETER& param, UINT registerIndex, UINT count, D3D12_SHADER_VISIBILITY visibility)
{
    param.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    param.Constants.Num32BitValues  = count;
    param.Constants.ShaderRegister  = registerIndex;
    param.Constants.RegisterSpace   = 0;
    param.ShaderVisibility          = visibility;
}

void InitAsCBV(D3D12_ROOT_PARAMETER& param, UINT registerIndex, D3D12_SHADER_VISIBILITY visibility)
{
    param.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    param.Descriptor.ShaderRegister = registerIndex;
    param.Descriptor.RegisterSpace  = 0;
    param.ShaderVisibility          = visibility;
}

void InitAsSRV(D3D12_ROOT_PARAMETER& param, UINT registerIndex, D3D12_SHADER_VISIBILITY visibility)
{
    param.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_SRV;
    param.Descriptor.ShaderRegister = registerIndex;
    param.Descriptor.RegisterSpace  = 0;
    param.ShaderVisibility          = visibility;
}

void InitAsTable(
    D3D12_ROOT_PARAMETER&           param,
    UINT                            count,
    const D3D12_DESCRIPTOR_RANGE*   range,
    D3D12_SHADER_VISIBILITY         visibility
)
{
    param.ParameterType                         = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param.DescriptorTable.NumDescriptorRanges   = 1;
    param.DescriptorTable.pDescriptorRanges     = range;
    param.ShaderVisibility                      = visibility;
}

bool InitRootSignature
(
    ID3D12Device*                       pDevice,
    const D3D12_ROOT_SIGNATURE_DESC*    pDesc,
    ID3D12RootSignature**               ppRootSig
)
{
    asdx::RefPtr<ID3DBlob> blob;
    asdx::RefPtr<ID3DBlob> errorBlob;
    auto hr = D3D12SerializeRootSignature(
        pDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, blob.GetAddress(), errorBlob.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : D3D12SerializeRootSignature() Failed. errcode = 0x%x", hr);
        if (errorBlob) {
            ELOG("Error : msg = %s", reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
        }
        return false;
    }

    hr = pDevice->CreateRootSignature(
        0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(ppRootSig));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateRootSignature() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

} // namespace asdx
