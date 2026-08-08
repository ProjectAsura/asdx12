//-----------------------------------------------------------------------------
// File : asdxIBLBaker.cpp
// Desc : IBL Baker.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <gfx/asdxIBLBaker.h>
#include <gfx/asdxPipelineState.h>
#include <gfx/asdxPresetState.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxScopedMarker.h>


namespace {

///////////////////////////////////////////////////////////////////////////////
// ROOT_PARAM enum
///////////////////////////////////////////////////////////////////////////////
enum ROOT_PARAM
{
    ROOT_PARAM_B0,
    ROOT_PARAM_B1,
    ROOT_PARAM_T0,
    ROOT_PARAM_U0,
    MAX_ROOT_PARAM_COUNT,
};

///////////////////////////////////////////////////////////////////////////////
// BakeParam structure
///////////////////////////////////////////////////////////////////////////////
struct BakeParam
{
    uint32_t    Width;          //!< レンダーターゲットの横幅.
    uint32_t    Height;         //!< レンダーターゲットの縦幅.
    float       Roughness;      //!< ラフネス値.
    float       MipCount;       //!< ミップマップ数.
};

///////////////////////////////////////////////////////////////////////////////
// EnvMapParam structure
///////////////////////////////////////////////////////////////////////////////
struct EnvMapParam
{
    uint32_t    WidthMip0;
    uint32_t    HeightMip0;
    uint32_t    WidthCurMip;
    uint32_t    HeightCurMip;
};

//-----------------------------------------------------------------------------
// Shaders
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/asdxIBLBakeDiffuseCS.inc"
#include "../res/shaders/Compiled/asdxIBLBakeSpecularCS.inc"

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// IBLBake class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
IBLBaker::IBLBaker()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
IBLBaker::~IBLBaker()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool IBLBaker::Init()
{
    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    // ルートシグニチャを生成.
    {
        D3D12_DESCRIPTOR_RANGE ranges[2] = {};
        asdx::InitRangeAsSRV(ranges[0], 0);
        asdx::InitRangeAsUAV(ranges[1], 0);

        D3D12_ROOT_PARAMETER  params[MAX_ROOT_PARAM_COUNT] = {};
        asdx::InitAsConstants(params[ROOT_PARAM_B0], 0, 4, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsConstants(params[ROOT_PARAM_B1], 1, 4, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsTable    (params[ROOT_PARAM_T0], 1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsTable    (params[ROOT_PARAM_U0], 1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters      = _countof(params);
        desc.pParameters        = params;
        desc.NumStaticSamplers  = _countof(asdx::Preset::StaticSamplers);
        desc.pStaticSamplers    = asdx::Preset::StaticSamplers;

        if (!asdx::InitRootSignature(pDevice, &desc, m_RootSignature.GetAddress()))
        {
            ELOGA("Error : InitRootSignature() Failed.");
            return false;
        }
    }

    // Diffuse用パイプラインステートを生成.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.CS = { asdxIBLBakeDiffuseCS, sizeof(asdxIBLBakeDiffuseCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_DiffuseState.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // Specular用パイプラインステートを生成.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.CS = { asdxIBLBakeSpecularCS, sizeof(asdxIBLBakeSpecularCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_SpecularState.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void IBLBaker::Term()
{
    m_DiffuseState .Reset();
    m_SpecularState.Reset();
    m_RootSignature.Reset();
}

//-----------------------------------------------------------------------------
//      ベイク処理を行います.
//-----------------------------------------------------------------------------
void IBLBaker::Bake
(
    ID3D12GraphicsCommandList*  pCmd,
    ComputeTarget&              diffuseLD,
    ComputeTarget&              specularLD,
    uint32_t                    width,
    uint32_t                    height,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSRV
)
{
    if (pCmd == nullptr)
        return;

    ASDX_SCOPED_MARKER(pCmd, IBLBake);

    BakeDiffuse (pCmd, diffuseLD,  width, height, handleSRV);
    BakeSpecular(pCmd, specularLD, width, height, handleSRV);
}

//-----------------------------------------------------------------------------
//      DiffuseLD項をベイクします.
//-----------------------------------------------------------------------------
void IBLBaker::BakeDiffuse
(
    ID3D12GraphicsCommandList*  pCmd,
    ComputeTarget&              diffuseLD,
    uint32_t                    width,
    uint32_t                    height,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSRV
)
{
    ASDX_SCOPED_MARKER(pCmd, BakeDiffuse);
    auto desc = diffuseLD.GetDesc();

    BakeParam param = {};
    param.Width     = uint32_t(desc.Width);
    param.Height    = uint32_t(desc.Height);
    param.MipCount  = float(desc.MipLevels);
    param.Roughness = 0.0f;

    pCmd->SetComputeRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_DiffuseState.GetPtr());
    pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_T0, handleSRV);

    auto w = param.Width;
    auto h = param.Height;

    auto mapW = width;
    auto mapH = height;

    EnvMapParam mapParam = {};
    mapParam.WidthMip0  = width;
    mapParam.HeightMip0 = height;

    for(auto m=0; m<desc.MipLevels; ++m)
    {
        param.Width  = w;
        param.Height = h;

        mapParam.WidthCurMip  = mapW;
        mapParam.HeightCurMip = mapH;

        pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_B0, 4, &param, 0);
        pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_B1, 4, &mapParam, 0);
        pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_U0, diffuseLD.GetGpuHandleUAV(m));

        auto x = (w + 7u) / 8;
        auto y = (h + 7u) / 8;
        pCmd->Dispatch(x, y, 1);

        w >>= 1;
        h >>= 1;
        if (w < 1) { w = 1; }
        if (h < 1) { h = 1; }

        mapW >>= 1;
        mapH >>= 1;
        if (mapW < 1) { mapW = 1; }
        if (mapH < 1) { mapH = 1; }
    }
}

//-----------------------------------------------------------------------------
//      SpecularLD項をベイクします.
//-----------------------------------------------------------------------------
void IBLBaker::BakeSpecular
(
    ID3D12GraphicsCommandList*  pCmd,
    ComputeTarget&              specularLD,
    uint32_t                    width,
    uint32_t                    height,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSRV
)
{
    ASDX_SCOPED_MARKER(pCmd, BakeSpecular);
    auto desc = specularLD.GetDesc();
    const auto linearRoughnessStep = 1.0f / float(desc.MipLevels - 1.0f);
    auto linearRoughness = 0.0f;

    BakeParam param = {};
    param.Width     = uint32_t(desc.Width);
    param.Height    = desc.Height;
    param.MipCount  = float(desc.MipLevels);
    param.Roughness = linearRoughness;

    pCmd->SetComputeRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_SpecularState.GetPtr());
    pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_T0, handleSRV);

    auto w = param.Width;
    auto h = param.Height;

    auto mapW = width;
    auto mapH = height;

    EnvMapParam mapParam = {};
    mapParam.WidthMip0  = width;
    mapParam.HeightMip0 = height;

    for(auto m=0; m<desc.MipLevels; ++m)
    {
        param.Width     = w;
        param.Height    = h;
        param.Roughness = linearRoughness;

        mapParam.WidthCurMip  = mapW;
        mapParam.HeightCurMip = mapH;

        pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_B0, 4, &param, 0);
        pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_B1, 4, &mapParam, 0);
        pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_U0, specularLD.GetGpuHandleUAV(m));

        auto x = (w + 7u) / 8u;
        auto y = (h + 7u) / 8u;
        pCmd->Dispatch(x, y, 1);

        linearRoughness += linearRoughnessStep;

        w >>= 1;
        h >>= 1;
        if (w < 1) { w = 1; }
        if (h < 1) { h = 1; }

        mapW >>= 1;
        mapH >>= 1;
        if (mapW < 1) { mapW = 1; }
        if (mapH < 1) { mapH = 1; }
    }
}

} // namespace asdx
