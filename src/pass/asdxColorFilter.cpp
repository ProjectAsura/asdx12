//-----------------------------------------------------------------------------
// File : asdxColorFilter.cpp
// Desc : Color Filter Pass.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <pass/asdxColorFilter.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ColorFilter class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ColorFilter::ColorFilter()
: m_Param ()
, m_Input (nullptr)
, m_Output(nullptr)
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
    }

    // パイプラインステート初期化.
    {
    }

    // 定数バッファ初期化.
    {
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了時の処理です.
//-----------------------------------------------------------------------------
void ColorFilter::Term()
{
    m_Input  = nullptr;
    m_Output = nullptr;

    m_PSO    .Term();
    m_RootSig.Term();
    m_CB     .Term();
}

//-----------------------------------------------------------------------------
//      ビルド時の処理です.
//-----------------------------------------------------------------------------
PassResource* ColorFilter::OnBuild(IPassGraph* graph, PassResource* input)
{
    if (!m_Param.Enable)
    { return input; }

    m_Input = input;

    struct PassHolder
    {
        ColorFilter* Pass;
    };

    PassHolder holder = {};
    holder.Pass = this;

    graph->AddPass(
        "ColorFilter",
        [&holder](IPassGraphBuilder* builder){
            auto pass = holder.Pass;
            auto desc = GetDesc(pass->m_Input);
            desc.Usage      = PASS_RESOURCE_USAGE_UAV;
            desc.InitState  = PASS_RESOURCE_STATE_NONE;

            pass->m_Output = builder->Create(desc);

            builder->Read (pass->m_Input);
            builder->Write(pass->m_Output);
       },
        [&holder](IPassGraphContext* context){
           holder.Pass->Draw(context);
       }
    );

    return m_Output;
}

//-----------------------------------------------------------------------------
//      パラメータを取得します.
//-----------------------------------------------------------------------------
ColorFilter::Param& ColorFilter::GetParam()
{ return m_Param; }

//-----------------------------------------------------------------------------
//      描画時の処理です.
//-----------------------------------------------------------------------------
void ColorFilter::Draw(IPassGraphContext* context)
{
    auto cmd = context->GetCommandList();
    auto srv = context->GetSRV(m_Input);
    auto uav = context->GetUAV(m_Output);

    // 定数バッファ更新.
    {
    }

    // リソース設定.


    // 描画キック.
}

} // namespace asdx
