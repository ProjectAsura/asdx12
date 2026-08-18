//-----------------------------------------------------------------------------
// File : asdxBloomEffect.cpp
// Desc : Kawase's Bloom Effect.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <gfx/asdxBloomEffect.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxPresetState.h>
#include <gfx/asdxLegacyBarrier.h>
#include <gfx/asdxScopedMarker.h>


namespace {

//-----------------------------------------------------------------------------
// Shaders
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/asdxBloomFirstPassCS.inc"
#include "../res/shaders/Compiled/asdxGaussianBlurCS.inc"
#include "../res/shaders/Compiled/asdxBloomCompositeCS.inc"
#include "../res/shaders/Compiled/asdxBloomFinalPassCS.inc"


///////////////////////////////////////////////////////////////////////////////
// ROOT_PARAM enum
///////////////////////////////////////////////////////////////////////////////
enum ROOT_PARAM
{
    ROOT_PARAM_CBV0,
    ROOT_PARAM_SRV0,
    ROOT_PARAM_UAV0,
    ROOT_PARAM_SRV1,
};

///////////////////////////////////////////////////////////////////////////////
// BloomFirstParam structure
///////////////////////////////////////////////////////////////////////////////
struct BloomFirstParam
{
    uint16_t    SrcW;
    uint16_t    SrcH;
    uint16_t    DstW;
    uint16_t    DstH;
    float       Threshold;
    float       Exposure;
};

///////////////////////////////////////////////////////////////////////////////
// BloomDownParam structure
///////////////////////////////////////////////////////////////////////////////
struct BloomDownParam
{
    float       Weights[8];
    float       Offsets[8];
    uint16_t    SrcW;
    uint16_t    SrcH;
    uint16_t    DstW;
    uint16_t    DstH;
    uint32_t    Flags;
};

///////////////////////////////////////////////////////////////////////////////
// BloomCompositeParam structure
///////////////////////////////////////////////////////////////////////////////
struct BloomCompositeParam
{
    uint16_t    SrcW;
    uint16_t    SrcH;
    uint16_t    DstW;
    uint16_t    DstH;
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
void ComputeGaussWeights(float sigma, BloomDownParam& param)
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
// BloomEffect class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
BloomEffect::BloomEffect()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
BloomEffect::~BloomEffect()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool BloomEffect::Init(uint32_t w, uint32_t h, DXGI_FORMAT format)
{
    if (w == 0 || h == 0 || format == DXGI_FORMAT_UNKNOWN)
        return false;

    auto pDevice = GetD3D12Device();

    // ルートシグニチャの初期化.
    {
        D3D12_DESCRIPTOR_RANGE range[3] = {};
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

        range[2].RangeType                          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range[2].NumDescriptors                     = 1;
        range[2].BaseShaderRegister                 = 1;
        range[2].RegisterSpace                      = 0;
        range[2].OffsetInDescriptorsFromTableStart  = 0;

        D3D12_ROOT_PARAMETER param[4] = {};
        param[ROOT_PARAM_CBV0].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[ROOT_PARAM_CBV0].Constants.Num32BitValues   = 19;
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

        param[ROOT_PARAM_SRV1].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[ROOT_PARAM_SRV1].DescriptorTable.NumDescriptorRanges    = 1;
        param[ROOT_PARAM_SRV1].DescriptorTable.pDescriptorRanges      = &range[2];
        param[ROOT_PARAM_SRV1].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_ALL;

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

    // 最初のパス用のコンピュートパイプラインステート.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.CS             = { asdxBloomFirstPassCS, sizeof(asdxBloomFirstPassCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_FirstPassPSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // ダウンサンプル用のコンピュートパイプラインステート.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.CS             = { asdxGaussianBlurCS, sizeof(asdxGaussianBlurCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_DownPassPSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // 合成用のコンピュートパイプラインステート.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.CS             = { asdxBloomCompositeCS, sizeof(asdxBloomCompositeCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_CompositePSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // 最終パス用のコンピュートパイプラインステート.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.CS             = { asdxBloomFinalPassCS, sizeof(asdxBloomFinalPassCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_FinalPassPSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // コンピュートターゲットの初期化.
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

        if (!m_ComputeTarget.Init(&desc))
        {
            ELOGA("Error : ComputeTarget::Init() Failed.");
            return false;
        }

        m_ComputeTargetStates = desc.InitState;
    }

    // ブラーターゲットの初期化.
    {
        TargetDesc desc = {};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width              = w / 4;
        desc.Height             = h / 4;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = format;
        desc.SampleDesc         = { 1, 0 };
        desc.InitState          = D3D12_RESOURCE_STATE_COMMON;
        desc.ClearColor[0]      = 0.0f;
        desc.ClearColor[1]      = 0.0f;
        desc.ClearColor[2]      = 0.0f;
        desc.ClearColor[3]      = 0.0f;

        if (desc.Width  % 2 != 0) desc.Width++;
        if (desc.Height % 2 != 0) desc.Height++;

        for(auto i=0u; i<kMaxTargetCount; i+=2)
        {
            if (!m_BlurTarget[i + 0].Init(&desc))
            {
                ELOGA("Error : ComputeTarget::Init() Failed. index = %u", i + 0);
                return false;
            }

            if (!m_BlurTarget[i + 1].Init(&desc))
            {
                ELOGA("Error : ComputeTarget::Init() Failed. index = %u", i + 1);
                return false;
            }

            m_BlurTargetStates[i + 0] = desc.InitState;
            m_BlurTargetStates[i + 1] = desc.InitState;

            desc.Width  >>= 1;
            desc.Height >>= 1;

            if (desc.Width  < 1) desc.Width  = 1;
            if (desc.Height < 1) desc.Height = 1;

            // 偶数になるように調整.
            if (desc.Width  % 2 != 0) desc.Width++;
            if (desc.Height % 2 != 0) desc.Height++;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void BloomEffect::Term()
{
    m_FirstPassPSO .Reset();
    m_DownPassPSO  .Reset();
    m_CompositePSO .Reset();
    m_FinalPassPSO .Reset();

    m_RootSignature .Reset();

    for(auto i=0u; i<kMaxTargetCount; ++i)
    { m_BlurTarget[i].Term(); }
    m_ComputeTarget.Term();
}

//-----------------------------------------------------------------------------
//      リサイズ処理を行います.
//-----------------------------------------------------------------------------
void BloomEffect::Resize(uint32_t w, uint32_t h)
{
    m_ComputeTarget.Resize(w, h);

    w = w / 4;
    h = h / 4;

    // 偶数になるように調整.
    if (w % 2 != 0) w++;
    if (h % 2 != 0) h++;

    for(auto i=0u; i<kMaxTargetCount; i+=2)
    {
        m_BlurTarget[i * 2 + 0].Resize(w, h);
        m_BlurTarget[i * 2 + 1].Resize(w, h);

        w >>= 1;
        h >>= 1;

        if (w < 1) w = 1;
        if (h < 1) h = 1;

        // 偶数になるように調整.
        if (w % 2 != 0) w++;
        if (h % 2 != 0) h++;
    }
}

//-----------------------------------------------------------------------------
//      エフェクトを適用します.
//-----------------------------------------------------------------------------
void BloomEffect::Dispatch
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    width,
    uint32_t                    height,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSRV
)
{
    ASDX_SCOPED_MARKER(pCmd, KawaseBloomEffect);

    assert(pCmd != nullptr);
    assert(width != 0);
    assert(height != 0);
    assert(handleSRV.ptr != 0);

    LegacyBarrier barrier;

    pCmd->SetComputeRootSignature(m_RootSignature.GetPtr());

    // ファーストパス.
    {
        ASDX_SCOPED_MARKER(pCmd, FirstPass);

        auto desc = m_BlurTarget[1].GetDesc();

        BloomFirstParam param = {};
        param.Threshold = m_Threshold;
        param.Exposure  = m_Exposure;
        param.SrcW      = width;
        param.SrcH      = height;
        param.DstW      = uint16_t(desc.Width);
        param.DstH      = uint16_t(desc.Height);

        auto threadX = (param.DstW + 7u) / 8u;
        auto threadY = (param.DstH + 7u) / 8u;

        barrier.Transition( m_BlurTarget[1].GetResource(), m_BlurTargetStates[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        barrier.Apply(pCmd);
        m_BlurTargetStates[1] = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

        pCmd->SetPipelineState(m_FirstPassPSO.GetPtr());
        pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_CBV0, 4, &param, 0);
        pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_SRV0, handleSRV);
        pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_UAV0, m_BlurTarget[1].GetGpuHandleUAV());
        pCmd->Dispatch(threadX, threadY, 1);
    }

    // ダウンサンプルパス.
    {
        ASDX_SCOPED_MARKER(pCmd, DownSamplePass);

        auto desc = m_BlurTarget[0].GetDesc();

        BloomDownParam param = {};

        auto src = 1;
        auto dst = 0;

        auto w = uint32_t(desc.Width);
        auto h = uint32_t(desc.Height);

        uint16_t srcW = uint16_t(w);
        uint16_t srcH = uint16_t(h);

        pCmd->SetPipelineState(m_DownPassPSO.GetPtr());

        for(auto i=0u; i<kMaxTargetCount; i+=2)
        {
            auto threadX = (w + 7u) / 8u;
            auto threadY = (h + 7u) / 8u;

            dst = i + 0;

            auto desc = m_BlurTarget[dst].GetDesc();
            param.SrcW  = srcW;
            param.SrcH  = srcH;
            param.DstW = uint16_t(desc.Width);
            param.DstH = uint16_t(desc.Height);
            ComputeGaussWeights(m_BlurStrength, param);

            // 水平方向ブラー.
            {
                ASDX_SCOPED_MARKER(pCmd, BlurX);

                barrier.UAV(m_BlurTarget[src].GetResource());
                barrier.Transition(m_BlurTarget[src].GetResource(), m_BlurTargetStates[src], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                barrier.Transition(m_BlurTarget[dst].GetResource(), m_BlurTargetStates[dst], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                barrier.Apply(pCmd); 
                m_BlurTargetStates[src] = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
                m_BlurTargetStates[dst] = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

                param.Flags = 0;
                pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_CBV0, 19, &param, 0);
                pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_SRV0, m_BlurTarget[src].GetGpuHandleSRV());
                pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_UAV0, m_BlurTarget[dst].GetGpuHandleUAV());
                pCmd->Dispatch(threadX, threadY, 1);
            }

            src = dst;
            dst = i + 1;

            // 垂直方向ブラー.
            {
                ASDX_SCOPED_MARKER(pCmd, BlurY);

                barrier.UAV(m_BlurTarget[src].GetResource());
                barrier.Transition(m_BlurTarget[src].GetResource(), m_BlurTargetStates[src], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                barrier.Transition(m_BlurTarget[dst].GetResource(), m_BlurTargetStates[dst], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                barrier.Apply(pCmd);
                m_BlurTargetStates[src] = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
                m_BlurTargetStates[dst] = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

                param.Flags = 1;
                pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_CBV0, 19, &param, 0);
                pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_SRV0, m_BlurTarget[src].GetGpuHandleSRV());
                pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_UAV0, m_BlurTarget[dst].GetGpuHandleUAV());
                pCmd->Dispatch(threadX, threadY, 1);
            }

            src  = dst;
            srcW = param.DstW;
            srcH = param.DstH;
        }
    }

