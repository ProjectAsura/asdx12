//-----------------------------------------------------------------------------
// File : ModelViewer.cpp
// Desc : Model Viewer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <ModelViewer.h>
#include <fnd/asdxMacro.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMisc.h>
#include <fnd/asdxPath.h>
#include <fnd/asdxMisc.h>
#include <fnd/asdxFileIO.h>
#include <edit/asdxGuiMgr.h>
#include <gfx/asdxPresetState.h>
#include "ModelConverter.h"
#include "MotionConverter.h"
#include "MaterialConverter.h"
#include <assimp/Exporter.hpp>


namespace {

//-----------------------------------------------------------------------------
// Shaders
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/MeshVS.inc"
#include "../res/shaders/Compiled/MeshPS.inc"

#include "../res/shaders/Compiled/ModelVS.inc"
#include "../res/shaders/Compiled/ModelPS.inc"

enum ROOT_PARAM
{
    ROOT_PARAM_B0,  // SceneParam.
    ROOT_PARAM_B1,  // mode
    ROOT_PARAM_T0,  // BaseColor
    ROOT_PARAM_T1,  // Normal
    ROOT_PARAM_T2,  // ORM
    ROOT_PARAM_T3,  // Emissive.
};

enum MODEL_ROOT_PARAM
{
    MODEL_ROOT_PARAM_CBV0,
    MODEL_ROOT_PARAM_CBV1,
    MODEL_ROOT_PARAM_CONSTANTS,
    MODEL_ROOT_PARAM_SRV0,
    MODEL_ROOT_PARAM_SRV1,
    MODEL_ROOT_PARAM_SRV2,
    MODEL_ROOT_PARAM_SRV3,
    MODEL_ROOT_PARAM_SRV4,
    MODEL_ROOT_PARAM_SRV5,
    MODEL_ROOT_PARAM_SRV6,
    MODEL_ROOT_PARAM_SRV7,
    MODEL_ROOT_PARAM_SRV8,
    MODEL_ROOT_PARAM_SRV9,
    MODEL_ROOT_PARAM_SRV10,
    MODEL_ROOT_PARAM_SRV11,
    MODEL_ROOT_PARAM_SRV12,
    MODEL_ROOT_PARAM_SRV13,
    MODEL_ROOT_PARAM_SRV14,
    MODEL_ROOT_PARAM_SRV15,
};

static const D3D12_INPUT_ELEMENT_DESC InputElements[] = {
    { "POSITION"   , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "NORMAL"     , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TANGENT"    , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD"   , 0, DXGI_FORMAT_R32G32_FLOAT      , 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "COLOR"      , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "BONEINDEX"  , 0, DXGI_FORMAT_R32G32B32A32_UINT , 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "BONEWEIGHT" , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};
static const uint32_t kStaticMeshElementCount   = 5;
static const uint32_t kSkeletalMeshElementCount = 7;

struct alignas(256) ParamScene
{
    asdx::Matrix    World;
    asdx::Matrix    View;
    asdx::Matrix    Proj;
    asdx::Vector3   CameraPos;
    float           FieldOfView;
    float           NearClip;
    float           FarClip;
    float           TargetWidth;
    float           TargetHeight;
};

//-----------------------------------------------------------------------------
//      ブレンドステート設定を取得します.
//-----------------------------------------------------------------------------
D3D12_BLEND_DESC GetBlendDesc(viewer::MaterialBlendState state)
{
    switch(state)
    {
    case viewer::MaterialBlendState::Opaque:
    default:
        return asdx::Preset::Opaque;

    case viewer::MaterialBlendState::AlphaBlend:
        return asdx::Preset::AlphaBlend;

    case viewer::MaterialBlendState::Additive:
        return asdx::Preset::Additive;

    case viewer::MaterialBlendState::Subtract:
        return asdx::Preset::Subtract;

    case viewer::MaterialBlendState::Premultiplied:
        return asdx::Preset::Premultiplied;

    case viewer::MaterialBlendState::Multiply:
        return asdx::Preset::Multiply;

    case viewer::MaterialBlendState::Screen:
        return asdx::Preset::Screen;
    }
}

//-----------------------------------------------------------------------------
//      深度ステート設定を取得します.
//-----------------------------------------------------------------------------
D3D12_DEPTH_STENCIL_DESC GetdepthStencilDesc(viewer::MaterialDepthState state)
{
    switch(state)
    {
    case viewer::MaterialDepthState::ReadWrite:
    default:
        return asdx::Preset::DepthReadWrite;

    case viewer::MaterialDepthState::ReadOnly:
        return asdx::Preset::DepthReadOnly;

    case viewer::MaterialDepthState::WriteOnly:
        return asdx::Preset::DepthWriteOnly;

    case viewer::MaterialDepthState::None:
        return asdx::Preset::DepthNone;
    }
}

//-----------------------------------------------------------------------------
//      ラスタライザーステート設定を取得します.
//-----------------------------------------------------------------------------
D3D12_RASTERIZER_DESC GetRasterizerDesc(viewer::MaterialRasterizerState state)
{
    switch(state)
    {
    case viewer::MaterialRasterizerState::CullNone:
    default:
        return asdx::Preset::CullNone;

    case viewer::MaterialRasterizerState::CullBack:
        return asdx::Preset::CullBack;

    case viewer::MaterialRasterizerState::CullFront:
        return asdx::Preset::CullFront;

    case viewer::MaterialRasterizerState::Wireframe:
        return asdx::Preset::Wireframe;
    }
}

} // namespace


///////////////////////////////////////////////////////////////////////////////
// ModelViewer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ModelViewer::ModelViewer()
: asdx::App(L"ModelViewer", 1920, 1080, nullptr, nullptr, nullptr)
, m_ModelTranslation(0.0f, 0.0f, 0.0f)
, m_ModelRotation   (0.0f, 0.0f, 0.0f)
, m_ModelScale      (1.0f, 1.0f, 1.0f)
{
    m_SwapChainFormat    = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    m_DepthStencilFormat = DXGI_FORMAT_D32_FLOAT;

    m_ClearColor[0] = 0.2f;
    m_ClearColor[1] = 0.2f;
    m_ClearColor[2] = 0.2f;
    m_ClearColor[3] = 1.0f;

    m_DeviceDesc.MaxShaderResourceCount = 8192;
    m_DeviceDesc.MaxSamplerCount        = 128;
    m_DeviceDesc.MaxColorTargetCount    = 256;
    m_DeviceDesc.MaxDepthTargetCount    = 256;

#if ASDX_DEBUG
    m_DeviceDesc.EnableDebug   = true;
    m_DeviceDesc.EnableCapture = true;
#endif
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ModelViewer::~ModelViewer()
{
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ModelViewer::OnInit()
{
    auto pDevice = asdx::GetD3D12Device();

    m_GfxCmdList.Reset();
    auto pCmd = m_GfxCmdList.GetD3D12CommandList();

    // GUIマネージャの初期化処理.
    {
        if (!asdx::GuiMgr::Instance().Init(pCmd, m_hWnd, m_Width, m_Height, m_SwapChainFormat, nullptr))
        {
            ELOG("Error : GuiMgr::Init() Failed.");
            return false;
        }
    }

    // ルートシグニチャの生成.
    {
        D3D12_ROOT_PARAMETER params[3] = {};
        asdx::InitAsCBV(params[0], 0, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsConstants(params[1], 1, 4, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[2], 0, D3D12_SHADER_VISIBILITY_ALL);

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters      = _countof(params);
        desc.pParameters        = params;
        desc.NumStaticSamplers  = _countof(asdx::Preset::StaticSamplers);
        desc.pStaticSamplers    = asdx::Preset::StaticSamplers;
        desc.Flags              = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        if (!asdx::InitRootSignature(pDevice, &desc, m_RootSignature.GetAddress()))
        {
            ELOGA("Error : InitRootSignature() Failed.");
            return false;
        }
    }

    // モデルルートシグニチャの生成.
    {
        D3D12_ROOT_PARAMETER params[19] = {};
        asdx::InitAsCBV(params[MODEL_ROOT_PARAM_CBV0], 0, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsCBV(params[MODEL_ROOT_PARAM_CBV1], 1, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsConstants(params[MODEL_ROOT_PARAM_CONSTANTS], 2, 4, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[MODEL_ROOT_PARAM_SRV0], 0, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[MODEL_ROOT_PARAM_SRV1], 1, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[MODEL_ROOT_PARAM_SRV2], 2, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[MODEL_ROOT_PARAM_SRV3], 3, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[MODEL_ROOT_PARAM_SRV4], 4, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[MODEL_ROOT_PARAM_SRV5], 5, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[MODEL_ROOT_PARAM_SRV6], 6, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[MODEL_ROOT_PARAM_SRV7], 7, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[MODEL_ROOT_PARAM_SRV8], 8, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[MODEL_ROOT_PARAM_SRV9], 9, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[MODEL_ROOT_PARAM_SRV10], 10, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[MODEL_ROOT_PARAM_SRV11], 11, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[MODEL_ROOT_PARAM_SRV12], 12, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[MODEL_ROOT_PARAM_SRV13], 13, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[MODEL_ROOT_PARAM_SRV14], 14, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[MODEL_ROOT_PARAM_SRV15], 15, D3D12_SHADER_VISIBILITY_ALL);

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters      = _countof(params);
        desc.pParameters        = params;
        desc.NumStaticSamplers  = _countof(asdx::Preset::StaticSamplers);
        desc.pStaticSamplers    = asdx::Preset::StaticSamplers;
        desc.Flags              = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        if (!asdx::InitRootSignature(pDevice, &desc, m_ModelRootSignature.GetAddress()))
        {
            ELOGA("Error : InitRootSignature() Failed.");
            return false;
        }
    }

    // パイプラインステートの生成,
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature                 = m_RootSignature.GetPtr();
        desc.VS                             = { MeshVS, sizeof(MeshVS) };
        desc.PS                             = { MeshPS, sizeof(MeshPS) };
        desc.BlendState                     = asdx::Preset::Opaque;
        desc.SampleMask                     = D3D12_DEFAULT_SAMPLE_MASK;
        desc.RasterizerState                = asdx::Preset::CullBack;
        desc.DepthStencilState              = asdx::Preset::DepthReadWrite;
        desc.InputLayout.NumElements        = kStaticMeshElementCount;
        desc.InputLayout.pInputElementDescs = InputElements;
        desc.PrimitiveTopologyType          = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets               = 1;
        desc.RTVFormats[0]                  = m_SwapChainFormat;
        desc.DSVFormat                      = m_DepthStencilFormat;
        desc.SampleDesc.Count               = 1;
        desc.SampleDesc.Quality             = 0;

        if (!m_DefaultState.StaticModel.Init(&desc))
        {
            ELOGA("Error : PipelineStateManager::Create() Failed.");
            return false;
        }

        desc.RasterizerState = asdx::Preset::Wireframe;
        if (!m_WireframeState.StaticModel.Init(&desc))
        {
            ELOGA("Error : PipelineStateManager::Create() Failed.");
            return false;
        }
    }

    // パイプラインステートの生成,
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature                 = m_RootSignature.GetPtr();
        desc.VS                             = { ModelVS, sizeof(ModelVS) };
        desc.PS                             = { ModelPS, sizeof(ModelPS) };
        desc.BlendState                     = asdx::Preset::Opaque;
        desc.SampleMask                     = D3D12_DEFAULT_SAMPLE_MASK;
        desc.RasterizerState                = asdx::Preset::CullBack;
        desc.DepthStencilState              = asdx::Preset::DepthReadWrite;
        desc.InputLayout.NumElements        = kSkeletalMeshElementCount;
        desc.InputLayout.pInputElementDescs = InputElements;
        desc.PrimitiveTopologyType          = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets               = 1;
        desc.RTVFormats[0]                  = m_SwapChainFormat;
        desc.DSVFormat                      = m_DepthStencilFormat;
        desc.SampleDesc.Count               = 1;
        desc.SampleDesc.Quality             = 0;

        if (!m_DefaultState.SkeletalModel.Init(&desc))
        {
            ELOGA("Error : PipelineStateManager::Create() Failed.");
            return false;
        }

        desc.RasterizerState = asdx::Preset::Wireframe;
        if (!m_WireframeState.SkeletalModel.Init(&desc))
        {
            ELOGA("Error : PipelineStateManager::Create() Failed.");
            return false;
        }
    }

    for(auto i=0; i<2; ++i)
    {
        if (!m_SceneCB[i].Init(sizeof(ParamScene)))
        {
            ELOG("Error : ConstantBuffer::Init() Failed.");
            return false;
        }
    }

    m_Camera.Init(
        asdx::Vector3(0.0f, 0.0f, 5.0f),
        asdx::Vector3(0.0f, 0.0f, 0.0f),
        asdx::Vector3(0.0f, 1.0f, 0.0f),
        0.1f,
        10000.0f);

    if (!m_ShapeStates.Init(m_SwapChainFormat, m_DepthStencilFormat))
    {
        ELOG("Error : ShapeStates::Init() Failed.");
        return false;
    }

    if (!m_ShapeParams.Init(UINT16_MAX))
    {
        ELOG("Error : ShapeParams::Init() Failed.");
        return false;
    }

    if (!m_BoneShape.Init(1.0f, 0.25f))
    {
        ELOG("Error : BoneShape::Init() Failed.");
        return false;
    }

    if (!m_SphereShape.Init(1.0f, 20))
    {
        ELOG("Error : SphereShape::Init() Failed.");
        return false;
    }

    if (!m_LineRenderer.Init(12 * UINT16_MAX, m_SwapChainFormat, DXGI_FORMAT_UNKNOWN))
    {
        ELOG("Error : LineRenderer::Init() Failed.");
        return false;
    }

    // コマンドの記録を終了.
    pCmd->Close();

    // セットアップコマンド実行.
    {
        ID3D12CommandList* pCmds[] = { pCmd };

        // グラフィックスキューを取得.
        auto pGraphicsQueue = asdx::GetGraphicsQueue();

        // コマンドを実行.
        pGraphicsQueue->Execute(_countof(pCmds), pCmds);

        // 待機点を発行.
        m_FrameWaitPoint = pGraphicsQueue->Signal();

        // 完了を待機.
        pGraphicsQueue->Sync(m_FrameWaitPoint);
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ModelViewer::OnTerm()
{
    m_LineRenderer.Term();
    m_MotionPlayer.Term();

    m_SphereShape.Term();
    m_BoneShape  .Term();
    m_ShapeParams.Term();
    m_ShapeStates.Term();

    for(auto i=0; i<2; ++i)
    {
        m_SceneCB[i].Term();
        m_MatrixPalletBuffer[i].Term();
    }

    m_DefaultState  .Reset();
    m_WireframeState.Reset();

    m_RootSignature.Reset();
    m_ModelRootSignature.Reset();

    m_Model.Reset();

    // GUIマネージャの終了処理.
    asdx::GuiMgr::Instance().Term();
}

//-----------------------------------------------------------------------------
//      フレーム遷移処理です.
//-----------------------------------------------------------------------------
void ModelViewer::OnFrameMove(const asdx::App::FrameEventArgs& args)
{
    auto pCmd = m_GfxCmdList.Reset();

    auto root = m_MotionBinary.GetRootTransform();
    m_MotionPlayer.Update(float(args.ElapsedTimeSec), root);

    asdx::GuiMgr::Instance().Update(m_Width, m_Height);

    m_LineRenderer.Reset();

    ImGuizmo::BeginFrame();
    ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());

    // 情報表示.
    DrawModelInfo();
 
    // ギズモを描画.
    auto modelWorld = asdx::Matrix::CreateIdentity();
    DrawGizmo(modelWorld);

    // コンテキストメニューを描画.
    DrawContextMenu(pCmd);

    // バウンディングスフィア描画.
    DrawBoundingSphere(modelWorld);

    // ボーン描画.
    DrawBones(modelWorld);

    // プロパティウィンドウを描画.
    DrawPropertyWindow();

    // ライセンスポップアップを描画.
    DrawLisence();

    // 射影行列を計算.
    constexpr auto fov = asdx::ToRadian(37.5f);
    const auto aspect = float(m_Width) / float(m_Height);
    m_Proj = asdx::Matrix::CreatePerspectiveFieldOfView(fov, aspect, m_Camera.GetNearClip(), m_Camera.GetFarClip());

    // シェイプ用のカメラ行列を設定.
    m_ShapeStates.SetViewProj(m_Camera.GetView(), m_Proj);

    // 定数バッファを更新.
    {
        auto idx = GetCurrentBackBufferIndex();

        auto param = m_SceneCB[idx].MapAs<ParamScene>();
        assert(param != nullptr);

        param->World        = modelWorld;
        param->View         = m_Camera.GetView();
        param->Proj         = m_Proj;
        param->CameraPos    = m_Camera.GetPosition();
        param->FieldOfView  = fov;
        param->NearClip     = m_Camera.GetNearClip();
        param->FarClip      = m_Camera.GetFarClip();
        param->TargetWidth  = float(m_Width);
        param->TargetHeight = float(m_Height);

        m_SceneCB[idx].Unmap();
    }

    // 行列パレットバッファ更新
    {
        auto isSkeletal = (m_MotionBinary.GetClipCount() > 0) && (m_Model->GetBoneCount() > 0);
        if (isSkeletal)
        {
            auto idx = GetCurrentBackBufferIndex();

            if (m_MatrixPalletBuffer[idx].GetResource() != nullptr)
            {
                auto& mtx = m_MotionPlayer.GetMatrixPalettes();
                auto  ptr = m_MatrixPalletBuffer[idx].Map();
                memcpy(ptr, mtx.data(), sizeof(asdx::Matrix) * mtx.size());
                m_MatrixPalletBuffer[idx].Unmap();
            }

        }
    }
}

//-----------------------------------------------------------------------------
//      フレーム描画処理です.
//-----------------------------------------------------------------------------
void ModelViewer::OnFrameRender(const asdx::App::FrameEventArgs& args)
{
    auto idx = GetCurrentBackBufferIndex();

    auto pCmd = m_GfxCmdList.GetD3D12CommandList();

    // 書き込み状態に遷移.
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource    = m_ColorTarget[idx].GetResource();
        barrier.Transition.StateBefore  = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter   = D3D12_RESOURCE_STATE_RENDER_TARGET;
        pCmd->ResourceBarrier(1, &barrier);
    }

    // レンダーターゲット設定.
    auto handleRTV = m_ColorTarget[idx].GetCpuHandleRTV();
    auto handleDSV = m_DepthTarget.GetCpuHandleDSV();
    pCmd->OMSetRenderTargets(1, &handleRTV, FALSE, &handleDSV);
    pCmd->ClearRenderTargetView(handleRTV, m_ClearColor, 0, nullptr);
    pCmd->ClearDepthStencilView(handleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    pCmd->RSSetViewports(1, &m_Viewport);
    pCmd->RSSetScissorRects(1, &m_ScissorRect);

    // モデルを描画
    if (m_ModelInfo.MeshCount > 0)
    {
        auto addressParam = m_ShapeParams.Update();

        pCmd->SetGraphicsRootSignature(m_RootSignature.GetPtr());

        auto isSkeletal = (m_MotionBinary.GetClipCount() > 0) 
            && (m_Model->GetBoneCount() > 0)
            && (m_MatrixPalletBuffer[idx].GetResource() != nullptr);

        if (isSkeletal)
        {
            if (m_EnableWireframe)
            { m_WireframeState.SkeletalModel.SetState(pCmd); }
            else
            { m_DefaultState.SkeletalModel.SetState(pCmd); }

            pCmd->SetGraphicsRootShaderResourceView(ROOT_PARAM_T0, m_MatrixPalletBuffer[idx].GetGpuAddress());
        }
        else
        {
            if (m_EnableWireframe)
            { m_WireframeState.StaticModel.SetState(pCmd); }
            else
            { m_DefaultState.StaticModel.SetState(pCmd); }
        }

        pCmd->SetGraphicsRootConstantBufferView(ROOT_PARAM_B0, m_SceneCB[idx].GetGpuAddress());
        pCmd->SetGraphicsRoot32BitConstants(ROOT_PARAM_B1, 1, &m_DrawMode, 0);
        pCmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        for(auto i=0u; i<m_Model->GetMeshCount(); ++i)
        {
            const auto mesh = m_Model->GetMesh(i);

            D3D12_VERTEX_BUFFER_VIEW VBVs[] = {
                mesh->GetPositions  ().GetVBV(),
                mesh->GetNormals    ().GetVBV(),
                mesh->GetTangents   ().GetVBV(),
                mesh->GetTexCoords  ().GetVBV(),
                mesh->GetColors     ().GetVBV(),
                mesh->GetBoneIndices().GetVBV(),
                mesh->GetBoneWeights().GetVBV(),
            };

            auto IBV = mesh->GetIndices().GetIBV();
            auto countVBV = (isSkeletal) ? kSkeletalMeshElementCount : kStaticMeshElementCount;

            pCmd->IASetVertexBuffers(0, countVBV, VBVs);
            pCmd->IASetIndexBuffer(&IBV);

            pCmd->DrawIndexedInstanced(mesh->GetIndexCount(), 1, 0, 0, 0);
        }

        if (m_DrawBoundingSphere)
        {
            m_ShapeStates.ApplyTranslucentState(pCmd);
            pCmd->SetGraphicsRootShaderResourceView(asdx::ShapeStates::SRV0, addressParam);

            for(size_t i=0; i<m_Model->GetMeshCount(); ++i)
            {
                uint32_t index = uint32_t(i);
                pCmd->SetGraphicsRoot32BitConstant(asdx::ShapeStates::Constants3, index, 0);
                m_SphereShape.Draw(pCmd);
            }

            {
                uint32_t index = uint32_t(m_Model->GetMeshCount());
                pCmd->SetGraphicsRoot32BitConstant(asdx::ShapeStates::Constants3, index, 0);
                m_SphereShape.Draw(pCmd);
            }
        }

        if (m_DrawBones)
        {
            auto addr = m_ShapeStates.GetBufferAddress();
            m_LineRenderer.SetPipelineState(pCmd);
            pCmd->SetGraphicsRootConstantBufferView(0, addr);
            m_LineRenderer.Draw(pCmd);
        }
    }

    // GUIを描画.
    asdx::GuiMgr::Instance().Draw(pCmd);

    // 表示状態に遷移.
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource    = m_ColorTarget[idx].GetResource();
        barrier.Transition.StateBefore  = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter   = D3D12_RESOURCE_STATE_PRESENT;
        pCmd->ResourceBarrier(1, &barrier);
    }

    // コマンド記録終了.
    pCmd->Close();

    // コマンドを実行.
    {
        ID3D12CommandList* pCmds[] = { pCmd };

        auto pGraphicsQueue = asdx::GetGraphicsQueue();

        // 前フレームの描画の完了を待機.
        if (m_FrameWaitPoint.IsValid())
        { pGraphicsQueue->Sync(m_FrameWaitPoint); }

        // コマンドを実行.
        pGraphicsQueue->Execute(_countof(pCmds), pCmds);

        // 待機点を発行.
        m_FrameWaitPoint = pGraphicsQueue->Signal();
    }

    Present(1);
}

//-----------------------------------------------------------------------------
//      リサイズ処理です.
//-----------------------------------------------------------------------------
void ModelViewer::OnResize(const asdx::App::ResizeEventArgs& args)
{
}

//-----------------------------------------------------------------------------
//      キー処理です.
//-----------------------------------------------------------------------------
void ModelViewer::OnKey(const asdx::App::KeyEventArgs& args)
{
    asdx::GuiMgr::Instance().OnKey(args.KeyCode, args.IsKeyDown, args.IsAltDown);

    m_Camera.OnKey(args.KeyCode, args.IsKeyDown, args.IsAltDown);

    if (args.IsKeyDown)
    {
        switch(args.KeyCode)
        {
        // 移動ツール.
        case 'W':
            {
                if (m_EnableGuizmo && m_GuizmoOperation == ImGuizmo::OPERATION::TRANSLATE)
                { m_EnableGuizmo = false; }
                else
                {
                    m_EnableGuizmo    = true;
                    m_GuizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
                }
            }
            break;

        // 回転ツール.
        case 'E':
            {
                if (m_EnableGuizmo && m_GuizmoOperation == ImGuizmo::OPERATION::ROTATE)
                { m_EnableGuizmo = false; }
                else
                {
                    m_EnableGuizmo    = true;
                    m_GuizmoOperation = ImGuizmo::OPERATION::ROTATE;
                }
            }
            break;

        // スケールツール.
        case 'R':
            {
                if (m_EnableGuizmo && m_GuizmoOperation == ImGuizmo::OPERATION::SCALE)
                { m_EnableGuizmo = false; }
                else
                {
                    m_EnableGuizmo    = true;
                    m_GuizmoOperation = ImGuizmo::OPERATION::SCALE;
                }
            }
            break;

        // ワールド行列リセット.
        case 'Z':
            {
                m_ModelTranslation  = asdx::Vector3(0.0f, 0.0f, 0.0f);
                m_ModelRotation     = asdx::Vector3(0.0f, 0.0f, 0.0f);
                m_ModelScale        = asdx::Vector3(1.0f, 1.0f, 1.0f);
            }
            break;

        // ワイヤーフレーム.
        case '4':
            {
                m_EnableWireframe = true;
            }
            break;

        // シェーディング
        case '5':
            {
                m_EnableWireframe = false;
            }
            break;

        default:
            break;
        }
    }
}

//-----------------------------------------------------------------------------
//      マウス処理です.
//-----------------------------------------------------------------------------
void ModelViewer::OnMouse(const asdx::App::MouseEventArgs& args)
{
    if (args.IsAltDown)
    {
        m_Camera.OnMouse(
            args.X,
            args.Y,
            args.WheelDelta,
            args.IsDownL,
            args.IsDownR,
            args.IsDownM,
            args.IsDownX1,
            args.IsDownX2);
    }
    else
    {
        asdx::GuiMgr::Instance().OnMouse(
            args.X, args.Y, args.WheelDelta, args.IsDownL, args.IsDownM, args.IsDownR);
    }
}

//-----------------------------------------------------------------------------
//      タイピング処理です.
//-----------------------------------------------------------------------------
void ModelViewer::OnTyping(uint32_t keyCode)
{
    asdx::GuiMgr::Instance().OnTyping(keyCode);
}

//-----------------------------------------------------------------------------
//      ファイルドロップ時の処理です.
//-----------------------------------------------------------------------------
void ModelViewer::OnDrop(const wchar_t** dropFiles, uint32_t fileCount)
{
    if (dropFiles == nullptr || fileCount == 0)
        return;

    // 最初の1つだけ処理する.
    auto path = asdx::fs::path(dropFiles[0]);
    std::vector<uint8_t> modelBinary;
    if (path.extension().string() == ".mdb")
    {
        if (asdx::LoadA(path.string().c_str(), modelBinary))
        {
            m_ModelBinary = std::move(modelBinary);
            RecreateModel();
        }
    }
    else
    {
        if (ModelConverter::Convert(path.string().c_str(), modelBinary))
        {
            m_ModelBinary = std::move(modelBinary);
            RecreateModel();
        }
    }
}

//-----------------------------------------------------------------------------
//      ファイルメニュー処理です.
//-----------------------------------------------------------------------------
void ModelViewer::MenuFile(ID3D12GraphicsCommandList* pCmd)
{
    // モデルファイルを開く.
    if (ImGui::MenuItem(asdx::ToChar(u8"モデルファイルを開く")))
    { LoadModel(); }

    if (!m_ModelBinary.empty())
    {
        if (ImGui::MenuItem(asdx::ToChar(u8"名前を付けてモデルを保存")))
        {
            const char* filter = "Project Asura Model Binary (*.mdb)\0*.mdb\0\0";
            std::string base;
            std::string ext = ".mdb";

            asdx::fs::path path;
            if (asdx::SaveFileDlg(filter, path))
            {
                m_ModelOutputPath = path.string();
                SaveModelBinary(m_ModelOutputPath.c_str());
            }
        }

        if (!m_ModelOutputPath.empty())
        {
            if (ImGui::MenuItem(asdx::ToChar(u8"モデルを上書き保存")))
            { SaveModelBinary(m_ModelOutputPath.c_str()); }
        }
    }

    ImGui::Separator();

    // プレハブを開く.
    if (ImGui::MenuItem(asdx::ToChar(u8"プレハブファイルを開く")))
    { LoadPrefab(); }

    if (!m_ModelBinary.empty())
    {
        if (ImGui::MenuItem(asdx::ToChar(u8"名前を付けてプレハブを保存")))
        {
            const char* filter = "Project Asura ModelPrefab Binary (*.mpb)\0*.mpb\0\0";
            std::string base;
            std::string ext = ".mpb";

            asdx::fs::path path;
            if (asdx::SaveFileDlg(filter, path))
            {
                m_PrefabOutputPath = path.string();
                SavePrefabBinary(m_PrefabOutputPath.c_str());
            }
        }

        if (!m_PrefabOutputPath.empty())
        {
            if (ImGui::MenuItem(asdx::ToChar(u8"プレハブを上書き保存")))
            { SavePrefabBinary(m_PrefabOutputPath.c_str()); }
        }
    }

    ImGui::Separator();

    // モーションファイルを開く.
    if (ImGui::MenuItem(asdx::ToChar(u8"モーションファイルを開く")))
    { LoadMotion(); }


#if 0
    //if (ImGui::MenuItem(asdx::ToChar(u8"マテリアルスキーマファイルを開く")))
    //{
    //    asdx::fs::path path;
    //    if (asdx::OpenFileDlg("JSONファイル (*.json)\0*.json\0\0", path))
    //    { LoadMaterialSchema(path.string().c_str()); }
    //}
#endif
}

//-----------------------------------------------------------------------------
//      ヘルプメニュー処理です.
//-----------------------------------------------------------------------------
void ModelViewer::MenuHelp()
{
    // バージョン情報.
    if (ImGui::MenuItem(asdx::ToChar(u8"バージョン情報")))
    {
        asdx::InfoDlg("Version Info",
            "ModelViewer ver 0.1\n"
            "Build 0.1\n"
            "Copyright(c) Project Asura.");
    }

    // ライセンス情報.
    if (ImGui::MenuItem(asdx::ToChar(u8"ライセンス情報")))
    {
        m_ShowLisence = true;
    }
}

//-----------------------------------------------------------------------------
//      モデルを再生成します.
//-----------------------------------------------------------------------------
void ModelViewer::RecreateModel()
{
    asdx::Model* pModel = nullptr;

    // モデルを生成をします.
    std::vector<uint8_t> copyBinary = m_ModelBinary;
    if (!asdx::Model::Create(std::move(copyBinary), &pModel))
    {
        ELOGA("Error : ModelManager::CreateModel() Failed.");
        return;
    }

    m_MotionPlayer.SetClip(nullptr);
    m_ClipIndex = 0;
    m_ClipNames.clear();
    m_MotionBinary.Term();

    // 成功したら差し替え.
    m_Model.Attach(pModel);

    m_ModelInfo.MeshCount     = m_Model->GetMeshCount();
    m_ModelInfo.MaterialCount = m_Model->GetMaterialCount();
    m_ModelInfo.BoneCount     = m_Model->GetBoneCount();

    m_ModelInfo.VertexCount = 0;
    m_ModelInfo.IndexCount  = 0;

    for(auto i=0u; i<m_Model->GetMeshCount(); ++i)
    {
        auto mesh = m_Model->GetMesh(i);
        m_ModelInfo.VertexCount += mesh->GetVertexCount();
        m_ModelInfo.IndexCount  += mesh->GetIndexCount();
    }

    auto sphere = m_Model->GetBoundingSphere();

    // カメラを初期化.
    m_Camera.Init(
        asdx::Vector3(0.0f, 0.0f, sphere.Radius * 3.0f),
        asdx::Vector3(0.0f, 0.0f, 0.0f),
        asdx::Vector3(0.0f, 0.0f, 1.0f),
        0.1f,
        10000.0f);
    m_Camera.Present();

    // モーションプレイヤーを初期化.
    m_MotionPlayer.Init(pModel);

    // 行列パレット用 SRV を生成.
    auto boneCount = m_Model->GetBoneCount();
    for(auto i=0; i<2; ++i)
    {
        m_MatrixPalletBuffer[i].Term();

        if (!m_MatrixPalletBuffer[i].Init(boneCount, sizeof(asdx::Matrix), D3D12_RESOURCE_STATE_COMMON, true))
            ELOG("Error : Matrix Pallet Buffer Init Failed. index = %u", i);
    }

    auto materialCount = m_Model->GetMaterialCount();

    // 編集可能マテリアルを初期化.
    m_EditMaterials.clear();
    m_EditMaterials.resize(materialCount);

    // モデルプレハブのマテリアルを初期化.
    m_Prefab.Materials.resize(materialCount);
    for(auto i=0u; i<materialCount; ++i)
    {
        m_Prefab.Materials[i].Name = m_Model->GetMaterialName(i);
        m_Prefab.Materials[i].Path.clear();
    }
}

//-----------------------------------------------------------------------------
//      プレハブを生成します.
//-----------------------------------------------------------------------------
void ModelViewer::RecreatePrefab()
{
    if (m_Prefab.ModelPath.empty())
        return;

    if (!asdx::LoadA(m_Prefab.ModelPath.c_str(), m_ModelBinary))
    {
        ELOG("Error : ModelBinary Load Failed. path = %s", m_Prefab.ModelPath.c_str());
        return;
    }

    asdx::Model* pModel = nullptr;

    // モデルを生成をします.
    std::vector<uint8_t> copyBinary = m_ModelBinary;
    if (!asdx::Model::Create(std::move(copyBinary), &pModel))
    {
        ELOGA("Error : ModelManager::CreateModel() Failed.");
        return;
    }

    m_MotionPlayer.SetClip(nullptr);
    m_ClipIndex = 0;
    m_ClipNames.clear();
    m_MotionBinary.Term();

    // 成功したら差し替え.
    m_Model.Attach(pModel);

    m_ModelInfo.MeshCount     = m_Model->GetMeshCount();
    m_ModelInfo.MaterialCount = m_Model->GetMaterialCount();
    m_ModelInfo.BoneCount     = m_Model->GetBoneCount();

    m_ModelInfo.VertexCount = 0;
    m_ModelInfo.IndexCount  = 0;

    for(auto i=0u; i<m_Model->GetMeshCount(); ++i)
    {
        auto mesh = m_Model->GetMesh(i);
        m_ModelInfo.VertexCount += mesh->GetVertexCount();
        m_ModelInfo.IndexCount  += mesh->GetIndexCount();
    }

    auto sphere = m_Model->GetBoundingSphere();

    // カメラを初期化.
    m_Camera.Init(
        asdx::Vector3(0.0f, 0.0f, sphere.Radius * 3.0f),
        asdx::Vector3(0.0f, 0.0f, 0.0f),
        asdx::Vector3(0.0f, 0.0f, 1.0f),
        0.1f,
        10000.0f);
    m_Camera.Present();

    // モーションプレイヤーを初期化.
    m_MotionPlayer.Init(pModel);

    // 行列パレット用 SRV を生成.
    auto boneCount = m_Model->GetBoneCount();
    for(auto i=0; i<2; ++i)
    {
        m_MatrixPalletBuffer[i].Term();

        if (!m_MatrixPalletBuffer[i].Init(boneCount, sizeof(asdx::Matrix), D3D12_RESOURCE_STATE_COMMON, true))
            ELOG("Error : Matrix Pallet Buffer Init Failed. index = %u", i);
    }

    auto materialCount = m_Model->GetMaterialCount();

    // 編集可能マテリアルを初期化.
    m_EditMaterials.clear();
    m_EditMaterials.resize(materialCount);

    // マテリアルバイナリを設定.
    for(auto i=0u; i<materialCount; ++i)
    {
        std::vector<uint8_t> matBin;
        if (!MaterialConverter::Convert(m_Prefab.Materials[i].Path.c_str(), matBin))
        {
            ELOG("Error : MaterialConvert::Convert() Failed. materialIndex = %u", i);
            continue;
        }

        if (!MaterialConverter::ReverseConvert(matBin, m_EditMaterials[i]))
        {
            ELOG("Error : MaterialConvert::ReverseConvert() Failed. materialIndex = %u", i);
            continue;
        }
    }
}

//-----------------------------------------------------------------------------
//      モデルバイナリを保存します.
//-----------------------------------------------------------------------------
void ModelViewer::SaveModelBinary(const char* path)
{
    if (path == nullptr)
        return;

    if (!asdx::SaveA(path, m_ModelBinary))
    {
        ELOG("Error : SaveA() Failed. path = %s", path);
        return;
    }

    ILOG("Info : ModelBinary Output success. path = %s", path);
}

//-----------------------------------------------------------------------------
//      プレハブバイナリを保存します.
//-----------------------------------------------------------------------------
void ModelViewer::SavePrefabBinary(const char* path)
{
    if (path == nullptr)
        return;

    // Prefabバイナリに変換.
    std::vector<uint8_t> binary;
    if (!ModelPrefabConverter::Convert(m_Prefab, binary))
    {
        ELOG("Error : ModelPrefabConverter::Convert() Failed.");
        return;
    }

    if (!asdx::SaveA(path, binary))
    {
        ELOG("Error : SaveA() Failed. path = %s", path);
        return;
    }

    ILOG("Info : ModelPrefabBinary Output success. path = %s", path);
}

//-----------------------------------------------------------------------------
//      マテリアルバイナリを保存します.
//-----------------------------------------------------------------------------
void ModelViewer::SaveMaterialBinary(const char* path, edit::Material& material)
{
    if (path == nullptr)
        return;

    std::vector<uint8_t> binary;
    if (!MaterialConverter::Convert(material, binary))
    {
        ELOG("Error : MaterialConverter::Convert() Failed.");
        return;
    }

    if (!asdx::SaveA(path, binary))
    {
        ELOG("Error : SaveA() Failed. path = %s", path);
        return;
    }

    ILOG("Info : MaterialBinary Output success. path = %s", path);
}

//-----------------------------------------------------------------------------
//      モデルファイルを読み込みます.
//-----------------------------------------------------------------------------
void ModelViewer::LoadModel()
{
    const char* filter = 
        "読み込み可能なモデルファイル\0*.mdb;*.dae;*.xml;*.blend;*.3ds;*.ase;*.gltf;*.fbx;*.ply;*.dxf;*.smd;*.vta;*.mdl;*.md2;*.md3;*.md5mesh;*.md5anim;*.x;*.obj;*.ter;*.ms3d;*.lxo;*.lwo;*.lws;*.pmd;*.pmx;*.vmd;*.usd;*.usda;*.usdc;*.usdz\0"
        "Project Asura Model Binary (*.mdb)\0*.mdb\0"
        "Blender Format (*.blend)\0*.blend\0"
        "Collada (*.dae, *.xml)\0*.dae;*.xml\0"
        "3D Studio Max 3DS (*.3ds)\0*.3ds\0"
        "3D Studio Max ASE (*.ase)\0*.ase\0"
        "GL Transmission Format (*.gltf)\0*.gltf\0"
        "Film Box (*.fbx)\0*.fbx\0"
        "Standard Polygon Library (*.ply)\0*.ply\0"
        "Autodesk DXF (*.dxf)\0*.dxf\0"
        "Valve Model (*.smd, *.vta)\0*.smd;*.vta\0"
        "Quake1 Model (*.mdl)\0*.mdl\0"
        "Quake2 Model (*.md2)\0*.md2\0"
        "Quake3 Model (*.md3, *.md4)\0*.md3, *.md4\0"
        "Doom3 Model (*.md5mesh)\0*.md5mesh;\0"
        "DirectX X File (*.x)\0*.x\0"
        "Wavefront Object (*.obj)\0*.obj\0"
        "Terragen Terrain (*.ter)\0*.ter\0"
        "Milkshape 3D (*.ms3d)\0*.ms3d\0"
        "Modo Model (*.lxo)\0*.lxo\0"
        "LightWave Model (*.lwo)\0*.lwo\0"
        "LightWave Scene (*.lws)\0*.lws\0"
        "MikuMikuDance (*.pmd, *.pmx, *.vmd)\0*.pmd;*pmx;*.vmd\0"
        "Universal Scene Description (*.usd, *.usda, *.usdc, *.usdz)\0*.usd;*.usda;*.usdc;*.usdz\0"
        "全てのファイル (*.*)\0*.*\0\0";

    asdx::fs::path path;
    if (!asdx::OpenFileDlg(filter, path))
        return;

    auto input = path.string();

    std::vector<uint8_t> modelBinary;
    if (path.extension().string() == ".mdb")
    {
        if (!asdx::LoadA(input.c_str(), modelBinary))
        {
            ELOG("Error : asdx::LoadA() Failed. path = %s", input.c_str());
            return;
        }

        m_ModelBinary = std::move(modelBinary);
        m_Prefab.ModelPath = input.c_str();
        RecreateModel();
    }
    else
    {
        if (!ModelConverter::Convert(input.c_str(), modelBinary))
        {
            ELOG("Error : ModelConverter::Convert() Failed.");
            return;
        }

        m_ModelBinary = std::move(modelBinary);
        m_Prefab.ModelPath = input.c_str();
        RecreateModel();
    }
}

//-----------------------------------------------------------------------------
//      モーションファイルを読み込みます.
//-----------------------------------------------------------------------------
void ModelViewer::LoadMotion()
{
    const char* filter = 
        "読み込み可能なモーションファイル\0*.mob;*.dae;*.xml;*.blend;*.3ds;*.ase;*.gltf;*.fbx;*.ply;*.dxf;*.smd;*.vta;*.mdl;*.md2;*.md3;*.md5anim;*.x;*.ms3d;*.lws;*.pmd;*.pmx;*.vmd;*.usd;*.usda;*.usdc;*.usdz\0"
        "Project Asura Motion Binary (*.mob)\0*.mob\0"
        "Blender Format (*.blend)\0*.blend\0"
        "Collada (*.dae, *.xml)\0*.dae;*.xml\0"
        "3D Studio Max 3DS (*.3ds)\0*.3ds\0"
        "3D Studio Max ASE (*.ase)\0*.ase\0"
        "GL Transmission Format (*.gltf)\0*.gltf\0"
        "Film Box (*.fbx)\0*.fbx\0"
        "Autodesk DXF (*.dxf)\0*.dxf\0"
        "Valve Model (*.smd, *.vta)\0*.smd;*.vta\0"
        "Quake1 Model (*.mdl)\0*.mdl\0"
        "Quake2 Model (*.md2)\0*.md2\0"
        "Quake3 Model (*.md3, *.md4)\0*.md3, *.md4\0"
        "Doom3 Animation (*.md5anim)\0*.md5anim\0"
        "DirectX X File (*.x)\0*.x\0"
        "Milkshape 3D (*.ms3d)\0*.ms3d\0"
        "Modo Model (*.lxo)\0*.lxo\0"
        "LightWave Scene (*.lws)\0*.lws\0"
        "MikuMikuDance (*.pmd, *.pmx, *.vmd)\0*.pmd;*pmx;*.vmd\0"
        "Universal Scene Description (*.usd, *.usda, *.usdc, *.usdz)\0*.usd;*.usda;*.usdc;*.usdz\0"
        "全てのファイル (*.*)\0*.*\0\0";

    asdx::fs::path path;
    if (!asdx::OpenFileDlg(filter, path))
        return;

    auto input = path.string();

    std::vector<uint8_t> motionBinary;
    if (path.extension().string() == ".mob")
    {
        if (!asdx::LoadA(input.c_str(), motionBinary))
        {
            ELOG("Error : asdx::LoadA() Failed. path = %s", input.c_str());
            return;
        }

        m_MotionBinary.Load(std::move(motionBinary));
    }
    else
    {
        if (!MotionConverter::Convert(input.c_str(), motionBinary))
        {
            ELOG("Error : MotionConverter::Convert() Failed.");
            return;
        }

        m_MotionBinary.Load(std::move(motionBinary));
    }

    auto count = m_MotionBinary.GetClipCount();
    if (count == 0)
    {
        ELOG("Error : Clip Count is zero.");
        return;
    }

    m_ClipNames.resize(count);

    for(auto i=0u; i<count; ++i)
        m_ClipNames[i] = asdx::MotionClipProxy::GetName(m_MotionBinary.GetClip(i));

    auto clip = m_MotionBinary.GetClip(0);
    m_MotionPlayer.SetClip(clip);
    m_ClipIndex = 0;
}

//-----------------------------------------------------------------------------
//      プレハブファイルを読み込みます.
//-----------------------------------------------------------------------------
void ModelViewer::LoadPrefab()
{
    const char* filter = "Project Asura ModelPrefab Binary (*.mpb)\0*.mpb\0\0";

    asdx::fs::path path;
    if (!asdx::OpenFileDlg(filter, path))
        return;

    auto input = path.string();

    std::vector<uint8_t> prefabBinary;
    if (!asdx::LoadA(input.c_str(), prefabBinary))
    {
        ELOG("Error : asdx::LoadA() Failed. path = %s", input.c_str());
        return;
    }

    ModelPrefab prefab;
    if (!ModelPrefabConverter::ReverseConvert(prefabBinary, prefab))
    {
        ELOG("Error : MaterialPrefabConverter::ReverseConvert() Failed.");
        return;
    }

    // プレハブデータを差し替え.
    m_Prefab = std::move(prefab);
}

//-----------------------------------------------------------------------------
//      マテリアルファイルを読み込みます.
//-----------------------------------------------------------------------------
void ModelViewer::LoadMaterial(const char* path, edit::Material& material)
{
    if (path == nullptr)
        return;

    std::vector<uint8_t> binary;
    if (!MaterialConverter::Convert(path, binary))
    {
        ELOG("Error : MaterialConverter::Convert() Failed.");
        return;
    }

    if (!MaterialConverter::ReverseConvert(binary, material))
    {
        ELOG("Error : MaterialConverter::ReverseConvert() Failed.");
        return;
    }
}

//-----------------------------------------------------------------------------
//      モデルパイプラインステートを生成します.
//-----------------------------------------------------------------------------
bool ModelViewer::CreateModelPipelineState
(
    D3D12_SHADER_BYTECODE           pixelShader,
    viewer::MaterialBlendState      blendState,
    viewer::MaterialDepthState      depthState,
    viewer::MaterialRasterizerState rasterizerState,
    ModelPipelineState&             result
)
{
    auto pDevice = asdx::GetD3D12Device();

    // スタティック用.
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature                 = m_ModelRootSignature.GetPtr();
        desc.VS                             = { MeshVS, sizeof(MeshVS) };
        desc.PS                             = pixelShader;
        desc.BlendState                     = GetBlendDesc(blendState);
        desc.SampleMask                     = D3D12_DEFAULT_SAMPLE_MASK;
        desc.RasterizerState                = GetRasterizerDesc(rasterizerState);
        desc.DepthStencilState              = GetdepthStencilDesc(depthState);
        desc.InputLayout.NumElements        = kStaticMeshElementCount;
        desc.InputLayout.pInputElementDescs = InputElements;
        desc.PrimitiveTopologyType          = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets               = 1;
        desc.RTVFormats[0]                  = m_SwapChainFormat;
        desc.DSVFormat                      = m_DepthStencilFormat;
        desc.SampleDesc.Count               = 1;
        desc.SampleDesc.Quality             = 0;

        if (!result.StaticModel.Init(&desc))
        {
            ELOG("Error : StaticModel PipelineState Init Failed.");
            return false;
        }
    }

    // スケルタル用.
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature                 = m_ModelRootSignature.GetPtr();
        desc.VS                             = { ModelVS, sizeof(ModelVS) };
        desc.PS                             = pixelShader;
        desc.BlendState                     = GetBlendDesc(blendState);
        desc.SampleMask                     = D3D12_DEFAULT_SAMPLE_MASK;
        desc.RasterizerState                = GetRasterizerDesc(rasterizerState);
        desc.DepthStencilState              = GetdepthStencilDesc(depthState);
        desc.InputLayout.NumElements        = kSkeletalMeshElementCount;
        desc.InputLayout.pInputElementDescs = InputElements;
        desc.PrimitiveTopologyType          = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets               = 1;
        desc.RTVFormats[0]                  = m_SwapChainFormat;
        desc.DSVFormat                      = m_DepthStencilFormat;
        desc.SampleDesc.Count               = 1;
        desc.SampleDesc.Quality             = 0;

        if (!result.SkeletalModel.Init(&desc))
        {
            ELOG("Error : SkeletalModel PipelineState Init Failed.");
            return false;
        }
    }

    return true;
}