//-----------------------------------------------------------------------------
// File : asdxBloomEffect.cpp
// Desc : Bloom Effect.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <gfx/asdxBloomEffect.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxPresetState.h>
#include <gfx/asdxCommandList.h>


namespace {

//-----------------------------------------------------------------------------
// Shaders
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/asdxBloomFirstPassCS.inc"
#include "../res/shaders/Compiled/asdxBloomDownPassCS.inc"
#include "../res/shaders/Compiled/asdxBloomCompositeCS.inc"
#include "../res/shaders/Compiled/asdxSimpleUpscaleCS.inc"


///////////////////////////////////////////////////////////////////////////////
// ROOT_PARAM enum
///////////////////////////////////////////////////////////////////////////////
enum ROOT_PARAM
{
    ROOT_PARAM_CBV0,
    ROOT_PARAM_SRV0,
    ROOT_PARAM_UAV0,
};

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
        param[ROOT_PARAM_CBV0].Constants.Num32BitValues   = 4;
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
        desc.CS             = { asdxBloomDownPassCS, sizeof(asdxBloomDownPassCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_DownPassPSO.GetAddress()));;
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // アップスケール用のコンピュートパイプラインステート.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.CS             = { asdxSimpleUpscaleCS, sizeof(asdxSimpleUpscaleCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_UpscalePSO.GetAddress()));
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

        m_States = desc.InitState;

        if (!m_ComputeTarget.Init(&desc))
        {
            ELOGA("Error : ComputeTarget::Init() Failed.",);
            return false;
        }
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

        // 偶数になるように調整.
        if (desc.Width  % 2 != 0) desc.Width++;
        if (desc.Height % 2 != 0) desc.Height++;

        for(auto i=0u; i<kMaxTargetCount; ++i)
        {
            m_BlurStates[i] = desc.InitState;

            if (!m_BlurTarget[i].Init(&desc))
            {
                ELOGA("Error : ComputeTarget::Init() Failed. index = %u", i);
                return false;
            }

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
    for(auto i=0; i<kMaxTargetCount; ++i)
    { m_BlurTarget[i].Term(); }

    m_ComputeTarget.Term();

    m_FirstPassPSO .Reset();
    m_DownPassPSO  .Reset();
    m_CompositePSO .Reset();
    m_UpscalePSO   .Reset();
    m_RootSignature.Reset();
}

//-----------------------------------------------------------------------------
//      リサイズ処理を行います.
//-----------------------------------------------------------------------------
void BloomEffect::Resize(uint32_t w, uint32_t h)
{
    m_ComputeTarget.Resize(w, h);

    w >>= 1;
    h >>= 1;

    if (w < 1) w = 1;
    if (h < 1) h = 1;

    // 偶数になるように調整.
    if (w % 2 != 0) w++;
    if (h % 2 != 0) h++;

    for(auto i=0; i<kMaxTargetCount; ++i)
    {
        m_BlurTarget[i].Resize(w, h);

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
    assert(pCmd != nullptr);
    assert(width != 0);
    assert(height != 0);
    assert(handleSRV.ptr != 0);

    struct Param
    {
        uint16_t srcW;
        uint16_t srcH;
        uint16_t dstW;
        uint16_t dstH;
        float    threshold;
        uint32_t reserved;
    };

    pCmd->SetComputeRootSignature(m_RootSignature.GetPtr());

    D3D12_RESOURCE_BARRIER barriers[2] = {};

    // 最初のパス.
    {
        auto desc = m_BlurTarget[0].GetDesc();

        auto dstW = uint32_t(desc.Width);
        auto dstH = uint32_t(desc.Height);

        Param param = {};
        param.srcW      = uint16_t(width);
        param.srcH      = uint16_t(height);
        param.dstW      = uint16_t(dstW);
        param.dstH      = uint16_t(dstH);
        param.threshold = m_Threshold;

        SetTransitionBarrier(barriers[0], m_BlurTarget[0].GetResource(), m_BlurStates[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        m_BlurStates[0] = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        pCmd->ResourceBarrier(1, barriers);

        pCmd->SetPipelineState(m_FirstPassPSO.GetPtr());
        pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_CBV0, 4, &param, 0);
        pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_SRV0, handleSRV);
        pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_UAV0, m_BlurTarget[0].GetGpuHandleUAV());

        auto threadX = (dstW + 7u) / 8u;
        auto threadY = (dstH + 7u) / 8u;

        pCmd->Dispatch(threadX, threadY, 1);

        UAVBarrier(pCmd, m_BlurTarget[0].GetResource());
    }

    // ダウンサンプルパス.
    {
        pCmd->SetPipelineState(m_DownPassPSO.GetPtr());

        Param param = {};

        for(auto i=1u; i<kMaxTargetCount; ++i)
        {
            uint32_t srcIdx = i - 1;
            uint32_t dstIdx = i - 0;

            auto& srcTarget = m_BlurTarget[srcIdx];
            auto& dstTarget = m_BlurTarget[dstIdx];

            auto pSrcStates = &m_BlurStates[srcIdx];
            auto pDstStates = &m_BlurStates[dstIdx];

            auto srcDesc = srcTarget.GetDesc();
            auto srcW = uint32_t(srcDesc.Width);
            auto srcH = uint32_t(srcDesc.Height);

            auto dstDesc = dstTarget.GetDesc();
            auto dstW = uint32_t(dstDesc.Width);
            auto dstH = uint32_t(dstDesc.Height);

            param.srcW = uint16_t(srcW);
            param.srcH = uint16_t(srcH);
            param.dstW = uint16_t(dstW);
            param.dstH = uint16_t(dstH);

            SetTransitionBarrier(barriers[0], srcTarget.GetResource(), (*pSrcStates), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            SetTransitionBarrier(barriers[1], dstTarget.GetResource(), (*pDstStates), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            (*pSrcStates) = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            (*pDstStates) = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            pCmd->ResourceBarrier(2, barriers);

            pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_CBV0, 4, &param, 0);
            pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_SRV0, srcTarget.GetGpuHandleSRV());
            pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_UAV0, dstTarget.GetGpuHandleUAV());

            auto threadX = (dstW + 7u) / 8u;
            auto threadY = (dstH + 7u) / 8u;

            pCmd->Dispatch(threadX, threadY, 1);

            UAVBarrier(pCmd, dstTarget.GetResource());
        }
    }