    // 合成パス.
    {
        ASDX_SCOPED_MARKER(pCmd, CompositePass);

        pCmd->SetPipelineState(m_CompositePSO.GetPtr());

        BloomCompositeParam param = {};

        auto count = kMaxTargetCount / 2;
        for(int i=count-1; i>=1; --i)
        {
            auto src = (i + 0) * 2 + 1;
            auto dst = (i - 1) * 2 + 1;

            auto srcDesc = m_BlurTarget[src].GetDesc();
            auto dstDesc = m_BlurTarget[dst].GetDesc();

            param.SrcW = uint16_t(srcDesc.Width);
            param.SrcH = uint16_t(srcDesc.Height);
            param.DstW = uint16_t(dstDesc.Width);
            param.DstH = uint16_t(dstDesc.Height);

            auto threadX = (param.DstW + 7u) / 8u;
            auto threadY = (param.DstH + 7u) / 8u;

            barrier.UAV(m_BlurTarget[src].GetResource());
            barrier.Transition(m_BlurTarget[src].GetResource(), m_BlurTargetStates[src], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            barrier.Transition(m_BlurTarget[dst].GetResource(), m_BlurTargetStates[dst], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            barrier.Apply(pCmd);
            m_BlurTargetStates[src] = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            m_BlurTargetStates[dst] = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

            pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_CBV0, 2, &param, 0);
            pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_SRV0, m_BlurTarget[src].GetGpuHandleSRV());
            pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_UAV0, m_BlurTarget[dst].GetGpuHandleUAV());
            pCmd->Dispatch(threadX, threadY, 1);
        }
    }

