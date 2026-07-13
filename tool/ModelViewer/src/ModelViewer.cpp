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
#include <fnd/asdxFileIO.h>
#include <edit/asdxGuiMgr.h>
#include <gfx/asdxPresetState.h>
#include <res/asdxResTexture.h>
#include "ModelConverter.h"
#include "MotionConverter.h"
#include "TextureConverter.h"
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
    ROOT_PARAM_B1,  // ModelParam.
    ROOT_PARAM_B2,  // MaterialParam.

    ROOT_PARAM_T0,  // WorldMatrix
    ROOT_PARAM_T1,  // MatrixPallets.

    ROOT_PARAM_T5,  // DFG.
    ROOT_PARAM_T6,  // DiffuseLD.
    ROOT_PARAM_T7,  // SpecularLD.

    ROOT_PARAM_T10,  // BaseColor
    ROOT_PARAM_T11,  // Normal
    ROOT_PARAM_T12,  // ORM
    ROOT_PARAM_T13,  // Emissive.
    ROOT_PARAM_T14,  // Reserved0.
    ROOT_PARAM_T15,  // Reserved1.

    MAX_ROOT_PARAM_COUNT,
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

///////////////////////////////////////////////////////////////////////////////
// ParamScene structure
///////////////////////////////////////////////////////////////////////////////
struct alignas(256) ParamScene
{
    asdx::Matrix    View;
    asdx::Matrix    Proj;
    asdx::Vector3   CameraPos;
    float           FieldOfView;
    float           NearClip;
    float           FarClip;
    float           TargetWidth;
    float           TargetHeight;
};

///////////////////////////////////////////////////////////////////////////////
// LineVertex structure
///////////////////////////////////////////////////////////////////////////////
struct LineVertex
{
    asdx::Vector3   Position;     //!< 位置座標です.
    asdx::Unorm4    Color;        //!< 頂点カラーです.

    LineVertex()
    { /* DO_NOTHING */ }

