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
#include <assimp/Exporter.hpp>


namespace {

//-----------------------------------------------------------------------------
// Shaders
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/ModelVS.inc"
#include "../res/shaders/Compiled/ModelPS.inc"

enum ROOT_PARAM
{
    ROOT_PARAM_B0,
    ROOT_PARAM_B1,
    ROOT_PARAM_T0,  // BaseColor
    ROOT_PARAM_T1,  // Normal
    ROOT_PARAM_T2,  // ORM
    ROOT_PARAM_T3,  // Emissive.
};

static const D3D12_INPUT_ELEMENT_DESC InputElements[] = {
    { "POSITION"   , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "NORMAL"     , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TANGENT"    , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD"   , 0, DXGI_FORMAT_R32G32_FLOAT      , 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "COLOR"      , 0, DXGI_FORMAT_R8G8B8A8_UNORM    , 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    //{ "BLENDINDEX" , 0, DXGI_FORMAT_R32G32B32A32_UINT , 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    //{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

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

const char* kDrawModes[] = {
    asdx::ToChar(u8"デフォルト"),
    asdx::ToChar(u8"スクリーン空間位置座標"),
    asdx::ToChar(u8"法線ベクトル"),
    asdx::ToChar(u8"接線ベクトル"),
    asdx::ToChar(u8"従接線ベクトル"),
    asdx::ToChar(u8"テクスチャ座標"),
    asdx::ToChar(u8"頂点カラー"),
};

//-----------------------------------------------------------------------------
//      コンボボックスを描画します.
//-----------------------------------------------------------------------------
static bool ImGuiCombo(const char* caption, size_t& index, const std::vector<std::string>& items)
{
    if (items.empty())
    {
        int idx = 0;
        ImGui::Combo(caption, &idx, "None");
        return false;
    }

    if (!ImGui::BeginCombo(caption, items[index].c_str())) 
        return false;

    assert(index < items.size());
    bool changed = false;
    for (size_t n=0; n<items.size(); n++)
    {
        auto selected = (n == index);
        if (ImGui::Selectable(items[n].c_str(), selected))
        {
            index   = n;
            changed = true;
        }
        if (selected)
        {
            ImGui::SetItemDefaultFocus();
        }
    }
    ImGui::EndCombo();

    return changed;
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
        D3D12_ROOT_PARAMETER params[2] = {};
        asdx::InitAsCBV(params[0], 0, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsConstants(params[1], 1, 4, D3D12_SHADER_VISIBILITY_ALL);

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
        desc.InputLayout.NumElements        = _countof(InputElements);
        desc.InputLayout.pInputElementDescs = InputElements;
        desc.PrimitiveTopologyType          = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets               = 1;
        desc.RTVFormats[0]                  = m_SwapChainFormat;
        desc.DSVFormat                      = m_DepthStencilFormat;
        desc.SampleDesc.Count               = 1;
        desc.SampleDesc.Quality             = 0;

        if (!m_SolidState.Init(&desc))
        {
            ELOGA("Error : PipelineStateManager::Create() Failed.");
            return false;
        }

        desc.RasterizerState = asdx::Preset::Wireframe;
        if (!m_WireframeState.Init(&desc))
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
    }

    m_SolidState.Term();
    m_WireframeState.Term();

    m_RootSignature.Reset();

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
    {
        const auto w = 200.0f;
        const auto h = 175.0f;
        const auto x = 10.0f;
        const auto y = 10.0f;

        ImGui::SetNextWindowPos(ImVec2(m_Width - (w + x), m_Height - (h + y)));
        ImGui::SetNextWindowSize(ImVec2(w, h));

        auto flags = ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoTitleBar;
        if (ImGui::Begin("Info", nullptr, flags))
        {
            ImGui::Text("FPS : %.2f", GetFPS());
            ImGui::Separator();

            ImGui::BeginTable(asdx::ToChar(u8"モデル情報"), 2);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(asdx::ToChar(u8"メッシュ数"));
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(asdx::ToChar(u8"%zu"), m_ModelInfo.MeshCount);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(asdx::ToChar(u8"マテリアル数"));
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(asdx::ToChar(u8"%zu"), m_ModelInfo.MaterialCount);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(asdx::ToChar(u8"ボーン数"));
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(asdx::ToChar(u8"%zu"), m_ModelInfo.BoneCount);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(asdx::ToChar(u8"頂点数"));
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(asdx::ToChar(u8"%zu"), m_ModelInfo.VertexCount);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(asdx::ToChar(u8"インデックス数"));
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(asdx::ToChar(u8"%zu"), m_ModelInfo.IndexCount);

            ImGui::EndTable();

            ImGui::Separator();

            ImGui::BeginTable(asdx::ToChar(u8"モーション情報"), 2);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(asdx::ToChar(u8"クリップ数"));
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(asdx::ToChar(u8"%u"), m_MotionBinary.GetClipCount());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(asdx::ToChar(u8"所要時間"));
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(asdx::ToChar(u8"%.2f"), m_MotionPlayer.GetDuration());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(asdx::ToChar(u8"再生時間"));
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(asdx::ToChar(u8"%.2f"), m_MotionPlayer.GetTimeInTicks());

            ImGui::EndTable();

            ImGui::End();
        }
    }

    // モーション制御.
    {
        const auto w = 300.0f;
        const auto h = 135.0f;
        const auto x = 10.0f;
        const auto y = 10.0f;

        ImGui::SetNextWindowPos(ImVec2(x, m_Height - (h + y)));
        ImGui::SetNextWindowSize(ImVec2(w, h));

        auto flags = ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoTitleBar;
        if (ImGui::Begin("MotionControl", nullptr, flags))
        {
            if (ImGuiCombo(asdx::ToChar(u8"再生クリップ"), m_ClipIndex, m_ClipNames))
            {
                // クリップを差し替え.
                m_MotionPlayer.SetClip(m_MotionBinary.GetClip(uint32_t(m_ClipIndex)));
            }

            if (ImGui::Button(asdx::ToChar(u8"再生")))
            {
                m_MotionPlayer.SetPause(false);
            }
            ImGui::SameLine();
            if (ImGui::Button(asdx::ToChar(u8"停止")))
            {
                m_MotionPlayer.SetPause(true);
            }
            ImGui::SameLine();
            if (ImGui::Button(asdx::ToChar(u8"コマ送り")))
            {
            }
            ImGui::SameLine();
            if (ImGui::Button(asdx::ToChar(u8"頭出し")))
            {
                m_MotionPlayer.Cue();
            }
            ImGui::SameLine();
            if (ImGui::Button(asdx::ToChar(u8"削除")))
            {
                m_MotionPlayer.SetClip(nullptr);
                m_ClipIndex = 0;
                m_ClipNames.clear();
                m_MotionBinary.Term();
            }

            auto loop = m_MotionPlayer.IsLoop();
            if (ImGui::Checkbox(asdx::ToChar(u8"ループ再生"), &loop))
            {
                m_MotionPlayer.SetLoop(loop);
            }
            auto speed = m_MotionPlayer.GetPlaySpeed();
            if (ImGui::DragFloat(asdx::ToChar(u8"再生速度"), &speed, 0.1f))
            {
                m_MotionPlayer.SetPlaySpeed(speed);
            }

            ImGui::End();
        }
    }

    // コンテキストメニュー.
    {
        if (ImGui::IsMouseClicked(1))
        { ImGui::OpenPopup("ContextMenu"); }

        if (ImGui::BeginPopup("ContextMenu"))
        {
            if (ImGui::BeginMenu(asdx::ToChar(u8"ファイル")))
            {
                MenuFile(pCmd);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(asdx::ToChar(u8"表示")))
            {
                MenuView();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(asdx::ToChar(u8"ヘルプ")))
            {
                MenuHelp();
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }
    }

    asdx::Matrix modelWorld = asdx::Matrix::CreateIdentity();

    constexpr auto fov = asdx::ToRadian(37.5f);
    auto aspect = float(m_Width) / float(m_Height);
    m_Proj = asdx::Matrix::CreatePerspectiveFieldOfView(fov, aspect, m_Camera.GetNearClip(), m_Camera.GetFarClip());

    if (m_ModelInfo.MeshCount > 0)
    {
        // ギズモ表示.
        if (m_EnableGuizmo)
        {
            auto& view = m_Camera.GetView();
            auto& proj = m_Proj;
            auto& io = ImGui::GetIO();

            float matrix[16] = {};
            ImGuizmo::RecomposeMatrixFromComponents(&m_ModelTranslation.x, &m_ModelRotation.x, &m_ModelScale.x, matrix);
            ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
            if (ImGuizmo::Manipulate(&view._11, &proj._11, m_GuizmoOperation, ImGuizmo::MODE::LOCAL, matrix))
            {
                ImGuizmo::DecomposeMatrixToComponents(
                    matrix, &m_ModelTranslation.x, &m_ModelRotation.x, &m_ModelScale.x);
            }
        }

        // バウンディングスフィア表示.
        if (m_DrawBoundingSphere)
        {
            for(auto i=0u; i<m_Model->GetMeshCount(); ++i)
            {
                auto& sphere = m_Model->GetMesh(i)->GetBoundingSphere();
                auto world = asdx::Matrix::CreateScale(sphere.Radius) * asdx::Matrix::CreateTranslation(sphere.Center);

                uint32_t index = uint32_t(i);
                m_ShapeParams.SetWorld(index, world);
                m_ShapeParams.SetColor(index, asdx::Vector4(0.0f, 1.0f, 0.0f, 0.1f));
            }

            {
                auto& sphere = m_Model->GetBoundingSphere();
                auto world = asdx::Matrix::CreateScale(sphere.Radius) * asdx::Matrix::CreateTranslation(sphere.Center);

                uint32_t index = uint32_t(m_Model->GetMeshCount());
                m_ShapeParams.SetWorld(index, world);
                m_ShapeParams.SetColor(index, asdx::Vector4(1.0f, 1.0f, 0.0f, 0.1f));
            }
        }

        // ボーン表示.
        if (m_DrawBones)
        {
            auto count = m_Model->GetBoneCount();
            // モーションがある場合はアニメーション後のボーンを表示
            if (m_MotionBinary.GetClipCount() > 0)
            {
                auto matrices = m_MotionPlayer.GetWorldTransforms();
                for(auto i=0u; i<count; ++i)
                {
                    auto& bone = m_Model->GetBone(i);
                    auto parentId = asdx::BoneProxy::GetParentId(bone);
                    if (parentId < 0)
                        continue;

                    auto m0 = matrices[parentId];
                    auto p0 = m0.GetPosition();

                    auto m1 = matrices[i];
                    auto p1 = m1.GetPosition();

                    asdx::DrawWireBone(m_LineRenderer, p0, p1, asdx::Vector4(0.0f, 1.0f, 1.0f, 1.0f));
                }
            }
            // バインドポーズ表示.
            else
            {
                for(auto i=0u; i<count; ++i)
                {
                    auto& bone = m_Model->GetBone(i);

                    auto parentId = asdx::BoneProxy::GetParentId(bone);
                    if (parentId < 0)
                        continue;

                    auto& parentBone = m_Model->GetBone(uint32_t(parentId));
                    auto m0 = asdx::BoneProxy::GetBindPoseMatrix(parentBone);
                    auto p0 = m0.GetPosition();

                    auto m1 = asdx::BoneProxy::GetBindPoseMatrix(bone);
                    auto p1 = m1.GetPosition();

                    asdx::DrawWireBone(m_LineRenderer, p0, p1, asdx::Vector4(0.0f, 1.0f, 1.0f, 1.0f));
                }

            }
        }
    }

    // シェイプ用のカメラ行列を設定.
    m_ShapeStates.SetViewProj(m_Camera.GetView(), m_Proj);

    // 定数バッファを更新.
    {
        auto idx = GetCurrentBackBufferIndex();

        modelWorld = asdx::Matrix::CreateTranslation(m_ModelTranslation)
            * asdx::Matrix::CreateRotationFromYawPitchRoll(
                asdx::ToRadian(m_ModelRotation.y),
                asdx::ToRadian(m_ModelRotation.x),
                asdx::ToRadian(m_ModelRotation.z))
            * asdx::Matrix::CreateScale(m_ModelScale);

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
        if (m_EnableWireframe)
        { m_WireframeState.SetState(pCmd); }
        else
        { m_SolidState.SetState(pCmd); }

        pCmd->SetGraphicsRootConstantBufferView(ROOT_PARAM_B0, m_SceneCB[idx].GetGpuAddress());
        pCmd->SetGraphicsRoot32BitConstants(ROOT_PARAM_B1, 1, &m_DrawMode, 0);
        pCmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        for(auto i=0u; i<m_Model->GetMeshCount(); ++i)
        {
            auto mesh = m_Model->GetMesh(i);

            D3D12_VERTEX_BUFFER_VIEW VBVs[] = {
                mesh->GetPositions().GetVBV(),
                mesh->GetNormals  ().GetVBV(),
                mesh->GetTangents ().GetVBV(),
                mesh->GetTexCoords().GetVBV(),
                mesh->GetColors   ().GetVBV(),
            };

            auto IBV = mesh->GetIndices().GetIBV();

            pCmd->IASetVertexBuffers(0, _countof(VBVs), VBVs);
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
                if (m_EnableGuizmo)
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
                if (m_EnableGuizmo)
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
                if (m_EnableGuizmo)
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
        if (asdx::ModelConverter::Convert(path.string().c_str(), modelBinary))
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
    {
        LoadModel();
    }

    // モーションファイルを開く.
    if (ImGui::MenuItem(asdx::ToChar(u8"モーションファイルを開く")))
    {
        LoadMotion();
    }

    // 保存処理.
    if (!m_ModelBinary.empty())
    {
        if (ImGui::MenuItem(asdx::ToChar(u8"名前を付けて保存")))
        {
            const char* filter = 
                "Project Asura Model Binary (*.mdb)\0*.mdb\0";
            std::string base;
            std::string ext = ".mdb";

            asdx::fs::path path;
            if (asdx::SaveFileDlg(filter, path))
            {
                m_OutputPath = path.string();
                SaveModelBinary(m_OutputPath.c_str());
            }
        }

        if (ImGui::MenuItem(asdx::ToChar(u8"上書き保存")))
        {
            SaveModelBinary(m_OutputPath.c_str());
        }
    }
}
//-----------------------------------------------------------------------------
//      表示メニュー処理です.
//-----------------------------------------------------------------------------
void ModelViewer::MenuView()
{
    int mode = (int)m_DrawMode;
    ImGui::Combo(asdx::ToChar(u8"描画モード"), &mode, kDrawModes, _countof(kDrawModes));
    m_DrawMode = mode;

    ImGui::Checkbox(asdx::ToChar(u8"ワイヤーフレーム"), &m_EnableWireframe);
    ImGui::Checkbox(asdx::ToChar(u8"バウンディングスフィア表示"), &m_DrawBoundingSphere);
    ImGui::Checkbox(asdx::ToChar(u8"ボーン表示"), &m_DrawBones);
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

    m_MotionPlayer.Term();

    // モーションプレイヤーを初期化.
    m_MotionPlayer.Init(pModel);
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
        ELOG("Error : SaveA() Failed path = %s", path);
        return;
    }

    ILOG("Info : ModelBinary Output success. path = %s", path);
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
    if (asdx::OpenFileDlg(filter, path))
    {
        auto input = path.string();

        std::vector<uint8_t> modelBinary;
        if (path.extension().string() == ".mdb")
        {
            if (asdx::LoadA(input.c_str(), modelBinary))
            {
                m_ModelBinary = std::move(modelBinary);
                RecreateModel();
            }
        }
        else
        {
            if (asdx::ModelConverter::Convert(input.c_str(), modelBinary))
            {
                m_ModelBinary = std::move(modelBinary);
                RecreateModel();
            }
        }
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
    if (asdx::OpenFileDlg(filter, path))
    {
        auto input = path.string();

        std::vector<uint8_t> motionBinary;
        if (path.extension().string() == ".mob")
        {
            if (asdx::LoadA(input.c_str(), motionBinary))
            {
                m_MotionBinary.Load(std::move(motionBinary));
            }
        }
        else
        {
            if (asdx::MotionConverter::Convert(input.c_str(), motionBinary))
            {
                m_MotionBinary.Load(std::move(motionBinary));
                auto count = m_MotionBinary.GetClipCount();
                if (count > 0)
                {
                    m_ClipNames.resize(count);

                    for(auto i=0; i<count; ++i)
                    {
                        m_ClipNames[i] = asdx::MotionClipProxy::GetName(m_MotionBinary.GetClip(i));
                    }
                    m_MotionPlayer.SetClip(m_MotionBinary.GetClip(0));
                    m_ClipIndex = 0;
                }
            }
        }
    }
}