    // 最終パス.
    {
        ASDX_SCOPED_MARKER(pCmd, FinalPass);

        auto srcDesc = m_BlurTarget[1].GetDesc();
        auto dstDesc = m_ComputeTarget.GetDesc();

        BloomCompositeParam param = {};
        param.SrcW = uint16_t(srcDesc.Width);
        param.SrcH = uint16_t(srcDesc.Height);
        param.DstW = uint16_t(dstDesc.Width);
        param.DstH = uint16_t(dstDesc.Height);

        auto threadX = (param.DstW + 7u) / 8u;
        auto threadY = (param.DstH + 7u) / 8u;

        pCmd->SetPipelineState(m_FinalPassPSO.GetPtr());

        barrier.UAV(m_BlurTarget[1].GetResource());
        barrier.Transition(m_BlurTarget[1].GetResource(), m_BlurTargetStates[1], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        barrier.Transition(m_ComputeTarget.GetResource(), m_ComputeTargetStates, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        barrier.Apply(pCmd);
        m_BlurTargetStates[1] = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        m_ComputeTargetStates = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

        pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_CBV0, 2, &param, 0);
        pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_SRV0, m_BlurTarget[1].GetGpuHandleSRV());
        pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_SRV1, handleSRV);
        pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_UAV0, m_ComputeTarget.GetGpuHandleUAV());
        pCmd->Dispatch(threadX, threadY, 1);

        barrier.UAV(m_ComputeTarget.GetResource());
        barrier.Transition(m_ComputeTarget.GetResource(), m_ComputeTargetStates, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
        barrier.Apply(pCmd);
        m_ComputeTargetStates = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
    }
}