    // 合成パス.
    {
        pCmd->SetPipelineState(m_CompositePSO.GetPtr());

        Param param = {};

        for(int i=kMaxTargetCount - 1; i >= 1; i--)
        {
            uint32_t srcIdx = i - 0;
            uint32_t dstIdx = i - 1;

            auto& srcTarget = m_BlurTarget[srcIdx];
            auto& dstTarget = m_BlurTarget[dstIdx];

            auto pSrcStates = &m_BlurStates[srcIdx];
            auto pDstStates = &m_BlurStates[dstIdx];

            auto srcDesc = srcTarget.GetDesc();
            auto dstDesc = dstTarget.GetDesc();

            auto srcW = uint32_t(srcDesc.Width);
            auto srcH = uint32_t(srcDesc.Height);

            auto dstW = uint32_t(dstDesc.Width);
            auto dstH = uint32_t(dstDesc.Height);

            param.srcW = uint16_t(srcW);
            param.srcH = uint16_t(srcH);
            param.dstW = uint16_t(dstW);
            param.dstH = uint16_t(dstH);

            SetTransitionBarrier(barriers[0], srcTarget.GetResource(), (*pSrcStates), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            SetTransitionBarrier(barriers[1], dstTarget.GetResource(), (*pDstStates), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            (*pSrcStates) = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            (*pDstStates) = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            pCmd->ResourceBarrier(2, barriers);

            pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_CBV0, 4, &param, 0);
            pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_SRV0, srcTarget.GetGpuHandleSRV());
            pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_UAV0, dstTarget.GetGpuHandleUAV());

            auto threadX = (dstW + 7u) / 8u;
            auto threadY = (dstH + 7u) / 8u;

            pCmd->Dispatch(threadX, threadY, 1);

            UAVBarrier(pCmd, dstTarget.GetResource());
        }

        // 最終解像度にアップスケール.
        {
            auto& srcTarget = m_BlurTarget[0];
            auto& dstTarget = m_ComputeTarget;

            auto pSrcStates = &m_BlurStates[0];
            auto pDstStates = &m_States;

            auto srcDesc = srcTarget.GetDesc();
            auto dstDesc = dstTarget.GetDesc();

            auto srcW = uint32_t(srcDesc.Width);
            auto srcH = uint32_t(srcDesc.Height);

            auto dstW = uint32_t(dstDesc.Width);
            auto dstH = uint32_t(dstDesc.Height);

            param.srcW = uint16_t(srcW);
            param.srcH = uint16_t(srcH);
            param.dstW = uint16_t(dstW);
            param.dstH = uint16_t(dstH);

            pCmd->SetPipelineState(m_UpscalePSO.GetPtr());

            SetTransitionBarrier(barriers[0], srcTarget.GetResource(), (*pSrcStates), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            SetTransitionBarrier(barriers[1], dstTarget.GetResource(), (*pDstStates), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            (*pSrcStates) = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            (*pDstStates) = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            pCmd->ResourceBarrier(2, barriers);

            pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_CBV0, 4, &param, 0);
            pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_SRV0, srcTarget.GetGpuHandleSRV());
            pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_UAV0, dstTarget.GetGpuHandleUAV());

            auto threadX = (dstW + 7u) / 8u;
            auto threadY = (dstH + 7u) / 8u;

            pCmd->Dispatch(threadX, threadY, 1);

            UAVBarrier(pCmd, dstTarget.GetResource());
        }

        // 最後に元画像を追加.
        {
            auto dstDesc = m_ComputeTarget.GetDesc();

            auto srcW = width;
            auto srcH = height;

            auto dstW = uint32_t(dstDesc.Width);
            auto dstH = uint32_t(dstDesc.Height);

            param.srcW = uint16_t(srcW);
            param.srcH = uint16_t(srcH);
            param.dstW = uint16_t(dstW);
            param.dstH = uint16_t(dstH);

            pCmd->SetPipelineState(m_CompositePSO.GetPtr());

            TransitionBarrier(pCmd, m_ComputeTarget.GetResource(), m_States, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            m_States = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

            pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_CBV0, 4, &param, 0);
            pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_SRV0, handleSRV);
            pCmd->SetComputeRootDescriptorTable(ROOT_PARAM_UAV0, m_ComputeTarget.GetGpuHandleUAV());

            auto threadX = (dstW + 7u) / 8u;
            auto threadY = (dstH + 7u) / 8u;

            pCmd->Dispatch(threadX, threadY, 1);

            UAVBarrier(pCmd, m_ComputeTarget.GetResource());
        }

        TransitionBarrier(pCmd, m_ComputeTarget.GetResource(), m_States, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
        m_States = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
    }
}

//-----------------------------------------------------------------------------
//      SRVハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE BloomEffect::GetGpuHandleSRV() const
{ return m_ComputeTarget.GetGpuHandleSRV(); }

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

} // namespace asdx