    LineVertex(const asdx::Vector3& position, const asdx::Vector4& color)
    : Position(position)
    , Color   (color)
    { /* DO_NOTHING */ }
};

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

    m_ClearColor[0] = 0.1f;
    m_ClearColor[1] = 0.1f;
    m_ClearColor[2] = 0.1f;
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
            ELOGA("Error : GuiMgr::Init() Failed.");
            return false;
        }
    }

    // ルートシグニチャの生成.
    {
        D3D12_DESCRIPTOR_RANGE ranges[9] = {};
        asdx::InitRangeAsSRV(ranges[0], 5);
        asdx::InitRangeAsSRV(ranges[1], 6);
        asdx::InitRangeAsSRV(ranges[2], 7);

        asdx::InitRangeAsSRV(ranges[3], 10);
        asdx::InitRangeAsSRV(ranges[4], 11);
        asdx::InitRangeAsSRV(ranges[5], 12);
        asdx::InitRangeAsSRV(ranges[6], 13);
        asdx::InitRangeAsSRV(ranges[7], 14);
        asdx::InitRangeAsSRV(ranges[8], 15);

        D3D12_ROOT_PARAMETER params[MAX_ROOT_PARAM_COUNT] = {};
        asdx::InitAsCBV(params[ROOT_PARAM_B0], 0, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsConstants(params[ROOT_PARAM_B1], 1, 4, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsCBV(params[ROOT_PARAM_B2], 2, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[ROOT_PARAM_T0], 0, D3D12_SHADER_VISIBILITY_ALL);
        asdx::InitAsSRV(params[ROOT_PARAM_T1], 1, D3D12_SHADER_VISIBILITY_VERTEX);

        asdx::InitAsTable(params[ROOT_PARAM_T5], 1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        asdx::InitAsTable(params[ROOT_PARAM_T6], 1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
        asdx::InitAsTable(params[ROOT_PARAM_T7], 1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);

        asdx::InitAsTable(params[ROOT_PARAM_T10], 1, &ranges[3], D3D12_SHADER_VISIBILITY_PIXEL);
        asdx::InitAsTable(params[ROOT_PARAM_T11], 1, &ranges[4], D3D12_SHADER_VISIBILITY_PIXEL);
        asdx::InitAsTable(params[ROOT_PARAM_T12], 1, &ranges[5], D3D12_SHADER_VISIBILITY_PIXEL);
        asdx::InitAsTable(params[ROOT_PARAM_T13], 1, &ranges[6], D3D12_SHADER_VISIBILITY_PIXEL);
        asdx::InitAsTable(params[ROOT_PARAM_T14], 1, &ranges[7], D3D12_SHADER_VISIBILITY_PIXEL);
        asdx::InitAsTable(params[ROOT_PARAM_T15], 1, &ranges[8], D3D12_SHADER_VISIBILITY_PIXEL);

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
        desc.VS                             = { MeshVS, sizeof(MeshVS) };
        desc.PS                             = { MeshPS, sizeof(MeshPS) };
        desc.BlendState                     = asdx::Preset::Opaque;
        desc.SampleMask                     = D3D12_DEFAULT_SAMPLE_MASK;
        desc.RasterizerState                = asdx::Preset::CullNone;
        desc.DepthStencilState              = asdx::Preset::DepthReadWrite;
        desc.InputLayout.NumElements        = kStaticMeshElementCount;
        desc.InputLayout.pInputElementDescs = InputElements;
        desc.PrimitiveTopologyType          = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets               = 1;
        desc.RTVFormats[0]                  = m_SwapChainFormat;
        desc.DSVFormat                      = m_DepthStencilFormat;
        desc.SampleDesc.Count               = 1;
        desc.SampleDesc.Quality             = 0;

        if (!m_OpaqueState.StaticModel.Init(&desc))
        {
            ELOGA("Error : PipelineStateManager::Create() Failed.");
            return false;
        }

        desc.BlendState         = asdx::Preset::AlphaBlend;
        desc.DepthStencilState  = asdx::Preset::DepthReadOnly;
        if (!m_AlphaBlendState.StaticModel.Init(&desc))
        {
            ELOGA("Error : PipelineStateManager::Create() Failed.");
            return false;
        }

        desc.BlendState         = asdx::Preset::Opaque;
        desc.RasterizerState    = asdx::Preset::Wireframe;
        desc.DepthStencilState  = asdx::Preset::DepthReadWrite;
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
        desc.RasterizerState                = asdx::Preset::CullNone;
        desc.DepthStencilState              = asdx::Preset::DepthReadWrite;
        desc.InputLayout.NumElements        = kSkeletalMeshElementCount;
        desc.InputLayout.pInputElementDescs = InputElements;
        desc.PrimitiveTopologyType          = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets               = 1;
        desc.RTVFormats[0]                  = m_SwapChainFormat;
        desc.DSVFormat                      = m_DepthStencilFormat;
        desc.SampleDesc.Count               = 1;
        desc.SampleDesc.Quality             = 0;

        if (!m_OpaqueState.SkeletalModel.Init(&desc))
        {
            ELOGA("Error : PipelineStateManager::Create() Failed.");
            return false;
        }

        desc.BlendState         = asdx::Preset::AlphaBlend;
        desc.DepthStencilState  = asdx::Preset::DepthReadOnly;
        if (!m_AlphaBlendState.SkeletalModel.Init(&desc))
        {
            ELOGA("Error : PipelineStateManager::Create() Failed.");
            return false;
        }

        desc.BlendState         = asdx::Preset::Opaque;
        desc.RasterizerState    = asdx::Preset::Wireframe;
        desc.DepthStencilState  = asdx::Preset::DepthReadWrite;
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
            ELOGA("Error : ConstantBuffer::Init() Failed.");
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
        ELOGA("Error : ShapeStates::Init() Failed.");
        return false;
    }

    if (!m_ShapeParams.Init(UINT16_MAX))
    {
        ELOGA("Error : ShapeParams::Init() Failed.");
        return false;
    }

    if (!m_BoneShape.Init(1.0f, 0.25f))
    {
        ELOGA("Error : BoneShape::Init() Failed.");
        return false;
    }

    if (!m_SphereShape.Init(1.0f, 20))
    {
        ELOGA("Error : SphereShape::Init() Failed.");
        return false;
    }

    if (!m_LineRenderer.Init(12 * UINT16_MAX, m_SwapChainFormat, DXGI_FORMAT_UNKNOWN))
    {
        ELOGA("Error : LineRenderer::Init() Failed.");
        return false;
    }

    if (!CreateAxis(100.0f))
    {
        ELOGA("Error : CreateAxis() Failed.");
        return false;
    }

    if (!CreateGrid(1.0f, 200))
    {
        ELOGA("Error : CreateGrid() Failed.");
        return false;
    }

    if (!m_SkyContext.Init())
    {
        ELOGA("Error : SkyContext::Init() Failed.");
        return false;
    }

    if (!m_SkyBoxPS.Init(m_SkyContext, m_SwapChainFormat))
    {
        ELOGA("Error : SkyBoxPS::Init() Failed.");
        return false;
    }

    if (!m_SkySpherePS.Init(m_SkyContext, m_SwapChainFormat))
    {
        ELOGA("Error : SkySpherePS::Init() Failed.");
        return false;
    }

    // DFGテクスチャのロード.
    {
        asdx::fs::path path;
        if (!asdx::SearchFilePath("../../../res/textures/env_brdf.tga", path))
        {
            ELOGA("Error : File Not Found.");
            return false;
        }

        CreateTexture(path.string().c_str(), &m_DFG);
    }

    // EnvMapのロード.
    {
        asdx::fs::path path;
        if (!asdx::SearchFilePath("../res/textures/treasure_island.env.dds", path))
        {
            ELOGA("Error : File Not Found.");
            return false;
        }

        m_PathEnvMap = path.filename().string();
        CreateTexture(path.string().c_str(),&m_EnvMap);
    }

    // DiffuseLDテクスチャのロード.
    {
        asdx::fs::path path;
        if (!asdx::SearchFilePath("../res/textures/treasure_island.d.dds", path))
        {
            ELOGA("Error : File Not Found.");
            return false;
        }

        m_PathDiffuseLD = path.filename().string();
        CreateTexture(path.string().c_str(), &m_DiffuseLD);
    }

    // SpecularLDテクスチャのロード.
    {
        asdx::fs::path path;
        if (!asdx::SearchFilePath("../res/textures/treasure_island.s.dds", path))
        {
            ELOGA("Error : File Not Found.");
            return false;
        }

        m_PathSpecularLD = path.filename().string();
        CreateTexture(path.string().c_str(), &m_SpecularLD);
    }

    // コマンドの記録を終了.
    pCmd->Close();

    // セットアップコマンド実行.
    {
        // グラフィックスキューを取得.
        auto pGraphicsQueue = asdx::GetGraphicsQueue();

        // コマンドを実行.
        if (asdx::TextureManager::Instance().HasCommand())
        {
            auto pTextureCmd = asdx::TextureManager::Instance().Swap();

            ID3D12CommandList* pCmds[] = { pCmd, pTextureCmd };
            pGraphicsQueue->Execute(_countof(pCmds), pCmds);
        }
        else
        {
            ID3D12CommandList* pCmds[] = { pCmd };
            pGraphicsQueue->Execute(_countof(pCmds), pCmds);
        }

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
        m_WorldMatrixBuffer[i].Term();
        m_MatrixPalletBuffer[i].Term();
    }

    m_OpaqueState    .Reset();
    m_AlphaBlendState.Reset();

    m_WireframeState.Reset();

    m_RootSignature.Reset();

    m_Model.Reset();

    m_AxisVertexBuffer.Term();
    m_GridVertexBuffer.Term();

    m_SkyBoxPS   .Term();
    m_SkySpherePS.Term();
    m_SkyContext .Term();

    asdx::SafeRelease(m_EnvMap);
    asdx::SafeRelease(m_DiffuseLD);
    asdx::SafeRelease(m_SpecularLD);
    asdx::SafeRelease(m_DFG);

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
    auto modelWorld = asdx::Transform4x3::CreateIdentity();
    DrawGizmo(modelWorld);

    // コンテキストメニューを描画.
    DrawContextMenu(pCmd);

    // バウンディングスフィア描画.
    DrawBoundingSphere(modelWorld);

    // ボーン描画.
    DrawBones(modelWorld * root);

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

    // スカイ用カメラ行列を更新.
    auto skyProj = asdx::Matrix::CreatePerspectiveFieldOfView(fov, aspect, 1.0f, 10.0f);
    m_SkyContext.UpdateBuffer(m_Camera.GetView(), skyProj);

    // 定数バッファを更新.
    {
        auto idx = GetCurrentBackBufferIndex();

        auto param = m_SceneCB[idx].MapAs<ParamScene>();
        assert(param != nullptr);

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

    if (!!m_Model)
    {
        auto idx = GetCurrentBackBufferIndex();

        auto isSkeletal = (m_MotionBinary.GetClipCount() > 0) && (m_Model->GetBoneCount() > 0);
        // 行列パレットバッファ更新
        if (isSkeletal)
        {
            if (m_MatrixPalletBuffer[idx].GetResource() != nullptr)
            {
                auto& mtx = m_MotionPlayer.GetMatrixPalettes();
                auto  ptr = m_MatrixPalletBuffer[idx].Map();
                memcpy(ptr, mtx.data(), sizeof(asdx::Transform4x3) * mtx.size());
                m_MatrixPalletBuffer[idx].Unmap();
            }
        }

        // ワールド行列バッファ更新.
        {
            auto worlds = m_WorldMatrixBuffer[idx].MapAs<asdx::Transform4x3>();

            auto batchCount = m_Model->GetBatchCount();
            size_t offset = 0;
            for(auto i=0u; i<batchCount; ++i)
            {
                const auto& batch = m_Model->GetBatch(i);
                auto transform     = asdx::ModelBatchProxy::GetTransforms(batch);
                auto instanceCount = asdx::ModelBatchProxy::GetInstanceCount(batch);

                for(auto j=0u; j<instanceCount; ++j)
                    worlds[offset + j] = modelWorld * transform[j];

                offset += instanceCount;
            }

            m_WorldMatrixBuffer[idx].Unmap();
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
    m_ColorTarget[idx].ChangeState(pCmd, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // レンダーターゲット設定.
    auto handleRTV = m_ColorTarget[idx].GetCpuHandleRTV();
    auto handleDSV = m_DepthTarget.GetCpuHandleDSV();
    pCmd->OMSetRenderTargets(1, &handleRTV, FALSE, &handleDSV);
    pCmd->ClearRenderTargetView(handleRTV, m_ClearColor, 0, nullptr);
    pCmd->ClearDepthStencilView(handleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    pCmd->RSSetViewports(1, &m_Viewport);
    pCmd->RSSetScissorRects(1, &m_ScissorRect);

    // スカイ描画.
    DrawSky(pCmd);

    // グリッド描画.
    DrawGrid(pCmd);

    // 軸描画.
    DrawAxis(pCmd);

    // モデルを描画
    if (m_ModelInfo.MeshCount > 0)
    {
        auto addressParam = m_ShapeParams.Update();

        pCmd->SetGraphicsRootSignature(m_RootSignature.GetPtr());

        auto isSkeletal = (m_MotionBinary.GetClipCount() > 0) 
            && (m_Model->GetBoneCount() > 0)
            && (m_MatrixPalletBuffer[idx].GetResource() != nullptr);

        pCmd->SetGraphicsRootConstantBufferView(ROOT_PARAM_B0, m_SceneCB[idx].GetGpuAddress());
        pCmd->SetGraphicsRootShaderResourceView(ROOT_PARAM_T0, m_WorldMatrixBuffer[idx].GetGpuAddress());
        pCmd->SetGraphicsRoot32BitConstants(ROOT_PARAM_B1, 1, &m_DrawMode, 1);
        pCmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pCmd->SetGraphicsRootDescriptorTable(ROOT_PARAM_T5, m_DFG->GetHandleGPU());
        pCmd->SetGraphicsRootDescriptorTable(ROOT_PARAM_T6, m_DiffuseLD->GetHandleGPU());
        pCmd->SetGraphicsRootDescriptorTable(ROOT_PARAM_T7, m_SpecularLD->GetHandleGPU());

        if (isSkeletal)
        { pCmd->SetGraphicsRootShaderResourceView(ROOT_PARAM_T1, m_MatrixPalletBuffer[idx].GetGpuAddress()); }

        auto batchCount = m_Model->GetBatchCount();
        auto offset = 0u;
        for(auto i=0u; i<batchCount; ++i)
        {
            const auto& batch = m_Model->GetBatch(i);
            auto meshIds = asdx::ModelBatchProxy::GetMeshIds(batch);

            uint32_t matrixId = offset;

            pCmd->SetGraphicsRoot32BitConstants(ROOT_PARAM_B1, 1, &matrixId, 0);
            auto instanceCount = asdx::ModelBatchProxy::GetInstanceCount(batch);

            for(auto j=0u; j<meshIds.size(); ++j)
            {
                const auto mesh = m_Model->GetMesh(meshIds[j]);
                if (!mesh->IsVisible())
                    continue;

                const auto material  = m_Model->GetMaterial(mesh->GetMaterialId());
                const auto alphaMode = material->GetAlphaMode();

                if (isSkeletal)
                {
                    if (m_EnableWireframe)
                    { m_WireframeState.SkeletalModel.SetState(pCmd); }
                    else if (alphaMode == asdx::AlphaMode::Opaque || alphaMode == asdx::AlphaMode::Mask)
                    { m_OpaqueState.SkeletalModel.SetState(pCmd); }
                    else if (alphaMode == asdx::AlphaMode::Blend)
                    { m_AlphaBlendState.SkeletalModel.SetState(pCmd); }
                }
                else
                {
                    if (m_EnableWireframe)
                    { m_WireframeState.StaticModel.SetState(pCmd); }
                    else if (alphaMode == asdx::AlphaMode::Opaque || alphaMode == asdx::AlphaMode::Mask)
                    { m_OpaqueState.StaticModel.SetState(pCmd); }
                    else if (alphaMode == asdx::AlphaMode::Blend)
                    { m_AlphaBlendState.StaticModel.SetState(pCmd); }
                }

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

                pCmd->SetGraphicsRootConstantBufferView(ROOT_PARAM_B2,  material->GetBuffer().GetGpuAddress());
                pCmd->SetGraphicsRootDescriptorTable   (ROOT_PARAM_T10, material->GetTexture(asdx::Material::TEXTURE_BASE_COLOR).GetHandleGPU());
                pCmd->SetGraphicsRootDescriptorTable   (ROOT_PARAM_T11, material->GetTexture(asdx::Material::TEXTURE_NORMAL)    .GetHandleGPU());
                pCmd->SetGraphicsRootDescriptorTable   (ROOT_PARAM_T12, material->GetTexture(asdx::Material::TEXTURE_ORM)       .GetHandleGPU());
                pCmd->SetGraphicsRootDescriptorTable   (ROOT_PARAM_T13, material->GetTexture(asdx::Material::TEXTURE_EMISSIVE)  .GetHandleGPU());

                pCmd->IASetVertexBuffers(0, countVBV, VBVs);
                pCmd->IASetIndexBuffer(&IBV);

                pCmd->DrawIndexedInstanced(mesh->GetIndexCount(), instanceCount, 0, 0, 0);
            }

            offset += instanceCount;
        }

        if (m_DrawBoundingSphere)
        {
            m_ShapeStates.ApplyTranslucentState(pCmd);
            pCmd->SetGraphicsRootShaderResourceView(asdx::ShapeStates::SRV0, addressParam);

            for(size_t i=0; i<m_Model->GetMeshCount(); ++i)
            {
                uint32_t index = uint32_t(i);
                pCmd->SetGraphicsRoot32BitConstant(asdx::ShapeStates::CBV3, index, 0);
                m_SphereShape.Draw(pCmd);
            }

            {
                uint32_t index = uint32_t(m_Model->GetMeshCount());
                pCmd->SetGraphicsRoot32BitConstant(asdx::ShapeStates::CBV3, index, 0);
                m_SphereShape.Draw(pCmd);
            }
        }
    }

    if (m_DrawBones || m_DrawAxis)
    {
        auto addr = m_ShapeStates.GetBufferAddress();
        m_LineRenderer.SetPipelineState(pCmd);
        pCmd->SetGraphicsRootConstantBufferView(0, addr);
        m_LineRenderer.Draw(pCmd);
    }

    // GUIを描画.
    asdx::GuiMgr::Instance().Draw(pCmd);

    // 表示状態に遷移.
    m_ColorTarget[idx].ChangeState(pCmd, D3D12_RESOURCE_STATE_PRESENT);

    // コマンド記録終了.
    pCmd->Close();

    // コマンドを実行.
    {
        auto pGraphicsQueue = asdx::GetGraphicsQueue();

        // 前フレームの描画の完了を待機.
        if (m_FrameWaitPoint.IsValid())
        { pGraphicsQueue->Sync(m_FrameWaitPoint); }

        // コマンドを実行.
        if (asdx::TextureManager::Instance().HasCommand())
        {
            auto pTextureCmd = asdx::TextureManager::Instance().Swap();

            ID3D12CommandList* pCmds[] = { pCmd, pTextureCmd };
            pGraphicsQueue->Execute(_countof(pCmds), pCmds);
        }
        else
        {
            ID3D12CommandList* pCmds[] = { pCmd };
            pGraphicsQueue->Execute(_countof(pCmds), pCmds);
        }

        // 待機点を発行.
        m_FrameWaitPoint = pGraphicsQueue->Signal();
    }

    Present(1);

    asdx::FrameSync();
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

    // 文字入力を誤検知しないようにチェック.
    auto itemActive = ImGui::IsAnyItemFocused() || ImGui::IsAnyItemActive();

    if (args.IsKeyDown)
    {
        switch(args.KeyCode)
        {
        // 移動ツール.
        case 'W':
            {
                if (!itemActive)
                {
                    if (m_EnableGuizmo && m_GuizmoOperation == ImGuizmo::OPERATION::TRANSLATE)
                    { m_EnableGuizmo = false; }
                    else
                    {
                        m_EnableGuizmo    = true;
                        m_GuizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
                    }
                }
            }
            break;

        // 回転ツール.
        case 'E':
            {
                if (!itemActive)
                {
                    if (m_EnableGuizmo && m_GuizmoOperation == ImGuizmo::OPERATION::ROTATE)
                    { m_EnableGuizmo = false; }
                    else
                    {
                        m_EnableGuizmo    = true;
                        m_GuizmoOperation = ImGuizmo::OPERATION::ROTATE;
                    }
                }
            }
            break;

        // スケールツール.
        case 'R':
            {
                if (!itemActive)
                {
                    if (m_EnableGuizmo && m_GuizmoOperation == ImGuizmo::OPERATION::SCALE)
                    { m_EnableGuizmo = false; }
                    else
                    {
                        m_EnableGuizmo    = true;
                        m_GuizmoOperation = ImGuizmo::OPERATION::SCALE;
                    }
                }
            }
            break;

        // ワールド行列リセット.
        case 'Z':
            {
                if (!itemActive)
                {
                    m_ModelTranslation  = asdx::Vector3(0.0f, 0.0f, 0.0f);
                    m_ModelRotation     = asdx::Vector3(0.0f, 0.0f, 0.0f);
                    m_ModelScale        = asdx::Vector3(1.0f, 1.0f, 1.0f);
                    m_EnableGuizmo      = false;
                }
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
            // カレントディレクトリ取得.
            char tempDir[512] = {};
            GetCurrentDirectoryA(512, tempDir);

            // モデルパスをカレントディレクトリに変更に.
            auto dir = path.parent_path().string();
            m_ModelBaseDir = dir;

            m_ModelBinary = std::move(modelBinary);
            RecreateModel();

            // ディレクトリを元に戻す.
            SetCurrentDirectoryA(tempDir);
        }
    }
    else
    {
        if (ModelConverter::Convert(path.string().c_str(), true, path.parent_path().string(), modelBinary))
        {
            // カレントディレクトリ取得.
            char tempDir[512] = {};
            GetCurrentDirectoryA(512, tempDir);

            // モデルパスをカレントディレクトリに変更に.
            auto dir = path.parent_path().string();
            m_ModelBaseDir = dir;

            m_ModelBinary = std::move(modelBinary);
            RecreateModel();

            // ディレクトリを元に戻す.
            SetCurrentDirectoryA(tempDir);
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

    // モーションファイルを開く.
    if (ImGui::MenuItem(asdx::ToChar(u8"モーションファイルを開く")))
    { LoadMotion(); }
}

//-----------------------------------------------------------------------------
//      表示メニュー処理です.
//-----------------------------------------------------------------------------
void ModelViewer::MenuView()
{
    ImGui::Checkbox(asdx::ToChar(u8"プロパティウィンドウ"), &m_ShowProperty);
    ImGui::Checkbox(asdx::ToChar(u8"情報パネル"), &m_ShowInfo);
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
    if (!asdx::Model::Create(std::move(copyBinary), m_ModelBaseDir.c_str(), &pModel))
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

    auto sphere = m_Model->GetSphere();

    // カメラを初期化.
    m_Camera.Init(
        asdx::Vector3(sphere.Center.x, sphere.Center.y, sphere.Center.z + sphere.Radius * 3.0f),
        sphere.Center,
        asdx::Vector3(0.0f, 0.0f, 1.0f),
        0.1f,
        10000.0f);
    m_Camera.Present();

    // モーションプレイヤーを初期化.
    m_MotionPlayer.Init(pModel);

    // 行列パレット用 SRV を生成.
    auto boneCount = m_Model->GetBoneCount();
    if (boneCount > 0)
    {
        for(auto i=0; i<2; ++i)
        {
            m_MatrixPalletBuffer[i].Term();

            if (!m_MatrixPalletBuffer[i].Init(boneCount, sizeof(asdx::Transform4x3), D3D12_RESOURCE_STATE_COMMON, true))
            {
                ELOGA("Error : Matrix Pallet Buffer Init Failed. index = %u", i);
            }
        }
    }

    auto instanceCount = m_Model->GetTotalInstanceCount();
    auto batchCount    = m_Model->GetBatchCount();

    for(auto i=0; i<2; ++i)
    {
        m_WorldMatrixBuffer[i].Term();

        if (!m_WorldMatrixBuffer[i].Init(instanceCount, sizeof(asdx::Transform4x3), D3D12_RESOURCE_STATE_COMMON, true))
        {
            ELOGA("Error : WorldMatrix Buffer Init Failed. index = %u", i);
        }
    }

    auto ptr0 = m_WorldMatrixBuffer[0].MapAs<asdx::Transform4x3>();
    auto ptr1 = m_WorldMatrixBuffer[1].MapAs<asdx::Transform4x3>();

    size_t offset = 0;
    for(auto i=0u; i<batchCount; ++i)
    {
        const auto& batch = m_Model->GetBatch(i);
        auto mtx = asdx::ModelBatchProxy::GetTransforms(batch);

        if (ptr0 != nullptr)
            memcpy(&ptr0[offset], mtx.data(), sizeof(asdx::Transform4x3) * mtx.size());
        if (ptr1 != nullptr)
            memcpy(&ptr1[offset], mtx.data(), sizeof(asdx::Transform4x3) * mtx.size());

        offset += mtx.size();
    }

    m_WorldMatrixBuffer[0].Unmap();
    m_WorldMatrixBuffer[1].Unmap();
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
        ELOGA("Error : SaveA() Failed. path = %s", path);
        return;
    }

    ILOGA("Info : ModelBinary Output success. path = %s", path);
}

//-----------------------------------------------------------------------------
//      モデルファイルを読み込みます.
//-----------------------------------------------------------------------------
void ModelViewer::LoadModel()
{
    const char* filter = 
        "読み込み可能なモデルファイル\0*.mdb;*.dae;*.xml;*.blend;*.3ds;*.ase;*.gltf;*.glb;*.fbx;*.ply;*.dxf;*.smd;*.vta;*.mdl;*.md2;*.md3;*.md5mesh;*.md5anim;*.x;*.obj;*.ter;*.ms3d;*.lxo;*.lwo;*.lws;*.pmd;*.pmx;*.vmd;*.usd;*.usda;*.usdc;*.usdz\0"
        "Project Asura Model Binary (*.mdb)\0*.mdb\0"
        "Blender Format (*.blend)\0*.blend\0"
        "Collada (*.dae, *.xml)\0*.dae;*.xml\0"
        "3D Studio Max 3DS (*.3ds)\0*.3ds\0"
        "3D Studio Max ASE (*.ase)\0*.ase\0"
        "GL Transmission Format (*.gltf, *.glb)\0*.gltf;*.glb\0"
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
            ELOGA("Error : asdx::LoadA() Failed. path = %s", input.c_str());
            return;
        }

        m_ModelBaseDir = path.parent_path().string();

        m_ModelBinary = std::move(modelBinary);
        RecreateModel();
    }
    else
    {
        m_ModelBaseDir = path.parent_path().string();

        if (!ModelConverter::Convert(input.c_str(), true, m_ModelBaseDir, modelBinary))
        {
            ELOGA("Error : ModelConverter::Convert() Failed.");
            return;
        }

        m_ModelBinary = std::move(modelBinary);
        RecreateModel();
    }
}

//-----------------------------------------------------------------------------
//      モーションファイルを読み込みます.
//-----------------------------------------------------------------------------
void ModelViewer::LoadMotion()
{
    const char* filter = 
        "読み込み可能なモーションファイル\0*.mob;*.dae;*.xml;*.blend;*.3ds;*.ase;*.gltf;*.glb;*.fbx;*.ply;*.dxf;*.smd;*.vta;*.mdl;*.md2;*.md3;*.md5anim;*.x;*.ms3d;*.lws;*.pmd;*.pmx;*.vmd;*.usd;*.usda;*.usdc;*.usdz\0"
        "Project Asura Motion Binary (*.mob)\0*.mob\0"
        "Blender Format (*.blend)\0*.blend\0"
        "Collada (*.dae, *.xml)\0*.dae;*.xml\0"
        "3D Studio Max 3DS (*.3ds)\0*.3ds\0"
        "3D Studio Max ASE (*.ase)\0*.ase\0"
        "GL Transmission Format (*.gltf, *.glb)\0*.gltf;*.glb\0"
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
            ELOGA("Error : asdx::LoadA() Failed. path = %s", input.c_str());
            return;
        }

        m_MotionBinary.Load(std::move(motionBinary));
    }
    else
    {
        if (!MotionConverter::Convert(input.c_str(), motionBinary))
        {
            ELOGA("Error : MotionConverter::Convert() Failed.");
            return;
        }

        m_MotionBinary.Load(std::move(motionBinary));
    }

    auto count = m_MotionBinary.GetClipCount();
    if (count == 0)
    {
        ELOGA("Error : Clip Count is zero.");
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
//      軸を生成します.
//-----------------------------------------------------------------------------
bool ModelViewer::CreateAxis(float length)
{
    LineVertex vertices[] = {
        // X軸.
        LineVertex( asdx::Vector3( 0.0f,   0.0f,  0.0f ), asdx::Vector4( 1.0f, 0.0f, 0.0f, 1.0f ) ),
        LineVertex( asdx::Vector3( length, 0.0f,  0.0f ), asdx::Vector4( 1.0f, 0.0f, 0.0f, 1.0f ) ),
        // Y軸.
        LineVertex( asdx::Vector3( 0.0f,   0.0f,  0.0f  ), asdx::Vector4( 0.0f, 1.0f, 0.0f, 1.0f ) ),
        LineVertex( asdx::Vector3( 0.0f,   length, 0.0f ), asdx::Vector4( 0.0f, 1.0f, 0.0f, 1.0f ) ),
        // Z軸.
        LineVertex( asdx::Vector3( 0.0f,   0.0f,  0.0f  ), asdx::Vector4( 0.0f, 0.0f, 1.0f, 1.0f ) ),
        LineVertex( asdx::Vector3( 0.0f,   0.0f, length ), asdx::Vector4( 0.0f, 0.0f, 1.0f, 1.0f ) )
    };

    if (!m_AxisVertexBuffer.Init(sizeof(LineVertex) * 6, sizeof(LineVertex)))
    {
        ELOGA("Error : VertexBuffer::Init() Failed.");
        return false;
    }

    auto dstPtr = m_AxisVertexBuffer.MapAs<LineVertex>();
    memcpy(dstPtr, vertices, sizeof(vertices));
    m_AxisVertexBuffer.Unmap();

    m_AxisVertexCount = 6;

    return true;
}

//-----------------------------------------------------------------------------
//      グリッドを生成します.
//-----------------------------------------------------------------------------
bool ModelViewer::CreateGrid(float step, uint32_t count)
{
    auto vertexCount = (count + 1) * 4;

    std::vector<LineVertex> vertices;
    vertices.resize(vertexCount);

    auto w = ( count / 2.0f ) * step;
    auto h = ( count / 2.0f ) * step;
    auto idx = 0;

    auto color      = asdx::Unorm4(0.75f, 0.75f, 0.75f, 1.0f);
    auto thin_color = asdx::Unorm4(0.25f, 0.25f, 0.25f, 1.0f);

    // 縦線.
    for( auto i=0u; i<=count; i++ )
    {
        auto x = -w + ( i * step );

        vertices[idx].Position = asdx::Vector3(  x, 0.0f, -h );
        vertices[idx].Color    = ( i % 10 == 0 ) ? color : thin_color;
        idx++;

        vertices[idx].Position = asdx::Vector3(  x, 0.0f,  h ); 
        vertices[idx].Color    = ( i % 10 == 0 ) ? color : thin_color;
        idx++;
    }

    // 横線.
    for( auto i=0u; i<=count; i++ )
    {
        auto z = -h + ( i * step );
        vertices[idx].Position = asdx::Vector3( -w, 0.0f, z );
        vertices[idx].Color    = ( i % 10 == 0 ) ? color : thin_color;
        idx++;

        vertices[idx].Position = asdx::Vector3(  w, 0.0f, z ); 
        vertices[idx].Color    = ( i % 10 == 0 ) ? color : thin_color;
        idx++;
    }

    auto ret = m_GridVertexBuffer.Init(sizeof(LineVertex) * vertexCount, sizeof(LineVertex));
    if (!ret)
    {
        ELOGA("Error : VertexBuffer::Init() Failed.");
        return false;
    }

    auto dstPtr = m_GridVertexBuffer.MapAs<LineVertex>();
    memcpy(dstPtr, vertices.data(), sizeof(LineVertex) * vertices.size());
    m_GridVertexBuffer.Unmap();

    m_GridVertexCount = vertexCount;

    return true;
}

//-----------------------------------------------------------------------------
//      軸を描画します.
//-----------------------------------------------------------------------------
void ModelViewer::DrawAxis(ID3D12GraphicsCommandList* pCmd)
{
    if (!m_DrawAxis || pCmd == nullptr)
        return;

    auto addr = m_ShapeStates.GetBufferAddress();
    m_LineRenderer.SetPipelineState(pCmd);
    pCmd->SetGraphicsRootConstantBufferView(0, addr);
    auto vbv = m_AxisVertexBuffer.GetVBV();
    pCmd->IASetVertexBuffers(0, 1, &vbv);
    pCmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    pCmd->DrawInstanced(m_AxisVertexCount, 1, 0, 0);
}

//-----------------------------------------------------------------------------
//      グリッドを描画します.
//-----------------------------------------------------------------------------
void ModelViewer::DrawGrid(ID3D12GraphicsCommandList* pCmd)
{
    if (!m_DrawGrid || pCmd == nullptr)
        return;

    auto addr = m_ShapeStates.GetBufferAddress();
    m_LineRenderer.SetPipelineState(pCmd);
    pCmd->SetGraphicsRootConstantBufferView(0, addr);
    auto vbv = m_GridVertexBuffer.GetVBV();
    pCmd->IASetVertexBuffers(0, 1, &vbv);
    pCmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    pCmd->DrawInstanced(m_GridVertexCount, 1, 0, 0);
}

//-----------------------------------------------------------------------------
//      スカイを描画します.
//-----------------------------------------------------------------------------
void ModelViewer::DrawSky(ID3D12GraphicsCommandList* pCmd)
{
    if (!m_DrawSky || pCmd == nullptr)
        return;

    if (!m_EnvMap)
        return;

    auto desc = m_EnvMap->GetDesc();
    if (desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        return;

    if (desc.DepthOrArraySize == 1)
        m_SkySpherePS.Draw(pCmd, m_SkyContext, m_EnvMap->GetHandleGPU());
    else
        m_SkyBoxPS.Draw(pCmd, m_SkyContext, m_EnvMap->GetHandleGPU());
}

//-----------------------------------------------------------------------------
//      テクスチャを生成します.
//-----------------------------------------------------------------------------
void ModelViewer::CreateTexture(const char* path, asdx::Texture** ppTexture)
{
    if (path == nullptr || ppTexture == nullptr)
    {
        ELOGA("Error : Invalid Argument.");
        return;
    }

    std::vector<uint8_t> binary;
    if (!TextureConverter::Convert(path, binary))
    {
        ELOGA("Error : TextureConvert::Convert() Failed. path = %s", path);
        return;
    }

    asdx::TextureBinary texBin;
    texBin.Load(std::move(binary));

    asdx::ResTexture res = texBin.GetResource();
    asdx::Texture* pTempTexture = nullptr;
    if (!asdx::Texture::Create(res, &pTempTexture))
    {
        ELOGA("Error : asdx::Texture::Create() Failed. path = %s", path);
    }
    else
    {
        // 既に作成されている奴がいたら解放.
        if ((*ppTexture) != nullptr)
        {
            (*ppTexture)->Release();
            (*ppTexture) = nullptr;
        }

        // 差し替え.
        (*ppTexture) = pTempTexture;
        pTempTexture = nullptr;
    }
}