//-----------------------------------------------------------------------------
//      SRVハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE BloomEffect::GetGpuHandleSRV() const
{ return m_ComputeTarget.GetGpuHandleSRV(); }

//-----------------------------------------------------------------------------
//      UAVハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE BloomEffect::GetGpuHandleUAV() const
{ return m_ComputeTarget.GetGpuHandleUAV(); }

//-----------------------------------------------------------------------------
//      ブラー用SRVハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE BloomEffect::GetBlurGpuHandleSRV(uint8_t index) const
{
    assert(index < kMaxTargetCount);
    return m_BlurTarget[index].GetGpuHandleSRV();
}

//-----------------------------------------------------------------------------
//      ブラー用UAVハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE BloomEffect::GetBlurGpuHandleUAV(uint8_t index) const
{
    assert(index < kMaxTargetCount);
    return m_BlurTarget[index].GetGpuHandleUAV();
}

//-----------------------------------------------------------------------------
//      閾値を設定します.
//-----------------------------------------------------------------------------
void BloomEffect::SetThreshold(float value)
{ m_Threshold = value; }

//-----------------------------------------------------------------------------
//      閾値を取得します.
//-----------------------------------------------------------------------------
float BloomEffect::GetThreshold() const
{ return m_Threshold; }

//-----------------------------------------------------------------------------
//      ブラーの強さを設定します.
//-----------------------------------------------------------------------------
void BloomEffect::SetBlurStrength(float value)
{
    assert(value > 0.0f);
    m_BlurStrength = value;
}

//-----------------------------------------------------------------------------
//      ブラーの強さを取得します.
//-----------------------------------------------------------------------------
float BloomEffect::GetBlurStrength() const
{ return m_BlurStrength; }

//-----------------------------------------------------------------------------
//      露出値を設定します.
//-----------------------------------------------------------------------------
void BloomEffect::SetExposure(float value)
{
    assert(value >= 0.0f);
    m_Exposure = value;
}

//-----------------------------------------------------------------------------
//      露出値を取得します.
//-----------------------------------------------------------------------------
float BloomEffect::GetExposure() const
{ return m_Exposure; }

} // namespace asdx
