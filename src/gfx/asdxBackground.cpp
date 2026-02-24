//-----------------------------------------------------------------------------
// File : asdxBackground.cpp
// Desc : Background Renderer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <gfx/asdxBackground.h>
#include <gfx/asdxPresetState.h>
#include <gfx/asdxPipelineState.h>
#include <gfx/asdxDevice.h>


namespace {

///////////////////////////////////////////////////////////////////////////////
// ROOT_PARAM enum
///////////////////////////////////////////////////////////////////////////////
enum ROOT_PARAM
{
    ROOT_PARAM_B0,
    ROOT_PARAM_T0,
    ROOT_PARAM_U0,
    MAX_ROOT_PARAM_COUNT,
};

///////////////////////////////////////////////////////////////////////////////
// Param structure
///////////////////////////////////////////////////////////////////////////////
struct Param
{
    asdx::Matrix    View;           //!< ビュー行列.
    asdx::Matrix    Proj;           //!< 射影行列.
    uint32_t        Width;          //!< 横幅.
    uint32_t        Height;         //!< 縦幅.
    float           InvWidth;       //!< 横幅の逆数.
    float           InvHeight;      //!< 縦幅の逆数.
};

//-----------------------------------------------------------------------------
// Shaders.
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/asdxBackgroundCubeCS.inc"
#include "../res/shaders/Compiled/asdxBackgroundSphereCS.inc"

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// BackgroundCube class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
BackgroundCube::BackgroundCube()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
BackgroundCube::~BackgroundCube()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool BackgroundCube::Init()
{
    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    // ルートシグニチャ生成.
    {
        D3D12_DESCRIPTOR_RANGE ranges[2] = {};
        InitRangeAsSRV(ranges[0], 0);
        InitRangeAsUAV(ranges[1], 0);

        D3D12_ROOT_PARAMETER params[MAX_ROOT_PARAM_COUNT] = {};
        InitAsCBV(params[ROOT_PARAM_B0], 0, D3D12_SHADER_VISIBILITY_ALL);
        InitAsTable(params[ROOT_PARAM_T0], 1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
        InitAsTable(params[ROOT_PARAM_U0], 1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters      = _countof(params);
        desc.pParameters        = params;
        desc.NumStaticSamplers  = _countof(Preset::StaticSamplers);
        desc.pStaticSamplers    = Preset::StaticSamplers;
        desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;

        if (!InitRootSignature(pDevice, &desc, m_RootSignature.GetAddress()))
        {
            ELOG("Error : InitRootSingature() Failed.");
            return false;
        }
    }

    // パイプラインステート生成.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.CS = { asdxBackgroundCubeCS, sizeof(asdxBackgroundCubeCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_PipelineState.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // 定数バッファ生成.
    {
        auto size = RoundUp<uint64_t>(sizeof(Param), 256);
        if (!m_ConstantBuffer.Init(size))
        {
            ELOG("Error : ConstantBuffer::Init() Failed.");
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void BackgroundCube::Term()
{
    m_ConstantBuffer.Term();
    m_PipelineState.Reset();
    m_RootSignature.Reset();
}

//-----------------------------------------------------------------------------
//      キューブマップを描画します.
//-----------------------------------------------------------------------------
void BackgroundCube::Draw
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    width,
    uint32_t                    height,
    Matrix&                     view,
    Matrix&                     proj,
    D3D12_GPU_DESCRIPTOR_HANDLE target,
    D3D12_GPU_DESCRIPTOR_HANDLE cubeMap
)
{
    if (pCmd == nullptr || target.ptr == 0 || cubeMap.ptr == 0)
        return;

    m_ConstantBuffer.SwapBuffer();

    Param param = {};
    param.View      = view;
    param.Proj      = proj;
    param.Width     = width;
    param.Height    = height;
    param.InvWidth  = 1.0f / float(width);
    param.InvHeight = 1.0f / float(height);

    m_ConstantBuffer.Update(&param, sizeof(param));

    pCmd->SetComputeRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_PipelineState.GetPtr());
    pCmd->SetComputeRootConstantBufferView(ROOT_PARAM_B0, m_ConstantBuffer.GetGpuAddress());
    pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_T0, cubeMap);
    pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_U0, target);

    auto x = RoundDiv(width,  8u);
    auto y = RoundDiv(height, 8u);
    pCmd->Dispatch(x, y, 1);
}


///////////////////////////////////////////////////////////////////////////////
// BackgroundSphere class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
BackgroundSphere::BackgroundSphere()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
BackgroundSphere::~BackgroundSphere()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool BackgroundSphere::Init()
{
    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    // ルートシグニチャ生成.
    {
        D3D12_DESCRIPTOR_RANGE ranges[2] = {};
        InitRangeAsSRV(ranges[0], 0);
        InitRangeAsUAV(ranges[1], 0);

        D3D12_ROOT_PARAMETER params[MAX_ROOT_PARAM_COUNT] = {};
        InitAsCBV(params[ROOT_PARAM_B0], 0, D3D12_SHADER_VISIBILITY_ALL);
        InitAsTable(params[ROOT_PARAM_T0], 1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
        InitAsTable(params[ROOT_PARAM_U0], 1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters      = _countof(params);
        desc.pParameters        = params;
        desc.NumStaticSamplers  = _countof(Preset::StaticSamplers);
        desc.pStaticSamplers    = Preset::StaticSamplers;
        desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;

        if (!InitRootSignature(pDevice, &desc, m_RootSignature.GetAddress()))
        {
            ELOG("Error : InitRootSingature() Failed.");
            return false;
        }
    }

    // パイプラインステート生成.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.CS = { asdxBackgroundSphereCS, sizeof(asdxBackgroundSphereCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_PipelineState.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // 定数バッファ生成.
    {
        auto size = RoundUp<uint64_t>(sizeof(Param), 256);
        if (!m_ConstantBuffer.Init(size))
        {
            ELOG("Error : ConstantBuffer::Init() Failed.");
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void BackgroundSphere::Term()
{
    m_ConstantBuffer.Term();
    m_PipelineState.Reset();
    m_RootSignature.Reset();
}

//-----------------------------------------------------------------------------
//      スフィアマップを描画します.
//-----------------------------------------------------------------------------
void BackgroundSphere::Draw
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    width,
    uint32_t                    height,
    Matrix&                     view,
    Matrix&                     proj,
    D3D12_GPU_DESCRIPTOR_HANDLE target,
    D3D12_GPU_DESCRIPTOR_HANDLE sphereMap
)
{
    if (pCmd == nullptr || target.ptr == 0 || sphereMap.ptr == 0)
        return;

    m_ConstantBuffer.SwapBuffer();

    Param param = {};
    param.View      = view;
    param.Proj      = proj;
    param.Width     = width;
    param.Height    = height;
    param.InvWidth  = 1.0f / float(width);
    param.InvHeight = 1.0f / float(height);

    m_ConstantBuffer.Update(&param, sizeof(param));

    pCmd->SetComputeRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_PipelineState.GetPtr());
    pCmd->SetComputeRootConstantBufferView(ROOT_PARAM_B0, m_ConstantBuffer.GetGpuAddress());
    pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_T0, sphereMap);
    pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_U0, target);

    auto x = RoundDiv(width,  8u);
    auto y = RoundDiv(height, 8u);
    pCmd->Dispatch(x, y, 1);
}

} // namespace asdx
