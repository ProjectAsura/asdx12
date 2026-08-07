//-----------------------------------------------------------------------------
// File : asdxGaussianBlurEffect.cpp
// Desc : Gaussian Blur Effect.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <gfx/asdxGaussianBlurEffect.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxPresetState.h>
#include <gfx/asdxLegacyBarrier.h>


namespace {

//-----------------------------------------------------------------------------
// Shaders.
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/asdxGaussianBlurPS.inc"
#include "../res/shaders/Compiled/asdxGaussianBlurCS.inc"


///////////////////////////////////////////////////////////////////////////////
// Param structure
///////////////////////////////////////////////////////////////////////////////
struct Param
{
    float       Weights[8];
    float       Offsets[8];
    uint16_t    SizeX;
    uint16_t    SizeY;
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
void ComputeGaussWeights(float sigma, Param& param)
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
// GaussianBlurEffectPS class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
GaussianBlurEffectPS::GaussianBlurEffectPS()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
GaussianBlurEffectPS::~GaussianBlurEffectPS()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool GaussianBlurEffectPS::Init(uint32_t width, uint32_t height, DXGI_FORMAT format)
{
    auto pDevice = asdx::GetD3D12Device();

    // ルートシグニチャの初期化.
    {
        D3D12_DESCRIPTOR_RANGE range = {};
        range.RangeType                          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range.NumDescriptors                     = 1;
        range.BaseShaderRegister                 = 0;
        range.RegisterSpace                      = 0;
        range.OffsetInDescriptorsFromTableStart  = 0;

        D3D12_ROOT_PARAMETER param[2] = {};
        param[0].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[0].Constants.Num32BitValues   = 18;
        param[0].Constants.ShaderRegister   = 0;
        param[0].Constants.RegisterSpace    = 0;
        param[0].ShaderVisibility           = D3D12_SHADER_VISIBILITY_PIXEL;

        param[1].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[1].DescriptorTable.NumDescriptorRanges    = 1;
        param[1].DescriptorTable.pDescriptorRanges      = &range;
        param[1].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_PIXEL;

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
            ELOG("Error : ID3D12Device::CreateRootSignature() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // グラフィックスパイプラインステートの初期化.
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature                 = m_RootSignature.GetPtr();
        desc.VS                             = Preset::FullScreenVS;
        desc.PS                             = { asdxGaussianBlurPS, sizeof(asdxGaussianBlurPS) };
        desc.BlendState                     = Preset::Opaque;
        desc.SampleMask                     = D3D12_DEFAULT_SAMPLE_MASK;
        desc.RasterizerState                = Preset::CullNone;
        desc.DepthStencilState              = Preset::DepthNone;
        desc.InputLayout.NumElements        = _countof(Preset::QuadElements);
        desc.InputLayout.pInputElementDescs = Preset::QuadElements;
        desc.PrimitiveTopologyType          = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets               = 1;
        desc.RTVFormats[0]                  = format;
        desc.DSVFormat                      = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count               = 1;
        desc.SampleDesc.Quality             = 0;

        auto hr = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_PipelineState.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // カラーターゲットを初期化.
    {
        TargetDesc desc = {};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width              = width;
        desc.Height             = height;
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
            if (!m_ColorTarget[i].Init(&desc))
            {
                ELOGA("Error : ColorTarget::Init() Failed.");
                return false;
            }
        }
    }

    m_State = D3D12_RESOURCE_STATE_COMMON;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void GaussianBlurEffectPS::Term()
{
    for(auto i=0; i<2; ++i)
    { m_ColorTarget[i].Term(); }
    m_PipelineState.Reset();
    m_RootSignature.Reset();
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void GaussianBlurEffectPS::Draw
(
    ID3D12GraphicsCommandList*  pCmd,
    float                       strength,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSRV
)
{
    assert(pCmd != nullptr);
    assert(strength > 0.0f);

    auto desc = GetDesc();

    Param param = {};
    param.SizeX = uint16_t(desc.Width);
    param.SizeY = uint16_t(desc.Height);
    param.Flags = 0;
    ComputeGaussWeights(strength, param);

    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX   = 0;
    viewport.TopLeftY   = 0;
    viewport.Width      = float(desc.Width);
    viewport.Height     = float(desc.Height);
    viewport.MinDepth   = 0.0f;
    viewport.MaxDepth   = 1.0f;

    D3D12_RECT rect = {};
    rect.left   = 0;
    rect.right  = LONG(desc.Width);
    rect.top    = 0;
    rect.bottom = LONG(desc.Height);

    float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    LegacyBarrier barrier;
    barrier.Transition(m_ColorTarget[0].GetResource(), m_State, D3D12_RESOURCE_STATE_RENDER_TARGET);
    barrier.Apply(pCmd);

    auto handleRTV = m_ColorTarget[0].GetCpuHandleRTV();

    // ルートシグニチャとパイプラインステートを設定.
    pCmd->SetGraphicsRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_PipelineState.GetPtr());

    // 最初の1回だけクリア.
    if (m_State == D3D12_RESOURCE_STATE_COMMON)
    { pCmd->ClearRenderTargetView(handleRTV, clearColor, 0, nullptr); }

    // 水平方向ブラー.
    pCmd->OMSetRenderTargets(1, &handleRTV, FALSE, nullptr);
    pCmd->RSSetViewports(1, &viewport);
    pCmd->RSSetScissorRects(1, &rect);
    pCmd->SetGraphicsRoot32BitConstants(0, 18, &param, 0);
    pCmd->SetGraphicsRootDescriptorTable(1, handleSRV);
    DrawQuad(pCmd);

    barrier.Transition(m_ColorTarget[0].GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    barrier.Transition(m_ColorTarget[1].GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    barrier.Apply(pCmd);

    param.Flags = 1;
    handleSRV = m_ColorTarget[0].GetGpuHandleSRV();
    handleRTV = m_ColorTarget[1].GetCpuHandleRTV();

    // 最初の1回だけクリア.
    if (m_State == D3D12_RESOURCE_STATE_COMMON)
    { pCmd->ClearRenderTargetView(handleRTV, clearColor, 0, nullptr); }

    // 垂直方向ブラー.
    pCmd->OMSetRenderTargets(1, &handleRTV, FALSE, nullptr);
    pCmd->RSSetViewports(1, &viewport);
    pCmd->RSSetScissorRects(1, &rect);
    pCmd->SetGraphicsRoot32BitConstants(0, 18, &param, 0);
    pCmd->SetGraphicsRootDescriptorTable(1, handleSRV);
    DrawQuad(pCmd);

    barrier.Transition(m_ColorTarget[1].GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    barrier.Apply(pCmd);
    m_State = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
}

//-----------------------------------------------------------------------------
//      GPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE GaussianBlurEffectPS::GetHandleGPU() const
{ return m_ColorTarget[1].GetGpuHandleSRV(); }

//-----------------------------------------------------------------------------
//      構成設定を取得します.
//-----------------------------------------------------------------------------
TargetDesc GaussianBlurEffectPS::GetDesc() const
{ return m_ColorTarget[0].GetDesc(); }

//-----------------------------------------------------------------------------
//      ターゲットをリサイズします.
//-----------------------------------------------------------------------------
bool GaussianBlurEffectPS::Resize(uint32_t width, uint32_t height)
{
    for(auto i=0; i<2; ++i)
    {
        if (!m_ColorTarget[i].Resize(width, height))
        { return false; }
    }

    m_State = D3D12_RESOURCE_STATE_COMMON;

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// GaussianBlurEffectCS class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
GaussianBlurEffectCS::GaussianBlurEffectCS()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
GaussianBlurEffectCS::~GaussianBlurEffectCS()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool GaussianBlurEffectCS::Init(uint32_t width, uint32_t height, DXGI_FORMAT format)
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
        param[0].Constants.Num32BitValues   = 18;
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

    // コンピュートターゲットを初期化.
    {
        TargetDesc desc = {};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width              = width;
        desc.Height             = height;
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
            if (!m_ComputeTarget[i].Init(&desc))
            {
                ELOGA("Error : ColorTarget::Init() Failed.");
                return false;
            }
        }

        m_State = D3D12_RESOURCE_STATE_COMMON;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void GaussianBlurEffectCS::Term()
{
    for(auto i=0; i<2; ++i)
    { m_ComputeTarget[i].Term(); }
    m_PipelineState.Reset();
    m_RootSignature.Reset();
}

//-----------------------------------------------------------------------------
//      コンピュートシェーダを起動します.
//-----------------------------------------------------------------------------
void GaussianBlurEffectCS::Dispatch(ID3D12GraphicsCommandList* pCmd, float strength, D3D12_GPU_DESCRIPTOR_HANDLE handleSRV)
{
    assert(pCmd != nullptr);
    assert(strength > 0.0f);

    auto desc = GetDesc();

    Param param = {};
    param.SizeX = uint16_t(desc.Width);
    param.SizeY = uint16_t(desc.Height);
    param.Flags = 0;
    ComputeGaussWeights(strength, param);

    pCmd->SetComputeRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_PipelineState.GetPtr());

    LegacyBarrier barrier;
    barrier.Transition(m_ComputeTarget[0].GetResource(), m_State, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    barrier.Apply(pCmd);

    float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    auto threadX = (uint32_t(desc.Width)  + 7) / 8;
    auto threadY = (desc.Height + 7) / 8;

    auto handleUAV = m_ComputeTarget[0].GetGpuHandleUAV();

    pCmd->SetComputeRoot32BitConstants(0, 18, &param, 0);
    pCmd->SetComputeRootDescriptorTable(1, handleSRV);
    pCmd->SetComputeRootDescriptorTable(2, handleUAV);
    pCmd->Dispatch(threadX, threadY, 1);

    handleSRV = m_ComputeTarget[0].GetGpuHandleSRV();
    handleUAV = m_ComputeTarget[1].GetGpuHandleUAV();

    barrier.UAV(m_ComputeTarget[0].GetResource());
    barrier.Transition(m_ComputeTarget[0].GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    barrier.Transition(m_ComputeTarget[1].GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    barrier.Apply(pCmd);

    param.Flags = 1;
    pCmd->SetComputeRoot32BitConstants(0, 18, &param, 0);
    pCmd->SetComputeRootDescriptorTable(1, handleSRV);
    pCmd->SetComputeRootDescriptorTable(2, handleUAV);
    pCmd->Dispatch(threadX, threadY, 1);

    barrier.UAV(m_ComputeTarget[1].GetResource());
    barrier.Transition(m_ComputeTarget[1].GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
    barrier.Apply(pCmd);
    m_State = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
}

//-----------------------------------------------------------------------------
//      構成設定を取得します.
//-----------------------------------------------------------------------------
TargetDesc GaussianBlurEffectCS::GetDesc() const
{ return m_ComputeTarget[0].GetDesc(); }

//-----------------------------------------------------------------------------
//      GPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE GaussianBlurEffectCS::GetHandleGPU() const
{ return m_ComputeTarget[1].GetGpuHandleSRV(); }

//-----------------------------------------------------------------------------
//      ターゲットをリサイズします.
//-----------------------------------------------------------------------------
bool GaussianBlurEffectCS::Resize(uint32_t width, uint32_t height)
{
    for(auto i=0; i<2; ++i)
    {
        if (!m_ComputeTarget->Resize(width, height))
        { return false; }
    }

    m_State = D3D12_RESOURCE_STATE_COMMON;

    return true;
}

} // namespace asdx
