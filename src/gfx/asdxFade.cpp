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
#include <gfx/asdxPipelineState.h>
#include <res/asdxResTexture.h>
#include <fnd/asdxLogger.h>


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/FullScreenVS.inc"
#include "../res/shaders/Compiled/FadePS.inc"

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
: m_Color0      (0.0f, 0.0f, 0.0f, 0.0f)
, m_Color1      (1.0f, 1.0f, 1.0f, 1.0f)
, m_ChangeSec   (1.0f)
, m_ElapsedSec  (0.0f)
, m_Complete    (false)
, m_EnablePulse (false)
, m_PulseSpeed  (1.0f)
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
        res.SubResourceCount            = 1;
        res.SubResources[0].Width       = 16;
        res.SubResources[0].Height      = 16;
        res.SubResources[0].RowPitch    = 16 * 4;
        res.SubResources[0].SlicePitch  = 16 * 16 * 4;
        res.SubResources[0].pPixels     = pixels.data();

        if (!m_WhiteTexture.Init(pCmd, res))
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
        desc.VS                             = { FullScreenVS, sizeof(FullScreenVS) };
        desc.PS                             = { FadePS, sizeof(FadePS) };
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
    m_WhiteTexture .Term();
    m_PipelineState.Reset();
    m_RootSig      .Reset();

    m_Complete   = false;
    m_ChangeSec  = 0.0f;
    m_ElapsedSec = 0.0f;
}

//-----------------------------------------------------------------------------
//      更新処理を行います.
//-----------------------------------------------------------------------------
void Fade::Update(float deltaSec)
{
    if (m_Complete)
        return;

    m_ElapsedSec += deltaSec;
    if (m_ElapsedSec > m_ChangeSec)
    {
        m_Complete   = true;
        m_ElapsedSec = m_ChangeSec;
    }
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void Fade::Draw(ID3D12GraphicsCommandList* pCmd, D3D12_GPU_DESCRIPTOR_HANDLE handleSRV)
{
    auto amount = asdx::Saturate(m_ElapsedSec / m_ChangeSec);
    Vector4 color;
    // フェード.
    if (!m_EnablePulse)
    { color  = Vector4::Lerp(m_Color0, m_Color1, amount); }
    // 点滅エフェクト.
    else
    {
        if (amount < 1.0f)
        {
            float phase = 1.0f + sinf(m_ElapsedSec * m_ChangeSec * m_PulseSpeed);
            color.x = (1.0f - phase) * m_Color1.x;
            color.y = (1.0f - phase) * m_Color1.y;
            color.z = (1.0f - phase) * m_Color1.z;
            color.w = (1.0f - phase) * m_Color1.w;
        }
        else
        {
            color = m_Color1;
        }
    }

    pCmd->SetGraphicsRootSignature(m_RootSig.GetPtr());
    pCmd->SetPipelineState(m_PipelineState.GetPtr());
    pCmd->SetGraphicsRoot32BitConstants(0, 4, &color.x, 0);
    pCmd->SetGraphicsRootDescriptorTable(1, handleSRV);

    DrawQuad(pCmd);
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void Fade::Draw(ID3D12GraphicsCommandList* pCmd)
{ Draw(pCmd, m_WhiteTexture.GetGpuHandleSRV()); }

//-----------------------------------------------------------------------------
//      カラー0を設定します.
//-----------------------------------------------------------------------------
void Fade::SetColor0(const asdx::Vector4& value)
{ m_Color0 = value; }

//-----------------------------------------------------------------------------
//      カラー0を設定します.
//-----------------------------------------------------------------------------
void Fade::SetColor0(float r, float g, float b, float a)
{
    m_Color0.x = r;
    m_Color0.y = g;
    m_Color0.z = b;
    m_Color0.w = a;
}

//-----------------------------------------------------------------------------
//      カラー1を設定します.
//-----------------------------------------------------------------------------
void Fade::SetColor1(const asdx::Vector4& value)
{ m_Color1 = value; }

//-----------------------------------------------------------------------------
//      カラー1を設定します.
//-----------------------------------------------------------------------------
void Fade::SetColor1(float r, float g, float b, float a)
{
    m_Color1.x = r;
    m_Color1.y = g;
    m_Color1.z = b;
    m_Color1.w = a;
}

//-----------------------------------------------------------------------------
//      切り替え時間を設定します.
//-----------------------------------------------------------------------------
void Fade::SetChangeSec(float value)
{ m_ChangeSec = value; }

//-----------------------------------------------------------------------------
//      カラー0を取得します.
//-----------------------------------------------------------------------------
const asdx::Vector4& Fade::GetColor0() const
{ return m_Color0; }

//-----------------------------------------------------------------------------
//      カラー1を取得します.
//-----------------------------------------------------------------------------
const asdx::Vector4& Fade::GetColor1() const
{ return m_Color1; }

//-----------------------------------------------------------------------------
//      切り替え時間を取得します.
//-----------------------------------------------------------------------------
float Fade::GetChangeSec() const
{ return m_ChangeSec; }

//-----------------------------------------------------------------------------
//      ステートをリセットします.
//-----------------------------------------------------------------------------
void Fade::ResetState()
{
    m_Complete   = false;
    m_ElapsedSec = 0.0f;
}

//-----------------------------------------------------------------------------
//      完了済みかどうかチェックします.
//-----------------------------------------------------------------------------
bool Fade::IsComplete() const
{ return m_Complete; }

//-----------------------------------------------------------------------------
//      点滅フラグを設定します.
//-----------------------------------------------------------------------------
void Fade::SetEnablePulse(bool value)
{ m_EnablePulse = value; }

//-----------------------------------------------------------------------------
//      点滅フラグを取得します.
//-----------------------------------------------------------------------------
bool Fade::IsEnablePulse() const
{ return m_EnablePulse; }

//-----------------------------------------------------------------------------
//      点滅速度を設定します.
//-----------------------------------------------------------------------------
void Fade::SetPulseSpeed(float value)
{ m_PulseSpeed = value; }

//-----------------------------------------------------------------------------
//      点滅速度を取得します.
//-----------------------------------------------------------------------------
float Fade::GetPulseSpeed() const
{ return m_PulseSpeed; }

} // namespace asdx
