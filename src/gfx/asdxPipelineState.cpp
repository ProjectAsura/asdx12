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
#include <asdxGraphicsDevice.h>


namespace {

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

    m_VS.resize(pDesc->VS.BytecodeLength);
    memcpy(m_VS.data(), pDesc->VS.pShaderBytecode, m_VS.size());

    m_PS.resize(pDesc->PS.BytecodeLength);
    memcpy(m_PS.data(), pDesc->PS.pShaderBytecode, m_PS.size());

    m_Type = PIPELINE_TYPE_GRAPHICS;
    m_Desc.Graphics = *pDesc;
    m_Desc.Graphics.VS.pShaderBytecode = m_VS.data();
    m_Desc.Graphics.PS.pShaderBytecode = m_PS.data();

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

    m_CS.resize(pDesc->CS.BytecodeLength);
    memcpy(m_CS.data(), pDesc->CS.pShaderBytecode, m_CS.size());

    m_Type = PIPELINE_TYPE_COMPUTE;
    m_Desc.Compute = *pDesc;
    m_Desc.Compute.CS.pShaderBytecode = m_CS.data();

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
void PipelineState::ReplaceVS(const void* pBinary, size_t binarySize)
{
    if (m_Type != PIPELINE_TYPE_GRAPHICS)
    { return; }

    m_VS.resize(binarySize);
    memcpy(m_VS.data(), pBinary, binarySize);

    m_Desc.Graphics.VS.pShaderBytecode = m_VS.data();
    m_Desc.Graphics.VS.BytecodeLength  = m_VS.size();
}

//-----------------------------------------------------------------------------
//      ピクセルシェーダを差し替えます.
//-----------------------------------------------------------------------------
void PipelineState::ReplacePS(const void* pBinary, size_t binarySize)
{
    if (m_Type == PIPELINE_TYPE_COMPUTE)
    { return; }

    m_PS.resize(binarySize);
    memcpy(m_PS.data(), pBinary, binarySize);

    if (m_Type == PIPELINE_TYPE_GRAPHICS)
    {
        m_Desc.Graphics.PS.pShaderBytecode = m_PS.data();
        m_Desc.Graphics.PS.BytecodeLength  = m_PS.size();
    }
    else if (m_Type == PIPELINE_TYPE_GEOMETRY)
    {
        m_Desc.Geometry.PS.pShaderBytecode = m_PS.data();
        m_Desc.Geometry.PS.BytecodeLength  = m_PS.size();
    }
}

//-----------------------------------------------------------------------------
//      コンピュートシェーダを差し替えます.
//-----------------------------------------------------------------------------
void PipelineState::ReplaceCS(const void* pBinary, size_t binarySize)
{
    if (m_Type != PIPELINE_TYPE_COMPUTE)
    { return; }

    m_CS.resize(binarySize);
    memcpy(m_CS.data(), pBinary, binarySize);

    m_Desc.Compute.CS.pShaderBytecode = m_CS.data();
    m_Desc.Compute.CS.BytecodeLength  = m_CS.size();
}

//-----------------------------------------------------------------------------
//      メッシュシェーダを差し替えます.
//-----------------------------------------------------------------------------
void PipelineState::ReplaceMS(const void* pBinary, size_t binarySize)
{
    if (m_Type != PIPELINE_TYPE_GEOMETRY)
    { return; }

    m_MS.resize(binarySize);
    memcpy(m_MS.data(), pBinary, binarySize);

    m_Desc.Geometry.MS.pShaderBytecode = m_MS.data();
    m_Desc.Geometry.MS.BytecodeLength  = m_MS.size();
}

//-----------------------------------------------------------------------------
//      増幅シェーダを差し替えます.
//-----------------------------------------------------------------------------
void PipelineState::ReplaceAS(const void* pBinary, size_t binarySize)
{
    if (m_Type != PIPELINE_TYPE_GEOMETRY)
    { return; }

    m_AS.resize(binarySize);
    memcpy(m_AS.data(), pBinary, binarySize);

    m_Desc.Geometry.AS.pShaderBytecode = m_AS.data();
    m_Desc.Geometry.AS.BytecodeLength  = m_AS.size();
}

//-----------------------------------------------------------------------------
//      パイプラインステートを再生成します.
//-----------------------------------------------------------------------------
void PipelineState::Recreate()
{
    if (!m_pRecreatePSO.GetPtr())
    {
        auto pso = m_pRecreatePSO.Detach();
        GfxDevice().PushToDisposer(pso);
    }

    if (m_Type == PIPELINE_TYPE_GRAPHICS)
    {
        auto hr = GetD3D12Device()->CreateGraphicsPipelineState(&m_Desc.Graphics, IID_PPV_ARGS(m_pRecreatePSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
            return;
        }

         m_pRecreatePSO->SetName(L"asdxGraphicsPipelineState_Reload");
    }
    else if (m_Type == PIPELINE_TYPE_COMPUTE)
    {
        auto hr = GetD3D12Device()->CreateComputePipelineState(&m_Desc.Compute, IID_PPV_ARGS(m_pRecreatePSO.GetAddress()));
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
//      パイプラインステートを取得します.
//-----------------------------------------------------------------------------
ID3D12PipelineState* PipelineState::GetPtr() const
{
    if (m_pRecreatePSO.GetPtr() != nullptr)
    { return m_pRecreatePSO.GetPtr(); }

    return m_pPSO.GetPtr();
}

//-----------------------------------------------------------------------------
//      パイプラインタイプを取得します.
//-----------------------------------------------------------------------------
PIPELINE_TYPE PipelineState::GetType() const
{ return m_Type; }

//-----------------------------------------------------------------------------
//      深度ステンシルステートを取得します.
//-----------------------------------------------------------------------------
D3D12_DEPTH_STENCIL_DESC PipelineState::GetDepthStencilState(DEPTH_STATE_TYPE type, D3D12_COMPARISON_FUNC func)
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
D3D12_RASTERIZER_DESC PipelineState::GetRasterizerState(RASTERIZER_STATE_TYPE type)
{
    D3D12_RASTERIZER_DESC result = {};

    result.FrontCounterClockwise    = FALSE;
    result.DepthBias                = D3D12_DEFAULT_DEPTH_BIAS;
    result.DepthBiasClamp           = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    result.SlopeScaledDepthBias     = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
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
D3D12_BLEND_DESC PipelineState::GetBlendState(BLEND_STATE_TYPE type)
{
    D3D12_BLEND_DESC result = {};

    result.AlphaToCoverageEnable    = FALSE;
    result.IndependentBlendEnable   = FALSE;
    result.RenderTarget[0].RenderTargetWriteMask    = D3D12_COLOR_WRITE_ENABLE_ALL;
    result.RenderTarget[0].LogicOpEnable            = FALSE;
    result.RenderTarget[0].LogicOp                  = D3D12_LOGIC_OP_NOOP;

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
