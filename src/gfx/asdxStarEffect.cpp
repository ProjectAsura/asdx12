//-----------------------------------------------------------------------------
// File : asdxStarEffect.cpp
// Desc : Star (Light Streak) Effect.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <array>
#include <fnd/asdxMath.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxArrayView.h>
#include <gfx/asdxStarEffect.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxPresetState.h>
#include <gfx/asdxCommandList.h>


namespace {

//-----------------------------------------------------------------------------
// Shaders
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/asdxStarCS.inc"
#include "../res/shaders/Compiled/asdxStarCompositeCS.inc"


///////////////////////////////////////////////////////////////////////////////
// ROOT_PARAM enum
///////////////////////////////////////////////////////////////////////////////
enum ROOT_PARAM
{
    ROOT_PARAM_CBV0,
    ROOT_PARAM_SRV0,
    ROOT_PARAM_UAV0,
};

///////////////////////////////////////////////////////////////////////////////
// STAR_DEF_TYPE enum
///////////////////////////////////////////////////////////////////////////////
enum STAR_DEF_TYPE
{
    STAR_DEF_DISABLE,
    STAR_DEF_CROSS,
    STAR_DEF_CROSS_FILTER,
    STAR_DEF_SNOW_CROSS,
    STAR_DEF_SUNNY_CROSS,
    STAR_DEF_HORIZONTAL,
};

///////////////////////////////////////////////////////////////////////////////
// StartDef structure
///////////////////////////////////////////////////////////////////////////////
struct StarDef
{
    const char* Name;
    int         StarLineCount;
    int         PassCount;
    float       SampleLength;
    float       Attenuation;
    float       Rotation;
    bool        IsRotate;
};

///////////////////////////////////////////////////////////////////////////////
// GlareDef structure
///////////////////////////////////////////////////////////////////////////////
struct GlareDef
{
    const char*     Name;
    float           StarLuminance;
    STAR_DEF_TYPE   StarType;
    float           StartInclination;
    float           ChromaticAberration;
};

///////////////////////////////////////////////////////////////////////////////
// StarLine structure
///////////////////////////////////////////////////////////////////////////////
struct StarLine
{
    int     PassCount;      //!< パス数.
    float   SampleLength;   //!< サンプル長.
    float   Attenuation;    //!< 減衰率.
    float   Inclination;    //!< 傾斜角.
};

///////////////////////////////////////////////////////////////////////////////
// ShaderParam structure
///////////////////////////////////////////////////////////////////////////////
struct ShaderParam
{
    asdx::Vector2 Offsets[8];
    asdx::Vector4 Weight[8];
    uint16_t      SizeX;
    uint16_t      SizeY;
};

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const StarDef kStarDef[] = {
    { "Disable",     0, 0, 0.0f, 0.00f, asdx::ToRadian(0.0f) , false },
    { "Cross",       4, 3, 1.0f, 0.85f, asdx::ToRadian(0.0f) , true  },
    { "CrossFilter", 4, 3, 1.0f, 0.95f, asdx::ToRadian(0.0f) , true  },
    { "SnowCross",   6, 3, 1.0f, 0.96f, asdx::ToRadian(20.0f), true  },
    { "SunnyCross",  8, 3, 1.0f, 0.88f, asdx::ToRadian(0.0f) , false }, 
    { "Horizonal",   2, 3, 1.0f, 0.96f, asdx::ToRadian(0.0f) , false },
};

static const GlareDef kGlareDef[] = {
    { "Disable"                 , 0.0f, STAR_DEF_DISABLE        , asdx::ToRadian(0.0f)  , 0.5f },
    //{ "Standard Camera"         , 1.0f, STAR_DEF_CROSS          , asdx::ToRadian(0.0f)  , 0.5f },
    //{ "Cheap Camera"            , 2.0f, STAR_DEF_CROSS          , asdx::ToRadian(0.0f)  , 0.5f },
    { "Cross Screen"            , 1.5f, STAR_DEF_CROSS_FILTER   , asdx::ToRadian(25.0f) , 0.5f },
    { "Spectral Cross Screen"   , 1.8f, STAR_DEF_CROSS_FILTER   , asdx::ToRadian(70.0f) , 1.5f },
    { "Snow Cross"              , 1.5f, STAR_DEF_SNOW_CROSS     , asdx::ToRadian(10.0f) , 0.5f },
    { "Spectral Snow Cross"     , 1.8f, STAR_DEF_SNOW_CROSS     , asdx::ToRadian(40.0f) , 1.5f },
    { "Sunny Cross"             , 1.5f, STAR_DEF_SUNNY_CROSS    , asdx::ToRadian(0.0f)  , 0.5f },
    { "Spectral Sunny Cross"    , 1.8f, STAR_DEF_SUNNY_CROSS    , asdx::ToRadian(45.0f) , 1.5f },
    { "Cinema Vertical"         , 1.0f, STAR_DEF_HORIZONTAL     , asdx::ToRadian(90.0f) , 0.5f },
    { "Cinema Horizontal"       , 1.0f, STAR_DEF_HORIZONTAL     , asdx::ToRadian(0.0f)  , 0.5f },
};

static const float kSunnyCrossLongAttenuation = 0.95f; // SunnyCrossで，2の倍数の時に適用する減衰率.

static const asdx::Vector4 kAberrationTable[8] = {
    asdx::Vector4(0.5f, 0.5f, 0.5f, 0.0f),  // w [0]
    asdx::Vector4(0.8f, 0.3f, 0.3f, 0.0f),  //   [1]
    asdx::Vector4(1.0f, 0.2f, 0.2f, 0.0f),  // r [2]
    asdx::Vector4(0.5f, 0.2f, 0.6f, 0.0f),  //   [3]
    asdx::Vector4(0.2f, 0.2f, 1.0f, 0.0f),  // b [4]
    asdx::Vector4(0.2f, 0.3f, 0.7f, 0.0f),  //   [5]
    asdx::Vector4(0.2f, 0.6f, 0.2f, 0.0f),  // g [6]
    asdx::Vector4(0.3f, 0.5f, 0.3f, 0.0f),  //   [7]
};

//-----------------------------------------------------------------------------
// Global Variables.
//-----------------------------------------------------------------------------
static std::array<StarLine, 4>  g_Cross;
static std::array<StarLine, 4>  g_CrossFilter;
static std::array<StarLine, 6>  g_SnowCross;
static std::array<StarLine, 8>  g_SunnyCross;
static std::array<StarLine, 2>  g_Horizontal;
static bool                     g_Init = false;

//-----------------------------------------------------------------------------
//      光芒定義から光芒ラインデータを初期化します.
//-----------------------------------------------------------------------------
void InitFromStarDef(const StarDef& def, StarLine* pLines, size_t count)
{
    auto rotate = asdx::ToRadian(360.0f / count);
    for(auto i=0; i<count; ++i)
    {
        pLines[i].PassCount      = def.PassCount;
        pLines[i].SampleLength   = def.SampleLength;
        pLines[i].Attenuation    = def.Attenuation;
        pLines[i].Inclination    = rotate * float(i);
    }
}

//-----------------------------------------------------------------------------
//      光芒パラメータを初期化します.
//-----------------------------------------------------------------------------
void InitStarLine()
{
    if (g_Init)
        return;

    InitFromStarDef(kStarDef[1], g_Cross      .data(), g_Cross      .size());
    InitFromStarDef(kStarDef[2], g_CrossFilter.data(), g_CrossFilter.size());
    InitFromStarDef(kStarDef[3], g_SnowCross  .data(), g_SnowCross  .size());
    InitFromStarDef(kStarDef[4], g_SunnyCross .data(), g_SunnyCross .size());
    InitFromStarDef(kStarDef[5], g_Horizontal .data(), g_Horizontal .size());

    for(size_t i=0; i<g_SunnyCross.size(); i++)
    {
        g_SunnyCross[i].PassCount = 3;
        if ((i % 2) == 0)
            g_SunnyCross[i].Attenuation = kSunnyCrossLongAttenuation;
    }

    g_Init = true;
}

//-----------------------------------------------------------------------------
//      光芒ラインデータを取得します.
//-----------------------------------------------------------------------------
asdx::ArrayView<StarLine> GetStarLines(STAR_DEF_TYPE type)
{
    switch(type)
    {
    case STAR_DEF_DISABLE:
    default:
        return asdx::ArrayView<StarLine>();

    case STAR_DEF_CROSS:
        return asdx::ArrayView<StarLine>(g_Cross.data(), g_Cross.size());

    case STAR_DEF_CROSS_FILTER:
        return asdx::ArrayView<StarLine>(g_CrossFilter.data(), g_CrossFilter.size());

    case STAR_DEF_SNOW_CROSS:
        return asdx::ArrayView<StarLine>(g_SnowCross.data(), g_SnowCross.size());

    case STAR_DEF_SUNNY_CROSS:
        return asdx::ArrayView<StarLine>(g_SunnyCross.data(), g_SunnyCross.size());

    case STAR_DEF_HORIZONTAL:
        return asdx::ArrayView<StarLine>(g_Horizontal.data(), g_Horizontal.size());
    }
}

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// StarEffect class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
StarEffect::StarEffect()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
StarEffect::~StarEffect()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool StarEffect::Init(uint32_t w, uint32_t h, DXGI_FORMAT format)
{
    auto pDevice = GetD3D12Device();

    // ルートシグニチャ生成.
    {
        D3D12_DESCRIPTOR_RANGE range[2] = {};
        range[0].RangeType                          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range[0].NumDescriptors                     = 1;
        range[0].BaseShaderRegister                 = 0;
        range[0].RegisterSpace                      = 0;
        range[0].OffsetInDescriptorsFromTableStart  = 0;

        range[1].RangeType                          = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        range[1].NumDescriptors                     = 1;
        range[1].BaseShaderRegister                 = 0;
        range[1].RegisterSpace                      = 0;
        range[1].OffsetInDescriptorsFromTableStart  = 0;

        D3D12_ROOT_PARAMETER param[3] = {};
        param[ROOT_PARAM_CBV0].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[ROOT_PARAM_CBV0].Constants.Num32BitValues   = 49;
        param[ROOT_PARAM_CBV0].Constants.ShaderRegister   = 0;
        param[ROOT_PARAM_CBV0].Constants.RegisterSpace    = 0;
        param[ROOT_PARAM_CBV0].ShaderVisibility           = D3D12_SHADER_VISIBILITY_ALL;

        param[ROOT_PARAM_SRV0].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[ROOT_PARAM_SRV0].DescriptorTable.NumDescriptorRanges    = 1;
        param[ROOT_PARAM_SRV0].DescriptorTable.pDescriptorRanges      = &range[0];
        param[ROOT_PARAM_SRV0].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_ALL;

        param[ROOT_PARAM_UAV0].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[ROOT_PARAM_UAV0].DescriptorTable.NumDescriptorRanges    = 1;
        param[ROOT_PARAM_UAV0].DescriptorTable.pDescriptorRanges      = &range[1];
        param[ROOT_PARAM_UAV0].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters      = _countof(param);
        desc.pParameters        = param;
        desc.NumStaticSamplers  = _countof(Preset::StaticSamplers);
        desc.pStaticSamplers    = Preset::StaticSamplers;
        desc.Flags              = D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
        desc.Flags             |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
        desc.Flags             |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
        desc.Flags             |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
        desc.Flags             |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

        RefPtr<ID3DBlob> blob;
        RefPtr<ID3DBlob> errorBlob;
        auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, blob.GetAddress(), errorBlob.GetAddress());
        if (FAILED(hr))
        {
            ELOGA("Error : D3D12SerializeRootSignature() Failed. errcode = 0x%x", hr);
            if (!errorBlob.GetPtr())
                ELOGA("Error : Msg = %s", reinterpret_cast<const char*>(errorBlob->GetBufferPointer())); 
            return false;
        }

        hr = pDevice->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateRootSignature() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // スター用コンピュートパイプライン初期化.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.CS             = { asdxStarCS, sizeof(asdxStarCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_StarPSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // 合成用パイプラインステート初期化.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.CS = { asdxStarCompositeCS, sizeof(asdxStarCompositeCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_CompositePSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // コンピュートターゲット生成.
    {
        TargetDesc desc = {};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width              = w;
        desc.Height             = h;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = format;
        desc.SampleDesc         = { 1, 0 };
        desc.InitState          = D3D12_RESOURCE_STATE_COMMON;
        desc.ClearColor[0]      = 0.0f;
        desc.ClearColor[1]      = 0.0f;
        desc.ClearColor[2]      = 0.0f;
        desc.ClearColor[3]      = 0.0f;

        for(auto i=0; i<2; ++i)
        {
            if (!m_PingPongTarget[i].Init(&desc))
            {
                ELOGA("Error : ComputeTarget::Init() Failed.");
                return false;
            }

            m_PingPongStates[i] = desc.InitState;
        }

        if (m_OutputTarget.Init(&desc))
        {
            ELOGA("Error : ComputeTarget::Init() Failed.");
            return false;
        }

        m_OutputStates = desc.InitState;
    }

    // パラメータ初期化.
    InitStarLine();

    return false;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void StarEffect::Term()
{
    m_StarPSO      .Reset();
    m_CompositePSO .Reset();
    m_RootSignature.Reset();

    for(auto i=0; i<2; ++i)
        m_PingPongTarget[i].Term();

    m_OutputTarget.Term();
}

//-----------------------------------------------------------------------------
//      エフェクトを適用します.
//-----------------------------------------------------------------------------
void StarEffect::Dispatch
(
    ID3D12GraphicsCommandList*  pCmd, 
    STAR_TYPE                   type,
    uint32_t                    width,
    uint32_t                    height,
    D3D12_GPU_DESCRIPTOR_HANDLE inputHandleSRV
)
{
    if (pCmd == nullptr)
        return;

    const auto tanFov       = atanf(asdx::F_PI / 8.0f);
    const auto kMaxPasses   = 3;
    const auto kSampleCount = 8;

    Vector4 colors[kMaxPasses][kSampleCount];
    const Vector4 kWhiteColor(0.63f, 0.63f, 0.63f, 0.0f);

    const GlareDef& glareDef = kGlareDef[type];
    const StarDef&  starDef  = kStarDef[glareDef.StarType];

    for(auto i=0; i<kMaxPasses; ++i)
    {
        float ratio = float(i + 1) / float(kMaxPasses);

        for(auto j=0; j<kSampleCount; ++j)
        {
            auto aberrColor = asdx::Vector4::Lerp(kAberrationTable[j], kWhiteColor, ratio);
            colors[i][j] = asdx::Vector4::Lerp(kWhiteColor, aberrColor, glareDef.ChromaticAberration);
        }
    }

    auto srcW = float(width);
    auto srcH = float(height);

    auto outDesc = m_OutputTarget.GetDesc();

    auto dstW = uint16_t(outDesc.Width);
    auto dstH = uint16_t(outDesc.Height);

    auto radOffset = glareDef.StartInclination + starDef.Rotation;

    ShaderParam param = {};
    param.SizeX = dstW;
    param.SizeY = dstH;

    auto threadX = (dstW + 7u) / 8u;
    auto threadY = (dstH + 7u) / 8u;

    auto starLines = GetStarLines(glareDef.StarType);

    auto srcIdx = 0u;
    auto dstIdx = 1u;

    D3D12_GPU_DESCRIPTOR_HANDLE handleUAV = m_PingPongTarget[0].GetGpuHandleSRV();

    pCmd->SetComputeRootSignature(m_RootSignature.GetPtr());

    // 方向ループ.
    for(auto d=0; d<starDef.StarLineCount; ++d)
    {
        auto& starLine = starLines[d];

        auto rad = radOffset + starLine.Inclination;
        auto s = sinf(rad);
        auto c = cosf(rad);

        Vector2 stepUV;
        stepUV.x = s / srcW * starLine.SampleLength;
        stepUV.y = c / srcH * starLine.SampleLength;

        // 減衰スケール.
        float attnPowScale = (tanFov + 0.1f) * (160.0f + 120.0f) / (srcW + srcH) * 1.2f;

        D3D12_GPU_DESCRIPTOR_HANDLE handleSRV = inputHandleSRV;

        pCmd->SetPipelineState(m_StarPSO.GetPtr());

        for(auto p=0; p<starLine.PassCount; ++p)
        {
            for(auto i=0; i<kSampleCount; ++i)
            {
                auto lum    = powf(starLine.Attenuation, attnPowScale * i);
                auto weight = colors[starLine.PassCount - 1 - p][i] * lum * (p + 1.0f) * 0.5f;
                auto offset = stepUV * float(i);

                if (fabs(offset.x) >= 0.9f || fabs(offset.y) >= 0.9f)
                {
                    offset = Vector2(0.0f, 0.0f);
                    weight = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
                }

                param.Weight [i]   = weight;
                param.Offsets[i].x = offset.x;
                param.Offsets[i].y = offset.y;
            }

            D3D12_RESOURCE_BARRIER barrier[2] = {};
            SetTransitionBarrier(barrier[0], m_PingPongTarget[dstIdx].GetResource(), m_PingPongStates[dstIdx], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            m_PingPongStates[dstIdx] = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

            if (p == 0)
            {
                pCmd->ResourceBarrier(1, barrier);
            }
            else
            {
                SetTransitionBarrier(barrier[1], m_PingPongTarget[srcIdx].GetResource(), m_PingPongStates[srcIdx], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                m_PingPongStates[srcIdx] = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
                pCmd->ResourceBarrier(2, barrier);
            }

            // 描画.
            pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_CBV0, 49, &param, 0);
            pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_SRV0, handleSRV);
            pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_UAV0, handleUAV);
            pCmd->Dispatch(threadX, threadY, 1);

            SetUAVBarrier(barrier[0], m_PingPongTarget[dstIdx].GetResource());
            pCmd->ResourceBarrier(1, barrier);

            stepUV       *= kSampleCount;
            attnPowScale *= kSampleCount;

            srcIdx = dstIdx;
            dstIdx = (dstIdx + 1) & 0x1;

            handleSRV = m_PingPongTarget[srcIdx].GetGpuHandleSRV();
            handleUAV = m_PingPongTarget[dstIdx].GetGpuHandleUAV();
        }

        // 出力用ターゲットに合成.
        {
            D3D12_RESOURCE_BARRIER barrier[2];
            SetTransitionBarrier(barrier[0], m_PingPongTarget[srcIdx].GetResource(), m_PingPongStates[srcIdx], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            SetTransitionBarrier(barrier[1], m_OutputTarget.GetResource(), m_OutputStates, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            m_PingPongStates[srcIdx] = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            m_OutputStates = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            pCmd->ResourceBarrier(2, barrier);

            pCmd->SetPipelineState(m_CompositePSO.GetPtr());
            pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_SRV0, handleSRV);
            pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_UAV0, m_OutputTarget.GetGpuHandleUAV());
            pCmd->Dispatch(threadX, threadY, 1);

            UAVBarrier(pCmd, m_OutputTarget.GetResource());
        }
    }

    TransitionBarrier(pCmd, m_OutputTarget.GetResource(), m_OutputStates, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
    m_OutputStates = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
}

//-----------------------------------------------------------------------------
//      SRVハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE StarEffect::GetHandleSRV() const
{ return m_OutputTarget.GetGpuHandleSRV(); }

} // namespace asdx
