//-----------------------------------------------------------------------------
// File : asdxKawaseGlareEffect.cpp
// Desc : Kawase's Glare Effect.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <gfx/asdxKawaseGlareEffect.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxPresetState.h>
#include <gfx/asdxCommandList.h>


namespace {

//-----------------------------------------------------------------------------
// Shaders
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/asdxBloomFirstPassCS.inc"
#include "../res/shaders/Compiled/asdxGaussianBlurCS.inc"
#include "../res/shaders/Compiled/asdxBloomCompositeCS.inc"


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
// BloomFirstParam structure
///////////////////////////////////////////////////////////////////////////////
struct BloomFirstParam
{
    float       Threshold;
    uint16_t    SizeX;
    uint16_t    SizeY;
};

///////////////////////////////////////////////////////////////////////////////
// BloomDownParam structure
///////////////////////////////////////////////////////////////////////////////
struct BloomDownParam
{
    float       Weights[8];
    float       Offsets[8];
    uint16_t    SizeX;
    uint16_t    SizeY;
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
// KawaseGlareEffect class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
KawaseGlareEffect::KawaseGlareEffect()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
KawaseGlareEffect::~KawaseGlareEffect()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool KawaseGlareEffect::Init(uint32_t w, uint32_t h, DXGI_FORMAT format)
{
    if (w == 0 || h == 0 || format == DXGI_FORMAT_UNKNOWN)
        return false;

    auto pDevice = GetD3D12Device();

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
        param[ROOT_PARAM_CBV0].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[ROOT_PARAM_CBV0].Constants.Num32BitValues   = 18;
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
        desc.NumStaticSamplers  = _countof(asdx::Preset::StaticSamplers);
        desc.pStaticSamplers    = asdx::Preset::StaticSamplers;
        desc.Flags              = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        desc.Flags             |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
        desc.Flags             |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
        desc.Flags             |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

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

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_BloomFirstPassPSO.GetAddress()));
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

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_BloomDownPassPSO.GetAddress()));
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

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_BloomCompositePSO.GetAddress()));
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

        if (desc.Width  % 2 != 0) desc.Width++;
        if (desc.Height % 2 != 0) desc.Height++;

        for(auto i=0u; i<kMaxTargetCount; i+=2)
        {
            if (!m_ComputeTarget[i + 0].Init(&desc))
            {
                ELOGA("Error : ComputeTarget::Init() Failed. index = %u", i + 0);
                return false;
            }

            if (!m_ComputeTarget[i + 1].Init(&desc))
            {
                ELOGA("Error : ComputeTarget::Init() Failed. index = %u", i + 1);
                return false;
            }

            m_TargetStates[i + 0] = desc.InitState;
            m_TargetStates[i + 1] = desc.InitState;

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
void KawaseGlareEffect::Term()
{
    m_BloomFirstPassPSO .Reset();
    m_BloomDownPassPSO  .Reset();
    m_BloomCompositePSO .Reset();

    m_RootSignature .Reset();

    for(auto i=0u; i<kMaxTargetCount; ++i)
    { m_ComputeTarget[i].Term(); }
}

//-----------------------------------------------------------------------------
//      リサイズ処理を行います.
//-----------------------------------------------------------------------------
void KawaseGlareEffect::Resize(uint32_t w, uint32_t h)
{
    for(auto i=0u; i<kMaxTargetCount; i+=2)
    {
        m_ComputeTarget[i * 2 + 0].Resize(w, h);
        m_ComputeTarget[i * 2 + 1].Resize(w, h);

        w >>= 1;
        h >>= 1;

        if (w < 1) w = 1;
        if (h < 1) h = 1;
    }
}

//-----------------------------------------------------------------------------
//      制御パラメータを設定します.
//-----------------------------------------------------------------------------
void KawaseGlareEffect::SetParam(const Param& param)
{ m_Param = param; }

//-----------------------------------------------------------------------------
//      制御パラメータを取得します.
//-----------------------------------------------------------------------------
const KawaseGlareEffect::Param& KawaseGlareEffect::GetParam() const
{ return m_Param; }

//-----------------------------------------------------------------------------
//      エフェクトを適用します.
//-----------------------------------------------------------------------------
void KawaseGlareEffect::Apply
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    width,
    uint32_t                    height,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSRV
)
{
    // ブルームを適用.
    ApplyBloom(pCmd, width, height, handleSRV);
}

//-----------------------------------------------------------------------------
//      SRVハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE KawaseGlareEffect::GetGpuHandleSRV(uint8_t index) const
{
    assert(index < kMaxTargetCount);
    return m_ComputeTarget[index].GetGpuHandleSRV();
}

