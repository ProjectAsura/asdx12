//-----------------------------------------------------------------------------
// File : asdxFade.cpp
// Desc : Fade.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxFade.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxPresetState.h>
#include <res/asdxResTexture.h>
#include <fnd/asdxLogger.h>


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/asdxFullScreenVS.inc"
#include "../res/shaders/Compiled/asdxFadePS.inc"

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Fade class
///////////////////////////////////////////////////////////////////////////////
Fade Fade::s_Instance = {};

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Fade::Fade()
: m_RootSig         (nullptr)
, m_PipelineState   (nullptr)
, m_WhiteTexture    (nullptr)
, m_StartColor      (0.0f, 0.0f, 0.0f)
, m_TargetColor     (0.0f, 0.0f, 0.0f)
, m_CurrentColor    (0.0f, 0.0f, 0.0f)
, m_StartAlpha      (0.0f)
, m_TargetAlpha     (0.0f)
, m_CurrentAlpha    (0.0f)
, m_ElapsedSec      (0.0f)
, m_Complete        (false)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Fade::~Fade()
{ Term(); }

//-----------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-----------------------------------------------------------------------------
Fade& Fade::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Fade::Init(ID3D12GraphicsCommandList* pCmd, DXGI_FORMAT rtvFormat)
{
    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    // WhiteTextureの生成.
    {
        std::vector<uint8_t> pixels;
        pixels.resize(16 * 16 * 4);
        for(size_t i=0; i<pixels.size(); ++i)
        { pixels[i] = 255; }

        ResTexture res = {};
        res.Dimension                   = TEXTURE_DIMENSION_2D;
        res.Width                       = 16;
        res.Height                      = 16;
        res.DepthOrArraySize            = 1;
        res.Format                      = DXGI_FORMAT_R8G8B8A8_UNORM;
        res.MipLevels                   = 1;
        res.Pixels                      = ArrayView(pixels.data(), pixels.size());

        ResSubResource subRes = {};
        subRes.Width       = 16;
        subRes.Height      = 16;
        subRes.RowPitch    = 16 * 4;
        subRes.SlicePitch  = 16 * 16 * 4;
        subRes.PixelOffset =0;

        res.SubResources = ArrayView(&subRes, 1);

        if (!Texture::Create(pCmd, res, &m_WhiteTexture))
        {
            ELOG("Error : WhiteTexture Init Failed.");
            return false;
        }
    }

    // RootSignatureの生成.
    {
        D3D12_DESCRIPTOR_RANGE range[1] = {};
        range[0].RangeType                          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range[0].NumDescriptors                     = 1;
        range[0].BaseShaderRegister                 = 0;
        range[0].RegisterSpace                      = 0;
        range[0].OffsetInDescriptorsFromTableStart  = 0;

        D3D12_ROOT_PARAMETER param[2] = {};
        param[0].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[0].Constants.Num32BitValues   = 4;
        param[0].Constants.ShaderRegister   = 0;
        param[0].Constants.RegisterSpace    = 0;
        param[0].ShaderVisibility           = D3D12_SHADER_VISIBILITY_PIXEL;

        param[1].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[1].DescriptorTable.NumDescriptorRanges    = 1;
        param[1].DescriptorTable.pDescriptorRanges      = &range[0];
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

        hr = pDevice->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_RootSig.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateRootSignature() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // PipelineStateの生成.
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature                 = m_RootSig.GetPtr();
        desc.VS                             = { asdxFullScreenVS, sizeof(asdxFullScreenVS) };
        desc.PS                             = { asdxFadePS, sizeof(asdxFadePS) };
        desc.BlendState                     = Preset::AlphaBlend;
        desc.SampleMask                     = D3D12_DEFAULT_SAMPLE_MASK;
        desc.RasterizerState                = Preset::CullBack;
        desc.DepthStencilState              = Preset::DepthNone;
        desc.InputLayout.NumElements        = _countof(Preset::QuadElements);
        desc.InputLayout.pInputElementDescs = Preset::QuadElements;
        desc.PrimitiveTopologyType          = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets               = 1;
        desc.RTVFormats[0]                  = rtvFormat;
        desc.DSVFormat                      = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count               = 1;
        desc.SampleDesc.Quality             = 0;

        auto hr = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_PipelineState.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Fade::Term()
{
    if (m_WhiteTexture)
    {
        m_WhiteTexture->Release();
        m_WhiteTexture = nullptr;
    }
    m_PipelineState.Reset();
    m_RootSig      .Reset();

    m_Complete   = false;
    m_ElapsedSec = 0.0f;

    m_CurrentColor = asdx::Vector3(0.0f, 0.0f, 0.0f);
    m_CurrentAlpha = 0.0f;

    m_StartColor  = asdx::Vector3(0.0f, 0.0f, 0.0f);
    m_TargetColor = asdx::Vector3(0.0f, 0.0f, 0.0f);

    m_StartAlpha  = 0.0f;
    m_TargetAlpha = 0.0f;
}

//-----------------------------------------------------------------------------
//      更新処理を行います.
//-----------------------------------------------------------------------------
void Fade::Update(float deltaSec)
{
    if (m_Complete)
        return;

    m_ElapsedSec += deltaSec;

    float t = Saturate(m_ElapsedSec / m_DurationSec);
    m_CurrentColor = Vector3::Lerp(m_StartColor, m_TargetColor, t);
    m_CurrentAlpha = Lerp(m_StartAlpha, m_TargetAlpha, t);

    if (t >= 1.0f)
    {
        m_Complete     = true;
        m_CurrentColor = m_TargetColor;
        m_CurrentAlpha = m_TargetAlpha;
    }
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void Fade::Draw(ID3D12GraphicsCommandList* pCmd, D3D12_GPU_DESCRIPTOR_HANDLE handleSRV)
{
    if (m_CurrentAlpha <= 0.0f)
        return;

    float color[] = {
        m_CurrentColor.x,
        m_CurrentColor.y,
        m_CurrentColor.z,
        m_CurrentAlpha
    };

    pCmd->SetGraphicsRootSignature(m_RootSig.GetPtr());
    pCmd->SetPipelineState(m_PipelineState.GetPtr());
    pCmd->SetGraphicsRoot32BitConstants(0, 4, color, 0);
    pCmd->SetGraphicsRootDescriptorTable(1, handleSRV);

    DrawQuad(pCmd);
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void Fade::Draw(ID3D12GraphicsCommandList* pCmd)
{ Draw(pCmd, m_WhiteTexture->GetHandleGPU()); }

//-----------------------------------------------------------------------------
//      フェード処理を設定します.
//-----------------------------------------------------------------------------
void Fade::FadeTo(const asdx::Vector3& color, float alpha, float durationSec)
{
    m_StartColor  = m_CurrentColor;
    m_TargetColor = color;

    m_StartAlpha  = m_CurrentAlpha;
    m_TargetAlpha = alpha;

    m_DurationSec = durationSec;
    m_ElapsedSec  = 0.0f;
    m_Complete    = false;
}

//-----------------------------------------------------------------------------
//      フェードインします.
//-----------------------------------------------------------------------------
void Fade::FadeIn(float durationSec)
{ FadeTo(m_CurrentColor, 0.0f, durationSec); }

//-----------------------------------------------------------------------------
//      フェードアウトします.
//-----------------------------------------------------------------------------
void Fade::FadeOut(float durationSec)
{ FadeTo(m_CurrentColor, 1.0f, durationSec); }

//-----------------------------------------------------------------------------
//      フェードインします.
//-----------------------------------------------------------------------------
void Fade::FadeIn(const asdx::Vector3& color, float durationSec)
{ FadeTo(color, 0.0f, durationSec); }

//-----------------------------------------------------------------------------
//      フェードアウトします.
//-----------------------------------------------------------------------------
void Fade::FadeOut(const asdx::Vector3& color, float durationSec)
{ FadeTo(color, 1.0f, durationSec); }

//-----------------------------------------------------------------------------
//      明滅させます.
//-----------------------------------------------------------------------------
void Fade::Flash(const asdx::Vector3& color, float duration)
{
    m_CurrentColor = color;
    m_CurrentAlpha = 1.0f;
    FadeTo(color, 0.0f, duration);
}

//-----------------------------------------------------------------------------
//      完了済みかどうかチェックします.
//-----------------------------------------------------------------------------
bool Fade::IsComplete() const
{ return m_Complete; }

} // namespace asdx
