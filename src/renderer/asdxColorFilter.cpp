//-----------------------------------------------------------------------------
// File : asdxColorFilter.cpp
// Desc : Color Filter Pass.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxLogger.h>
#include <asdxGraphicsDevice.h>
#include <asdxColorMatrix.h>
#include <renderer/asdxColorFilter.h>


namespace {

#include "../res/shaders/Compiled/ColorFilterCS.inc"

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const uint32_t kThreadSize = 8;


///////////////////////////////////////////////////////////////////////////////
// CbColorFilter structure
///////////////////////////////////////////////////////////////////////////////
struct CbColorFilter
{
    uint32_t        DispatchX;
    uint32_t        DispatchY;
    asdx::Vector2   InvTargetSize;
    asdx::Matrix    ColorMatrix;
};

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ColorFilter class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ColorFilter::ColorFilter()
: m_Param ()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ColorFilter::~ColorFilter()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化時の処理です.
//-----------------------------------------------------------------------------
bool ColorFilter::Init()
{
    // ルートシグニチャ初期化.
    {
        asdx::RootSignatureDesc desc;
        desc.AddCBV("CbColorFilter", asdx::SV_ALL, 0);
        desc.AddSRV("Input",  asdx::SV_ALL, 0);
        desc.AddUAV("Output", asdx::SV_ALL, 0);

        if (!m_RootSig.Init(GetD3D12Device(), desc))
        {
            ELOG("Error : RootSignature::Init() Failed.");
            return false;
        }
    }

    // パイプラインステート初期化.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature     = m_RootSig.GetPtr();
        desc.CS.pShaderBytecode = ColorFilterCS;
        desc.CS.BytecodeLength  = sizeof(ColorFilterCS);

        if (!m_PSO.Init(GetD3D12Device(), &desc))
        {
            ELOG("Error : PipelineState::Init() Failed.");
            return false;
        }
    }

    // 定数バッファ初期化.
    {
        if (!m_CB.Init(sizeof(CbColorFilter)))
        {
            ELOG("Error : ConstantBuffer::Init() Failed.");
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了時の処理です.
//-----------------------------------------------------------------------------
void ColorFilter::Term()
{
    m_PSO    .Term();
    m_RootSig.Term();
    m_CB     .Term();
}

//-----------------------------------------------------------------------------
//      パラメータを取得します.
//-----------------------------------------------------------------------------
ColorFilter::Param& ColorFilter::GetParam()
{ return m_Param; }

//-----------------------------------------------------------------------------
//      描画処理です.
//-----------------------------------------------------------------------------
void ColorFilter::Draw
(
    ID3D12GraphicsCommandList6* pCmdList,
    IUnorderedAccessView*       pUAV,
    IShaderResourceView*        pSRV
)
{
    uint32_t dispatchX = 0;
    uint32_t dispatchY = 0;

    // 定数バッファ更新.
    {
        auto desc = pSRV->GetResource()->GetDesc();

        auto w = uint32_t(desc.Width);
        auto h = desc.Height;

        dispatchX = (w + kThreadSize - 1) / kThreadSize;
        dispatchY = (h + kThreadSize - 1) / kThreadSize;

        asdx::Matrix matrix = asdx::Matrix::CreateIdentity();
        if (m_Param.Enable)
        {
            matrix *= asdx::CreateBrightnessMatrix(m_Param.Brightness);
            matrix *= asdx::CreateSaturationMatrix(m_Param.Saturation);
            matrix *= asdx::CreateContrastMatrix(m_Param.Contrast);
            matrix *= asdx::CreateHueMatrix(m_Param.Hue);
            matrix *= asdx::CreateSepiaMatrix(m_Param.SepiaTone);
            matrix *= asdx::CreateGrayScaleMatrix(m_Param.GrayScale);
        }

        CbColorFilter res = {};
        res.DispatchX       = dispatchX;
        res.DispatchY       = dispatchY;
        res.InvTargetSize.x = 1.0f / float(w);
        res.InvTargetSize.y = 1.0f / float(h);
        res.ColorMatrix     = matrix;

        m_CB.SwapBuffer();

        auto ptr = m_CB.Map();
        memcpy(ptr, &res, sizeof(res));
        m_CB.Unmap();
    }

    // リソース設定.
    pCmdList->SetComputeRootSignature(m_RootSig.GetPtr());
    pCmdList->SetPipelineState(m_PSO.GetPtr());
    pCmdList->SetGraphicsRootConstantBufferView (0, m_CB.GetView()->GetHandleCPU().ptr);
    pCmdList->SetGraphicsRootShaderResourceView (1, pSRV->GetHandleGPU().ptr);
    pCmdList->SetGraphicsRootUnorderedAccessView(2, pUAV->GetHandleGPU().ptr);

    // 描画キック.
    pCmdList->Dispatch(dispatchX, dispatchY, 1);
}

} // namespace asdx
