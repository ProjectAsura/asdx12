//-----------------------------------------------------------------------------
// File : asdxCrossBilateralFilter.cpp
// Desc : Cross Bilateral Filter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxMath.h>
#include <asdxLogger.h>
#include <asdxGraphicsDevice.h>
#include <renderer/asdxCrossBilateralFilter.h>


namespace { 

#include "../res/shaders/Compiled/CrossBilateralFilterCS.inc"

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const uint32_t kThreadSize = 8;


///////////////////////////////////////////////////////////////////////////////
// CbCrossBilateralFilter structure
///////////////////////////////////////////////////////////////////////////////
struct CbCrossBilateralFilter
{
    asdx::Vector2   Offset;
    asdx::Vector2   InvTargetSize;
    uint32_t        DispatchX;
    uint32_t        DispatchY;
    float           Sharpness;
    float           Padding0;
};

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// CrossBilateralFilter class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
CrossBilateralFilter::~CrossBilateralFilter()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool CrossBilateralFilter::Init()
{
    // ルートシグニチャ初期化.
    {
        asdx::RootSignatureDesc desc;
        desc.AddCBV("CbCrossBilateralFilter", asdx::SV_ALL, 0);
        desc.AddSRV("Input",  asdx::SV_ALL, 0);
        desc.AddUAV("Output", asdx::SV_ALL, 0);

        if (!m_RootSig.Init(GetD3D12Device(), desc))
        {
            ELOG("Error : RootSignature::Init() Failed.");
            return false;
        }
    }

    // パイプラインステートを初期化.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature     = m_RootSig.GetPtr();
        desc.CS.pShaderBytecode = CrossBilateralFilterCS;
        desc.CS.BytecodeLength  = sizeof(CrossBilateralFilterCS);

        if (!m_PSO.Init(GetD3D12Device(), &desc))
        {
            ELOG("Error : PipelineState::Init() Failed.");
            return false;
        }
    }

    if (!m_CBX.Init(sizeof(CbCrossBilateralFilter)))
    {
        ELOG("Error : ConstantBuffer::Init() Failed.");
        return false;
    }

    if (!m_CBY.Init(sizeof(CbCrossBilateralFilter)))
    {
        ELOG("Error : ConstantBuffer::Init() Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void CrossBilateralFilter::Term()
{
    m_CBX    .Term();
    m_CBY    .Term();
    m_PSO    .Term();
    m_RootSig.Term();
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void CrossBilateralFilter::Draw
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

        CbCrossBilateralFilter res = {};
        res.DispatchX       = dispatchX;
        res.DispatchY       = dispatchY;
        res.InvTargetSize.x = 1.0f / float(w);
        res.InvTargetSize.y = 1.0f / float(h);
        res.Offset.x = 1.0f / w;
        res.Offset.y = 0.0f;
        res.Sharpness = m_Param.Sharpness;

        {
            m_CBX.SwapBuffer();
            auto ptr = m_CBX.Map();
            memcpy(ptr, &res, sizeof(res));
            m_CBX.Unmap();
        }

        res.Offset.x = 0.0f;
        res.Offset.y = 1.0f / h;
        {
            m_CBY.SwapBuffer();
            auto ptr = m_CBY.Map();
            memcpy(ptr, &res, sizeof(res));
            m_CBY.Unmap();
        }
    }

    pCmdList->SetComputeRootSignature(m_RootSig.GetPtr());
    pCmdList->SetPipelineState(m_PSO.GetPtr());
    pCmdList->SetGraphicsRootConstantBufferView (0, m_CBX.GetView()->GetHandleCPU().ptr);
    pCmdList->SetGraphicsRootShaderResourceView (1, pSRV->GetHandleGPU().ptr);
    pCmdList->SetGraphicsRootUnorderedAccessView(2, pUAV->GetHandleGPU().ptr);
    pCmdList->Dispatch(dispatchX, dispatchY, 1);

    pCmdList->SetGraphicsRootConstantBufferView(0, m_CBY.GetView()->GetHandleCPU().ptr);
    pCmdList->Dispatch(dispatchX, dispatchY, 1);
}

} // namespace asdx
