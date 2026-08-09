//-----------------------------------------------------------------------------
// File : asdxSmaa.cpp
// Desc : Subpixel Morphological Anti-Aliasing.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <gfx/asdxSmaa.h>
#include <gfx/asdxTextureManager.h>
#include <gfx/asdxPresetState.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxLegacyBarrier.h>
#include <gfx/asdxScopedMarker.h>
#include <res/asdxResTexture.h>

namespace {

//-----------------------------------------------------------------------------
// Textures
//-----------------------------------------------------------------------------
#include "../../external/SMAA/Textures/AreaTex.h"
#include "../../external/SMAA/Textures/SearchTex.h"

//-----------------------------------------------------------------------------
// Shaders
//-----------------------------------------------------------------------------
#include "../../res/shaders/Compiled/asdxSmaaEdgeDetectionCS.inc"
#include "../../res/shaders/Compiled/asdxSmaaCalcBlendWeightCS.inc"
#include "../../res/shaders/Compiled/asdxSmaaNeighborBlendingCS.inc"


///////////////////////////////////////////////////////////////////////////////
// ROOT_PARAM enum
///////////////////////////////////////////////////////////////////////////////
enum ROOT_PARAM
{
    ROOT_CBV0,
    ROOT_SRV0,
    ROOT_SRV1,
    ROOT_SRV2,
    ROOT_UAV0,
};

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Smaa class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Smaa::Smaa()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Smaa::~Smaa()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Smaa::Init(uint32_t w, uint32_t h, DXGI_FORMAT format)
{
    if (w == 0 || h == 0 || format == DXGI_FORMAT_UNKNOWN)
    {
        ELOGA("Error : Invalid Argument.");
        return false;
    }

    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    // ルートシグニチャ生成.
    {
        D3D12_DESCRIPTOR_RANGE range[4] = {};
        range[0].RangeType                          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range[0].NumDescriptors                     = 1;
        range[0].BaseShaderRegister                 = 0;
        range[0].RegisterSpace                      = 0;
        range[0].OffsetInDescriptorsFromTableStart  = 0;

        range[1].RangeType                          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range[1].NumDescriptors                     = 1;
        range[1].BaseShaderRegister                 = 1;
        range[1].RegisterSpace                      = 0;
        range[1].OffsetInDescriptorsFromTableStart  = 0;

        range[2].RangeType                          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range[2].NumDescriptors                     = 1;
        range[2].BaseShaderRegister                 = 2;
        range[2].RegisterSpace                      = 0;
        range[2].OffsetInDescriptorsFromTableStart  = 0;

        range[3].RangeType                          = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        range[3].NumDescriptors                     = 1;
        range[3].BaseShaderRegister                 = 0;
        range[3].RegisterSpace                      = 0;
        range[3].OffsetInDescriptorsFromTableStart  = 0;

        D3D12_ROOT_PARAMETER param[5] = {};
        param[ROOT_CBV0].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[ROOT_CBV0].Constants.Num32BitValues   = 4;
        param[ROOT_CBV0].Constants.ShaderRegister   = 0;
        param[ROOT_CBV0].Constants.RegisterSpace    = 0;
        param[ROOT_CBV0].ShaderVisibility           = D3D12_SHADER_VISIBILITY_ALL;

        param[ROOT_SRV0].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[ROOT_SRV0].DescriptorTable.NumDescriptorRanges    = 1;
        param[ROOT_SRV0].DescriptorTable.pDescriptorRanges      = &range[0];
        param[ROOT_SRV0].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_ALL;

        param[ROOT_SRV1].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[ROOT_SRV1].DescriptorTable.NumDescriptorRanges    = 1;
        param[ROOT_SRV1].DescriptorTable.pDescriptorRanges      = &range[1];
        param[ROOT_SRV1].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_ALL;

        param[ROOT_SRV2].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[ROOT_SRV2].DescriptorTable.NumDescriptorRanges    = 1;
        param[ROOT_SRV2].DescriptorTable.pDescriptorRanges      = &range[2];
        param[ROOT_SRV2].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_ALL;

        param[ROOT_UAV0].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[ROOT_UAV0].DescriptorTable.NumDescriptorRanges    = 1;
        param[ROOT_UAV0].DescriptorTable.pDescriptorRanges      = &range[3];
        param[ROOT_UAV0].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_ALL;

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

    // エッジ検出用パイプラインステート.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.CS = { asdxSmaaEdgeDetectionCS, sizeof(asdxSmaaEdgeDetectionCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_EdgeDetectionPSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // ブレンドウェイト計算用パイプラインステート.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.CS = { asdxSmaaCalcBlendWeightCS, sizeof(asdxSmaaCalcBlendWeightCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_EdgeDetectionPSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // ブレンド処理用パイプラインステート.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.CS = { asdxSmaaNeighborBlendingCS, sizeof(asdxSmaaNeighborBlendingCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_EdgeDetectionPSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // エッジターゲット生成.
    {
        TargetDesc desc = {};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width              = w;
        desc.Height             = h;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = DXGI_FORMAT_R8G8_UNORM;
        desc.SampleDesc         = { 1, 0 };
        desc.InitState          = D3D12_RESOURCE_STATE_COMMON;
        desc.ClearColor[0]      = 0.0f;
        desc.ClearColor[1]      = 0.0f;
        desc.ClearColor[2]      = 0.0f;
        desc.ClearColor[3]      = 0.0f;

        if (!m_EdgeTarget.Init(&desc))
        {
            ELOGA("Error : ComputeTarget::Init() Failed.");
            return false;
        }

        m_EdgeTargetState = desc.InitState;
    }

    // ブレンドウェイトターゲット.
    {
        TargetDesc desc = {};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width              = w;
        desc.Height             = h;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc         = { 1, 0 };
        desc.InitState          = D3D12_RESOURCE_STATE_COMMON;
        desc.ClearColor[0]      = 0.0f;
        desc.ClearColor[1]      = 0.0f;
        desc.ClearColor[2]      = 0.0f;
        desc.ClearColor[3]      = 0.0f;

        if (!m_WeightTarget.Init(&desc))
        {
            ELOGA("Error : ComputeTarget::Init() Failed.");
            return false;
        }

        m_WeightTargetState = desc.InitState;
    }

    // 出力用ターゲット生成.
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

        if (!m_OutputTarget.Init(&desc))
        {
            ELOGA("Error : ComputeTarget::Init() Failed.");
            return false;
        }

        m_OutputTargetState = desc.InitState;
    }

    // 検索テクスチャ初期化.
    {
        ResSubResource subRes = {};
        subRes.Width        = SEARCHTEX_WIDTH;
        subRes.Height       = SEARCHTEX_HEIGHT;
        subRes.RowPitch     = SEARCHTEX_PITCH;
        subRes.SlicePitch   = SEARCHTEX_SIZE;
        subRes.PixelOffset  = 0;

        ResTexture res = {};
        res.Dimension           = TEXTURE_DIMENSION_2D;
        res.Width               = SEARCHTEX_WIDTH;
        res.Height              = SEARCHTEX_HEIGHT;
        res.DepthOrArraySize    = 1;
        res.Format              = DXGI_FORMAT_R8_UNORM;
        res.MipLevels           = 1;
        res.SubResources        = ArrayView<ResSubResource>(&subRes, 1);
        res.Pixels              = ArrayView<uint8_t>(::searchTexBytes, SEARCHTEX_SIZE);

        if (!TextureManager::Instance().CreateTexture(res, m_SearchTexture.GetAddress()))
        {
            ELOGA("Error : TextureManager::CreateTexture() Failed.");
            return false;
        }
    }

    // 範囲テクスチャ初期化.
    {
        ResSubResource subRes = {};
        subRes.Width        = AREATEX_WIDTH;
        subRes.Height       = AREATEX_HEIGHT;
        subRes.RowPitch     = AREATEX_PITCH;
        subRes.SlicePitch   = AREATEX_SIZE;
        subRes.PixelOffset  = 0;

        ResTexture res = {};
        res.Dimension           = TEXTURE_DIMENSION_2D;
        res.Width               = AREATEX_WIDTH;
        res.Height              = AREATEX_HEIGHT;
        res.DepthOrArraySize    = 1;
        res.Format              = DXGI_FORMAT_R8G8_UNORM;
        res.MipLevels           = 1;
        res.SubResources        = ArrayView<ResSubResource>(&subRes, 1);
        res.Pixels              = ArrayView<uint8_t>(::areaTexBytes, AREATEX_SIZE);

        if (!TextureManager::Instance().CreateTexture(res, m_AreaTexture.GetAddress()))
        {
            ELOGA("Error : TextureManager::CreateTexture() Failed.");
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Smaa::Term()
{
    m_EdgeDetectionPSO   .Reset();
    m_CalcBlendWeightPSO .Reset();
    m_NeighborBlendingPSO.Reset();
    m_RootSignature      .Reset();

    m_EdgeTarget  .Term();
    m_WeightTarget .Term();
    m_OutputTarget.Term();

    m_SearchTexture.Reset();
    m_AreaTexture  .Reset();
}

//-----------------------------------------------------------------------------
//      リサイズ処理を行います.
//-----------------------------------------------------------------------------
void Smaa::Resize(uint32_t w, uint32_t h)
{
    m_EdgeTarget  .Resize(w, h);
    m_WeightTarget .Resize(w, h);
    m_OutputTarget.Resize(w, h);
}

//-----------------------------------------------------------------------------
//      SMAAを適用します.
//-----------------------------------------------------------------------------
void Smaa::Dispatch(ID3D12GraphicsCommandList* pCmd, D3D12_GPU_DESCRIPTOR_HANDLE handleSRV)
{
    if (pCmd == nullptr || handleSRV.ptr == 0)
        return;

    ASDX_SCOPED_MARKER(pCmd, Smaa);

    auto desc = m_OutputTarget.GetDesc();
    auto dstW = uint32_t(desc.Width);
    auto dstH = uint32_t(desc.Height);

    struct Param
    {
        float InvW;
        float InvH;
        float W;
        float H;
    };
    Param param = {};
    param.InvW  = 1.0f / float(dstW);
    param.InvH  = 1.0f / float(dstH);
    param.W     = float(dstW);
    param.H     = float(dstH);

    auto threadX = (dstW + 7u) / 8u;
    auto threadY = (dstH + 7u) / 8u;

    LegacyBarrier barrier;

    pCmd->SetComputeRootSignature(m_RootSignature.GetPtr());

    // エッジ検出.
    {
        ASDX_SCOPED_MARKER(pCmd, EdgeDection);

        barrier.Transition(m_EdgeTarget.GetResource(), m_EdgeTargetState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        barrier.Apply(pCmd);
        m_EdgeTargetState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

        pCmd->SetPipelineState(m_EdgeDetectionPSO.GetPtr());
        pCmd->SetComputeRoot32BitConstants(ROOT_CBV0, 4, &param, 0);
        pCmd->SetComputeRootDescriptorTable(ROOT_SRV0, handleSRV);
        pCmd->SetComputeRootDescriptorTable(ROOT_UAV0, m_EdgeTarget.GetGpuHandleUAV());
        pCmd->Dispatch(threadX, threadY, 1);

    }

    // ブレンドウェイト計算.
    {
        ASDX_SCOPED_MARKER(pCmd, CalcBlendWeight);

        barrier.UAV(m_EdgeTarget.GetResource());
        barrier.Transition(m_EdgeTarget  .GetResource(), m_EdgeTargetState,   D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        barrier.Transition(m_WeightTarget.GetResource(), m_WeightTargetState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        barrier.Apply(pCmd);
        m_EdgeTargetState   = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        m_WeightTargetState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

        pCmd->SetPipelineState(m_EdgeDetectionPSO.GetPtr());
        pCmd->SetComputeRoot32BitConstants(ROOT_CBV0, 4, &param, 0);
        pCmd->SetComputeRootDescriptorTable(ROOT_SRV0, m_EdgeTarget.GetGpuHandleSRV());
        pCmd->SetComputeRootDescriptorTable(ROOT_SRV1, m_AreaTexture->GetHandleGPU());
        pCmd->SetComputeRootDescriptorTable(ROOT_SRV2, m_SearchTexture->GetHandleGPU());
        pCmd->SetComputeRootDescriptorTable(ROOT_UAV0, m_WeightTarget.GetGpuHandleUAV());
        pCmd->Dispatch(threadX, threadY, 1);

    }

    // ブレンディング処理.
    {
        ASDX_SCOPED_MARKER(pCmd, Blending);

        barrier.UAV(m_WeightTarget.GetResource());
        barrier.Transition(m_WeightTarget.GetResource(), m_WeightTargetState, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        barrier.Transition(m_OutputTarget.GetResource(), m_OutputTargetState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        barrier.Apply(pCmd);
        m_WeightTargetState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        m_OutputTargetState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

        pCmd->SetPipelineState(m_EdgeDetectionPSO.GetPtr());
        pCmd->SetComputeRoot32BitConstants(ROOT_CBV0, 4, &param, 0);
        pCmd->SetComputeRootDescriptorTable(ROOT_SRV0, handleSRV);
        pCmd->SetComputeRootDescriptorTable(ROOT_SRV1, m_WeightTarget.GetGpuHandleSRV());
        pCmd->SetComputeRootDescriptorTable(ROOT_UAV0, m_EdgeTarget.GetGpuHandleUAV());
        pCmd->Dispatch(threadX, threadY, 1);

        barrier.UAV(m_OutputTarget.GetResource());
        barrier.Transition(m_OutputTarget.GetResource(), m_OutputTargetState, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
        barrier.Apply(pCmd);
        m_OutputTargetState = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
    }
}

} // namespace asdx