//-----------------------------------------------------------------------------
//      ブルームエフェクトを適用します.
//-----------------------------------------------------------------------------
void KawaseGlareEffect::ApplyBloom
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    width,
    uint32_t                    height,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSRV
)
{
    assert(pCmd != nullptr);
    assert(width != 0);
    assert(height != 0);
    assert(handleSRV.ptr != 0);

    D3D12_RESOURCE_BARRIER barriers[3] = {};

    pCmd->SetComputeRootSignature(m_RootSignature.GetPtr());

    // ファーストパス.
    {
        auto desc = m_ComputeTarget[1].GetDesc();

        BloomFirstParam param = {};
        param.Threshold = m_Param.Threshold;
        param.SizeX     = uint16_t(desc.Width);
        param.SizeY     = uint16_t(desc.Height);

        auto threadX = (param.SizeX + 7u) / 8u;
        auto threadY = (param.SizeY + 7u) / 8u;

        SetTransitionBarrier(barriers[0], m_ComputeTarget[1].GetResource(), m_TargetStates[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        m_TargetStates[1] = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

        pCmd->ResourceBarrier(1, barriers);

        pCmd->SetPipelineState(m_BloomFirstPassPSO.GetPtr());
        pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_CBV0, 2, &param, 0);
        pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_SRV0, handleSRV);
        pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_UAV0, m_ComputeTarget[1].GetGpuHandleUAV());
        pCmd->Dispatch(threadX, threadY, 1);
    }

    // ダウンサンプルパス.
    {
        auto desc = m_ComputeTarget[0].GetDesc();

        BloomDownParam param = {};

        auto src = 1;
        auto dst = 0;

        auto w = uint32_t(desc.Width);
        auto h = uint32_t(desc.Height);

        pCmd->SetPipelineState(m_BloomDownPassPSO.GetPtr());

        for(auto i=0u; i<kMaxTargetCount; i+=2)
        {
            auto threadX = (w + 7u) / 8u;
            auto threadY = (h + 7u) / 8u;

            dst = i + 0;

            auto desc = m_ComputeTarget[dst].GetDesc();
            param.SizeX = uint16_t(desc.Width);
            param.SizeY = uint16_t(desc.Height);
            ComputeGaussWeights(m_Param.BloomStrength, param);

            // 水平方向ブラー.
            {
                SetUAVBarrier(barriers[0], m_ComputeTarget[src].GetResource());
                SetTransitionBarrier(barriers[1], m_ComputeTarget[src].GetResource(), m_TargetStates[src], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                SetTransitionBarrier(barriers[2], m_ComputeTarget[dst].GetResource(), m_TargetStates[dst], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
 
                m_TargetStates[src] = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
                m_TargetStates[dst] = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

                pCmd->ResourceBarrier(3, barriers);

                param.Flags = 0;
                pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_CBV0, 18, &param, 0);
                pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_SRV0, m_ComputeTarget[src].GetGpuHandleSRV());
                pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_UAV0, m_ComputeTarget[dst].GetGpuHandleUAV());
                pCmd->Dispatch(threadX, threadY, 1);
            }

            src = dst;
            dst = i + 1;

            // 垂直方向ブラー.
            {
                SetUAVBarrier(barriers[0], m_ComputeTarget[src].GetResource());
                SetTransitionBarrier(barriers[1], m_ComputeTarget[src].GetResource(), m_TargetStates[src], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                SetTransitionBarrier(barriers[2], m_ComputeTarget[dst].GetResource(), m_TargetStates[dst], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

                m_TargetStates[src] = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
                m_TargetStates[dst] = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

                pCmd->ResourceBarrier(3, barriers);

                param.Flags = 1;
                pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_CBV0, 18, &param, 0);
                pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_SRV0, m_ComputeTarget[src].GetGpuHandleSRV());
                pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_UAV0, m_ComputeTarget[dst].GetGpuHandleUAV());
                pCmd->Dispatch(threadX, threadY, 1);
            }

            src = dst;
        }
    }

    // 合成パス.
    {
        pCmd->SetPipelineState(m_BloomCompositePSO.GetPtr());

        BloomCompositeParam param = {};

        auto count = kMaxTargetCount / 2;
        for(int i=count-1; i>=1; --i)
        {
            auto src = (i + 0) * 2 + 1;
            auto dst = (i - 1) * 2 + 0;

            auto srcDesc = m_ComputeTarget[src].GetDesc();
            auto dstDesc = m_ComputeTarget[dst].GetDesc();

            param.SrcW = uint16_t(srcDesc.Width);
            param.SrcH = uint16_t(srcDesc.Height);
            param.DstW = uint16_t(dstDesc.Width);
            param.DstH = uint16_t(dstDesc.Height);

            auto threadX = (param.DstW + 7u) / 8u;
            auto threadY = (param.DstH + 7u) / 8u;

            SetUAVBarrier(barriers[0], m_ComputeTarget[src].GetResource());
            SetTransitionBarrier(barriers[1], m_ComputeTarget[src].GetResource(), m_TargetStates[src], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            SetTransitionBarrier(barriers[2], m_ComputeTarget[dst].GetResource(), m_TargetStates[dst], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            m_TargetStates[src] = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            m_TargetStates[dst] = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            pCmd->ResourceBarrier(3, barriers);

            pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_CBV0, 2, &param, 0);
            pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_SRV0, m_ComputeTarget[src].GetGpuHandleSRV());
            pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_UAV0, m_ComputeTarget[dst].GetGpuHandleUAV());
            pCmd->Dispatch(threadX, threadY, 1);
        }

        SetTransitionBarrier(barriers[0], m_ComputeTarget[0].GetResource(), m_TargetStates[0], D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
        m_TargetStates[0] = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
        pCmd->ResourceBarrier(1, barriers);
    }
}

} // namespace asdx
