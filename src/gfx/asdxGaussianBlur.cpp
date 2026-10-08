//-----------------------------------------------------------------------------
// File : asdxGaussianBlurEffect.cpp
// Desc : Gaussian Blur Effect.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <gfx/asdxGaussianBlur.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxPresetState.h>
#include <gfx/asdxLegacyBarrier.h>
#include <gfx/asdxScopedMarker.h>


namespace {

//-----------------------------------------------------------------------------
// Shaders.
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/asdxGaussianBlurCS.inc"
#include "../res/shaders/Compiled/asdxGaussianBlurPS.inc"


///////////////////////////////////////////////////////////////////////////////
// ShaderParam structure
///////////////////////////////////////////////////////////////////////////////
struct ShaderParam
{
    float       Weights[8];
    float       Offsets[8];
    uint16_t    SrcW;
    uint16_t    SrcH;
    uint16_t    DstW;
    uint16_t    DstH;
    uint32_t    Flags;
};

//-----------------------------------------------------------------------------
//      2乗値を求めます.
//-----------------------------------------------------------------------------
inline float Pow2(float value)
{ return value * value; }

//-----------------------------------------------------------------------------
//      ガウスブラーの重みを求めます.
//-----------------------------------------------------------------------------
float CalcGaussianWeight(float index, float sigma)
{
    // あとで正規化するのと，正規化項は定数倍なので比率計算には影響しないので考慮しない.
    return expf(-0.5f * Pow2(index) / Pow2(sigma));
}

//-----------------------------------------------------------------------------
//      バイリニアオフセットを求めます.
//-----------------------------------------------------------------------------
float CalcBilinearOffset(float w0, float w1)
{ return w1 / (w0 + w1); }

//-----------------------------------------------------------------------------
//      ガウスブラーの重みとオフセットを求めます.
//-----------------------------------------------------------------------------
float CalcGaussianWeightAndOffset(int index, float sigma, float& weight)
{
    float lhs = float(index) + 0.0f;
    float rhs = float(index) + 1.0f;

    float w0 = CalcGaussianWeight(lhs, sigma);
    float w1 = CalcGaussianWeight(rhs, sigma);

    if (index == 0)
        w0 *= 0.5f; // 中心は2回サンプルされるため，重みを半分に.

    float offset = CalcBilinearOffset(w0, w1);

    weight = (w0 + w1);
    return float(index) + offset;
}

//-----------------------------------------------------------------------------
//      ガウスブラーの重みを計算します.
//-----------------------------------------------------------------------------
void ComputeGaussWeights(float sigma, ShaderParam& param)
{
    float total = 0.0f;
    for(auto i=0; i<8; i++)
    {
        auto p = i * 2;
        auto w = 0.0f;

        param.Offsets[i] = CalcGaussianWeightAndOffset(p, sigma, w);
        param.Weights[i] = w;

        total += 2.0f * w;
    }

    // 正規化.
    auto invTotal = 1.0f / total;
    for(auto i=0; i<8; ++i)
        param.Weights[i] *= invTotal;
}

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// GaussianBlurCS class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
GaussianBlurCS::GaussianBlurCS()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
GaussianBlurCS::~GaussianBlurCS()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool GaussianBlurCS::Init()
{
    auto pDevice = asdx::GetD3D12Device();

    // ルートシグニチャの初期化.
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
        param[0].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[0].Constants.Num32BitValues   = 19;
        param[0].Constants.ShaderRegister   = 0;
        param[0].Constants.RegisterSpace    = 0;
        param[0].ShaderVisibility           = D3D12_SHADER_VISIBILITY_ALL;

        param[1].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[1].DescriptorTable.NumDescriptorRanges    = 1;
        param[1].DescriptorTable.pDescriptorRanges      = &range[0];
        param[1].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_ALL;

        param[2].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[2].DescriptorTable.NumDescriptorRanges    = 1;
        param[2].DescriptorTable.pDescriptorRanges      = &range[1];
        param[2].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_ALL;

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
            ELOG("Error : D3D12SerializeRootSignature() Failed. errcode = 0x%x", hr);
            if (errorBlob.GetPtr() != nullptr)
            {
                ELOG("Error : Msg = %s", reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
            }
            return false;
        }

        hr = pDevice->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateRootSignature() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // コンピュートパイプラインステートの初期化.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.CS = { asdxGaussianBlurCS, sizeof(asdxGaussianBlurCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_PipelineState.GetAddress()));
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
void GaussianBlurCS::Term()
{
    m_PipelineState.Reset();
    m_RootSignature.Reset();
}

//-----------------------------------------------------------------------------
//      コンピュートシェーダを起動します.
//-----------------------------------------------------------------------------
void GaussianBlurCS::Dispatch(ID3D12GraphicsCommandList* pCmd, Param& args)
{
    ASDX_SCOPED_MARKER(pCmd, GaussianBlurCS);

    assert(pCmd != nullptr);
    auto desc = args.Targets[0].GetDesc();

    ShaderParam param = {};
    param.SrcW = uint16_t(args.SrvWidth);
    param.SrcH = uint16_t(args.SrvHeight);
    param.DstW = uint16_t(desc.Width);
    param.DstH = uint16_t(desc.Height);
    param.Flags = 0;
    ComputeGaussWeights(args.BlurStrength, param);

    pCmd->SetComputeRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_PipelineState.GetPtr());

    LegacyBarrier barrier;

    auto threadX = (uint32_t(desc.Width)  + 7) / 8;
    auto threadY = (desc.Height + 7) / 8;

    auto handleSRV = args.HandleSRV;
    auto handleUAV = args.Targets[0].GetGpuHandleUAV();

    // 水平方向ブラー.
    {
        ASDX_SCOPED_MARKER(pCmd, BlurX);

        barrier.Transition(args.Targets[0].GetResource(), args.States[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        barrier.Apply(pCmd);

        pCmd->SetComputeRoot32BitConstants(0, 19, &param, 0);
        pCmd->SetComputeRootDescriptorTable(1, handleSRV);
        pCmd->SetComputeRootDescriptorTable(2, handleUAV);
        pCmd->Dispatch(threadX, threadY, 1);
    }

    handleSRV = args.Targets[0].GetGpuHandleSRV();
    handleUAV = args.Targets[1].GetGpuHandleUAV();

    // 垂直方向ブラー.
    {
        ASDX_SCOPED_MARKER(pCmd, BlurY);

        barrier.UAV(args.Targets[0].GetResource());
        barrier.Transition(args.Targets[0].GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        barrier.Transition(args.Targets[1].GetResource(), args.States[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        barrier.Apply(pCmd);

        param.Flags = 1;
        pCmd->SetComputeRoot32BitConstants(0, 19, &param, 0);
        pCmd->SetComputeRootDescriptorTable(1, handleSRV);
        pCmd->SetComputeRootDescriptorTable(2, handleUAV);
        pCmd->Dispatch(threadX, threadY, 1);

        barrier.UAV(args.Targets[1].GetResource());
        barrier.Transition(args.Targets[1].GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
        barrier.Apply(pCmd);

        args.States[0] = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        args.States[1] = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
    }
}


///////////////////////////////////////////////////////////////////////////////
// GaussianBlurPS class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
GaussianBlurPS::GaussianBlurPS()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
GaussianBlurPS::~GaussianBlurPS()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool GaussianBlurPS::Init(DXGI_FORMAT format)
{
    auto pDevice = asdx::GetD3D12Device();

    // ルートシグニチャの初期化.
    {
        D3D12_DESCRIPTOR_RANGE range[1] = {};
        range[0].RangeType                          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range[0].NumDescriptors                     = 1;
        range[0].BaseShaderRegister                 = 0;
        range[0].RegisterSpace                      = 0;
        range[0].OffsetInDescriptorsFromTableStart  = 0;

        D3D12_ROOT_PARAMETER param[2] = {};
        param[0].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[0].Constants.Num32BitValues   = 19;
        param[0].Constants.ShaderRegister   = 0;
        param[0].Constants.RegisterSpace    = 0;
        param[0].ShaderVisibility           = D3D12_SHADER_VISIBILITY_ALL;

        param[1].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[1].DescriptorTable.NumDescriptorRanges    = 1;
        param[1].DescriptorTable.pDescriptorRanges      = &range[0];
        param[1].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters      = _countof(param);
        desc.pParameters        = param;
        desc.NumStaticSamplers  = _countof(Preset::StaticSamplers);
        desc.pStaticSamplers    = Preset::StaticSamplers;
        desc.Flags              = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        desc.Flags             |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
        desc.Flags             |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
        desc.Flags             |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        RefPtr<ID3DBlob> blob;
        RefPtr<ID3DBlob> errorBlob;
        auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, blob.GetAddress(), errorBlob.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : D3D12SerializeRootSignature() Failed. errcode = 0x%x", hr);
            if (errorBlob.GetPtr() != nullptr)
            {
                ELOG("Error : Msg = %s", reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
            }
            return false;
        }

        hr = pDevice->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateRootSignature() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // パイプラインステート生成.
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature         = m_RootSignature.GetPtr();
        desc.VS                     = asdx::Preset::FullScreenVS;
        desc.PS                     = { asdxGaussianBlurPS, sizeof(asdxGaussianBlurPS) };
        desc.BlendState             = asdx::Preset::Opaque;
        desc.SampleMask             = D3D12_DEFAULT_SAMPLE_MASK;
        desc.RasterizerState        = asdx::Preset::CullNone;
        desc.DepthStencilState      = asdx::Preset::DepthNone;
        desc.InputLayout            = { asdx::Preset::QuadElements, _countof(asdx::Preset::QuadElements) };
        desc.PrimitiveTopologyType  = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets       = 1;
        desc.RTVFormats[0]          = format;
        desc.DSVFormat              = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc             = { 1, 0 };

        auto hr = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_PipelineState.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void GaussianBlurPS::Term()
{
    m_PipelineState.Reset();
    m_RootSignature.Reset();
}

//-----------------------------------------------------------------------------
//      描画を行います.
//-----------------------------------------------------------------------------
void GaussianBlurPS::Draw(ID3D12GraphicsCommandList* pCmd, Param& args)
{
    ASDX_SCOPED_MARKER(pCmd, GaussianBlurPS);

    assert(pCmd != nullptr);
    auto desc = args.Targets[0].GetDesc();

    ShaderParam param = {};
    param.SrcW = uint16_t(args.SrvWidth);
    param.SrcH = uint16_t(args.SrvHeight);
    param.DstW = uint16_t(desc.Width);
    param.DstH = uint16_t(desc.Height);
    param.Flags = 0;
    ComputeGaussWeights(args.BlurStrength, param);

    pCmd->SetGraphicsRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_PipelineState.GetPtr());

    LegacyBarrier barrier;

    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width    = FLOAT(desc.Width);
    viewport.Height   = FLOAT(desc.Height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    D3D12_RECT scissor = {};
    scissor.left    = 0;
    scissor.right   = LONG(desc.Width);
    scissor.top     = 0;
    scissor.bottom  = LONG(desc.Height);

    auto handleSRV = args.HandleSRV;

    // 水平方向ブラー.
    {
        ASDX_SCOPED_MARKER(pCmd, BlurX);

        barrier.Transition(args.Targets[0].GetResource(), args.States[0], D3D12_RESOURCE_STATE_RENDER_TARGET);
        barrier.Apply(pCmd);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
            args.Targets[0].GetCpuHandleRTV()
        };

        pCmd->OMSetRenderTargets(1, rtvs, FALSE, nullptr);
        pCmd->RSSetViewports(1, &viewport);
        pCmd->RSSetScissorRects(1, &scissor);

        pCmd->SetGraphicsRoot32BitConstants(0, 19, &param, 0);
        pCmd->SetGraphicsRootDescriptorTable(1, handleSRV);
        DrawQuad(pCmd);
    }

    handleSRV = args.Targets[0].GetGpuHandleSRV();

    // 垂直方向ブラー.
    {
        ASDX_SCOPED_MARKER(pCmd, BlurY);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
            args.Targets[1].GetCpuHandleRTV()
        };

        barrier.Transition(args.Targets[0].GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        barrier.Transition(args.Targets[1].GetResource(), args.States[1], D3D12_RESOURCE_STATE_RENDER_TARGET);
        barrier.Apply(pCmd);

        param.Flags = 1;
        pCmd->OMSetRenderTargets(1, rtvs, FALSE, nullptr);
        pCmd->RSSetViewports(1, &viewport);
        pCmd->RSSetScissorRects(1, &scissor);
        pCmd->SetGraphicsRoot32BitConstants(0, 19, &param, 0);
        pCmd->SetGraphicsRootDescriptorTable(1, handleSRV);
        DrawQuad(pCmd);

        barrier.Transition(args.Targets[1].GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
        barrier.Apply(pCmd);

        args.States[0] = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        args.States[1] = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
    }
}

} // namespace asdx
