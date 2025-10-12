//-----------------------------------------------------------------------------
// File : asdxPipelineState.cpp
// Desc : Pipeline State.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <algorithm>
#include <d3d12shader.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMisc.h>
#include <gfx/asdxPipelineState.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxShaderCompiler.h>
#include <fnd/asdxHash.h>
#include <fnd/asdxMacro.h>


// ヘッダにあるとバグの原因となるので，ソース側に定義.
#ifdef __ID3D12GraphicsCommandList6_INTERFACE_DEFINED__
#define ASDX_ENABLE_MESH_SHADER
#endif//__ID3D12GraphicsCommandList6_INTERFACE_DEFINED__

namespace {

///////////////////////////////////////////////////////////////////////////////
// ROOT_PARAM_TYPE
///////////////////////////////////////////////////////////////////////////////
enum ROOT_PARAM_TYPE
{
    ROOT_PARAM_VAR = 0,
    ROOT_PARAM_CBV = 1,
    ROOT_PARAM_SRV = 2,
    ROOT_PARAM_UAV = 3,
    ROOT_PARAM_SMP = 4,
    ROOT_PARAM_AS  = 5,
};

///////////////////////////////////////////////////////////////////////////////
// PSSubObject class
///////////////////////////////////////////////////////////////////////////////
template<typename StructType, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type>
class alignas(void*) SubObject
{
public:
    SubObject() noexcept
    : m_Type    (Type)
    , m_Value   (StructType())
    { /* DO_NOTHING */ }

    SubObject(StructType const& value) noexcept
    : m_Type    (Type)
    , m_Value   (value)
    { /* DO_NOTHING */ }

    SubObject& operator = (StructType const& value) noexcept
    {
        m_Type  = Type;
        m_Value = value;
        return *this;
    }

    operator StructType const&() const noexcept 
    { return m_Value; }

    operator StructType&() noexcept 
    { return m_Value; }

    StructType* operator&() noexcept
    { return &m_Value; }

    StructType const* operator&() const noexcept
    { return &m_Value; }

private:
    D3D12_PIPELINE_STATE_SUBOBJECT_TYPE m_Type;
    StructType                          m_Value;
};

#define PSST(x) D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_##x

using PSS_ROOT_SIGNATURE = SubObject< ID3D12RootSignature*,        PSST(ROOT_SIGNATURE) >;
#ifdef ASDX_ENABLE_MESH_SHADER
using PSS_AS             = SubObject< D3D12_SHADER_BYTECODE,       PSST(AS) >;
using PSS_MS             = SubObject< D3D12_SHADER_BYTECODE,       PSST(MS) >;
#endif//ASDX_ENABLE_MESH_SHADER
using PSS_PS             = SubObject< D3D12_SHADER_BYTECODE,       PSST(PS) >;
using PSS_BLEND          = SubObject< D3D12_BLEND_DESC,            PSST(BLEND) >;
using PSS_SAMPLE_MASK    = SubObject< UINT,                        PSST(SAMPLE_MASK) >;
using PSS_RASTERIZER     = SubObject< D3D12_RASTERIZER_DESC,       PSST(RASTERIZER) >;
using PSS_DEPTH_STENCIL  = SubObject< D3D12_DEPTH_STENCIL_DESC,    PSST(DEPTH_STENCIL) >;
using PSS_RTV_FORMATS    = SubObject< D3D12_RT_FORMAT_ARRAY,       PSST(RENDER_TARGET_FORMATS) >;
using PSS_DSV_FORMAT     = SubObject< DXGI_FORMAT,                 PSST(DEPTH_STENCIL_FORMAT) >;
using PSS_SAMPLE_DESC    = SubObject< DXGI_SAMPLE_DESC,            PSST(SAMPLE_DESC) >;
using PSS_NODE_MASK      = SubObject< UINT,                        PSST(NODE_MASK) >;
using PSS_CACHED_PSO     = SubObject< D3D12_CACHED_PIPELINE_STATE, PSST(CACHED_PSO) >;
using PSS_FLAGS          = SubObject< D3D12_PIPELINE_STATE_FLAGS,  PSST(FLAGS) >;

#undef PSST


///////////////////////////////////////////////////////////////////////////////
// MsPsoDesc structure
///////////////////////////////////////////////////////////////////////////////
struct MsPsoDesc
{
    PSS_ROOT_SIGNATURE  RootSignature;
#ifdef ASDX_ENABLE_MESH_SHADER
    PSS_AS              AS;
    PSS_MS              MS;
#endif//ASDX_ENABLE_MESH_SHADER
    PSS_PS              PS;
    PSS_BLEND           BlendState;
    PSS_SAMPLE_MASK     SampleMask;
    PSS_RASTERIZER      RasterizerState;
    PSS_DEPTH_STENCIL   DepthStencilState;
    PSS_RTV_FORMATS     RTVFormats;
    PSS_DSV_FORMAT      DSVFormat;
    PSS_SAMPLE_DESC     SampleDesc;
    PSS_NODE_MASK       NodeMask;
    PSS_CACHED_PSO      CachedPSO;
    PSS_FLAGS           Flags;

    MsPsoDesc()
    { /* DO_NOTHING */ }

    MsPsoDesc(const asdx::MESH_SHADER_PIPELINE_STATE_DESC* pValue)
    {
        RootSignature       = pValue->pRootSignature;
    #ifdef ASDX_ENABLE_MESH_SHADER
        AS                  = pValue->AS;
        MS                  = pValue->MS;
    #endif
        PS                  = pValue->PS;
        BlendState          = pValue->BlendState;
        SampleMask          = pValue->SampleMask;
        RasterizerState     = pValue->RasterizerState;
        DepthStencilState   = pValue->DepthStencilState;
        RTVFormats          = pValue->RTVFormats;
        DSVFormat           = pValue->DSVFormat;
        SampleDesc          = pValue->SampleDesc;
        NodeMask            = pValue->NodeMask;
        CachedPSO           = pValue->CachedPSO;
        Flags               = pValue->Flags;
    }
};

//-----------------------------------------------------------------------------
//      グラフィックスパイプラインステート設定のハッシュ値を計算します.
//-----------------------------------------------------------------------------
uint64_t CalcDescHash(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc)
{
    auto hash = asdx::CalcHash(pDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    if (pDesc->VS.pShaderBytecode != nullptr && pDesc->VS.BytecodeLength > 0)
    { hash = asdx::CalcHashWithSeed(pDesc->VS.pShaderBytecode, pDesc->VS.BytecodeLength, hash); }
    if (pDesc->PS.pShaderBytecode != nullptr && pDesc->PS.BytecodeLength > 0)
    { hash = asdx::CalcHashWithSeed(pDesc->PS.pShaderBytecode, pDesc->PS.BytecodeLength, hash); }
    if (pDesc->DS.pShaderBytecode != nullptr && pDesc->DS.BytecodeLength > 0)
    { hash = asdx::CalcHashWithSeed(pDesc->DS.pShaderBytecode, pDesc->DS.BytecodeLength, hash); }
    if (pDesc->HS.pShaderBytecode != nullptr && pDesc->HS.BytecodeLength > 0)
    { hash = asdx::CalcHashWithSeed(pDesc->HS.pShaderBytecode, pDesc->HS.BytecodeLength, hash); }
    if (pDesc->GS.pShaderBytecode != nullptr && pDesc->GS.BytecodeLength > 0)
    { hash = asdx::CalcHashWithSeed(pDesc->GS.pShaderBytecode, pDesc->GS.BytecodeLength, hash); }
    return hash;
}

//-----------------------------------------------------------------------------
//      コンピュートパイプラインステート設定のハッシュ値を計算します.
//-----------------------------------------------------------------------------
uint64_t CalcDescHash(const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc)
{
    auto hash = asdx::CalcHash(pDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
    hash = asdx::CalcHashWithSeed(pDesc->CS.pShaderBytecode, pDesc->CS.BytecodeLength, hash);
    return hash;
}

//-----------------------------------------------------------------------------
//      メッシュシェーダパイプラインステート設定のハッシュ値を計算します.
//-----------------------------------------------------------------------------
uint64_t CalcDescHash(const asdx::MESH_SHADER_PIPELINE_STATE_DESC* pDesc)
{
    auto hash = asdx::CalcHash(pDesc, sizeof(asdx::MESH_SHADER_PIPELINE_STATE_DESC));
    if (pDesc->AS.pShaderBytecode != nullptr && pDesc->AS.BytecodeLength > 0)
    { hash = asdx::CalcHashWithSeed(pDesc->AS.pShaderBytecode, pDesc->AS.BytecodeLength, hash); }
    if (pDesc->MS.pShaderBytecode != nullptr && pDesc->MS.BytecodeLength > 0)
    { hash = asdx::CalcHashWithSeed(pDesc->MS.pShaderBytecode, pDesc->MS.BytecodeLength, hash); }
    if (pDesc->PS.pShaderBytecode != nullptr && pDesc->PS.BytecodeLength > 0)
    { hash = asdx::CalcHashWithSeed(pDesc->PS.pShaderBytecode, pDesc->PS.BytecodeLength, hash); }
    return hash;
}

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// PipelineStateManager class
///////////////////////////////////////////////////////////////////////////////
PipelineStateManager PipelineStateManager::s_Instance = {};

//-----------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-----------------------------------------------------------------------------
PipelineStateManager& PipelineStateManager::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      グラフィックスパイプラインステートを生成します.
//-----------------------------------------------------------------------------
bool PipelineStateManager::Create(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, PipelineStateHandle& handle)
{
    if (pDesc == nullptr)
    {
        ELOG("Error : Invalid Arguments.");
        return false;
    }

    // ハッシュ値を計算.
    auto hash = CalcDescHash(pDesc);

    // 既に作成済みかどうかチェック.
    if (m_PipelineStates.find(hash) != m_PipelineStates.end())
    {
        ELOG("Error : Already Registered.");
        return false;
    }

    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    ID3D12PipelineState* pPipelineState = nullptr;
    auto hr = pDevice->CreateGraphicsPipelineState(pDesc, IID_PPV_ARGS(&pPipelineState));
    if (FAILED(hr))
    {
        if (pPipelineState != nullptr)
        {
            pPipelineState->Release();
            pPipelineState = nullptr;
        }
        ELOG("Error : ID3D12Device::CreateGraphicsState() Failed. errcode = 0x%x", hr);
        return false;
    }

    m_PipelineStates[hash] = pPipelineState;

    PipelineStateDesc desc;
    desc.Type = PIPELINE_TYPE_GRAPHICS;
    desc.Graphics = (*pDesc);

    if (pDesc->VS.pShaderBytecode != nullptr && pDesc->VS.BytecodeLength > 0)
    {
        desc.VS.resize(pDesc->VS.BytecodeLength);
        memcpy(desc.VS.data(), pDesc->VS.pShaderBytecode, pDesc->VS.BytecodeLength);
        desc.Graphics.VS.pShaderBytecode = desc.VS.data();
        desc.Graphics.VS.BytecodeLength  = desc.VS.size();
    }
    if (pDesc->PS.pShaderBytecode != nullptr && pDesc->PS.BytecodeLength > 0)
    {
        desc.PS.resize(pDesc->PS.BytecodeLength);
        memcpy(desc.PS.data(), pDesc->PS.pShaderBytecode, pDesc->PS.BytecodeLength);
        desc.Graphics.PS.pShaderBytecode = desc.PS.data();
        desc.Graphics.PS.BytecodeLength  = desc.PS.size();
    }
    if (pDesc->DS.pShaderBytecode != nullptr && pDesc->DS.BytecodeLength > 0)
    {
        desc.DS.resize(pDesc->DS.BytecodeLength);
        memcpy(desc.DS.data(), pDesc->DS.pShaderBytecode, pDesc->DS.BytecodeLength);
        desc.Graphics.DS.pShaderBytecode = desc.DS.data();
        desc.Graphics.DS.BytecodeLength  = desc.DS.size();
    }
    if (pDesc->HS.pShaderBytecode != nullptr && pDesc->HS.BytecodeLength > 0)
    {
        desc.HS.resize(pDesc->HS.BytecodeLength);
        memcpy(desc.HS.data(), pDesc->HS.pShaderBytecode, pDesc->HS.BytecodeLength);
        desc.Graphics.HS.pShaderBytecode = desc.HS.data();
        desc.Graphics.HS.BytecodeLength  = desc.HS.size();
    }
    if (pDesc->HS.pShaderBytecode != nullptr && pDesc->HS.BytecodeLength > 0)
    {
        desc.GS.resize(pDesc->GS.BytecodeLength);
        memcpy(desc.GS.data(), pDesc->GS.pShaderBytecode, pDesc->GS.BytecodeLength);
        desc.Graphics.GS.pShaderBytecode = desc.GS.data();
        desc.Graphics.GS.BytecodeLength  = desc.GS.size();
    }

    m_Descs[hash] = desc;
    handle = hash;

    return true;
}

//-----------------------------------------------------------------------------
//      コンピュートパイプラインステートを生成します.
//-----------------------------------------------------------------------------
bool PipelineStateManager::Create(const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc, PipelineStateHandle& handle)
{
    if (pDesc == nullptr)
    {
        ELOG("Error : Invalid Arguments.");
        return false;
    }

    // ハッシュ値を計算.
    auto hash = CalcDescHash(pDesc);

    // 既に作成済みかどうかチェック.
    if (m_PipelineStates.find(hash) != m_PipelineStates.end())
    {
        ELOG("Error : Already Registered.");
        return false;
    }

    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    ID3D12PipelineState* pPipelineState = nullptr;
    auto hr = pDevice->CreateComputePipelineState(pDesc, IID_PPV_ARGS(&pPipelineState));
    if (FAILED(hr))
    {
        if (pPipelineState != nullptr)
        {
            pPipelineState->Release();
            pPipelineState = nullptr;
        }
        ELOG("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
        return false;
    }

    m_PipelineStates[hash] = pPipelineState;

    PipelineStateDesc desc;
    desc.Type    = PIPELINE_TYPE_COMPUTE;
    desc.Compute = (*pDesc);

    if (pDesc->CS.pShaderBytecode != nullptr && pDesc->CS.BytecodeLength > 0)
    {
        desc.CS.resize(pDesc->CS.BytecodeLength);
        memcpy(desc.CS.data(), pDesc->CS.pShaderBytecode, pDesc->CS.BytecodeLength);
        desc.Compute.CS.pShaderBytecode = desc.CS.data();
        desc.Compute.CS.BytecodeLength  = desc.CS.size();
    }

    m_Descs[hash] = desc;
    handle = hash;

    return true;
}

//-----------------------------------------------------------------------------
//      メッシュシェーダパイプラインステートを生成します.
//-----------------------------------------------------------------------------
bool PipelineStateManager::Create(const MESH_SHADER_PIPELINE_STATE_DESC* pDesc, PipelineStateHandle& handle)
{
    if (pDesc == nullptr)
    {
        ELOG("Error : Invalid Arguments.");
        return false;
    }

    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

#ifdef ASDX_ENABLE_MESH_SHADER
    // シェーダモデルをチェック.
    {
        D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_5 };
        auto hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel));
        if (FAILED(hr) || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_5))
        {
            ELOG("Error : Shader Model 6.5 is not supported.");
            return false;
        }
    }

    // メッシュシェーダをサポートしているかどうかチェック.
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS7 features = {};
        auto hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &features, sizeof(features));
        if (FAILED(hr) || (features.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED))
        {
            ELOG("Error : Mesh Shaders aren't supported.");
            return false;
        }
    }
#endif

    // ハッシュ値を計算.
    auto hash = CalcDescHash(pDesc);

    // 既に作成済みかどうかチェック.
    if (m_PipelineStates.find(hash) != m_PipelineStates.end())
    {
        ELOG("Error : Already Registered.");
        return false;
    }

    MsPsoDesc msPsoDesc(pDesc);

    D3D12_PIPELINE_STATE_STREAM_DESC pssDesc = {};
    pssDesc.SizeInBytes = sizeof(msPsoDesc);
    pssDesc.pPipelineStateSubobjectStream = &msPsoDesc;

    // パイプラインステート生成.
    ID3D12PipelineState* pPipelineState = nullptr;
    auto hr = pDevice->CreatePipelineState(&pssDesc, IID_PPV_ARGS(&pPipelineState));
    if (FAILED(hr))
    {
        if (pPipelineState != nullptr)
        {
            pPipelineState->Release();
            pPipelineState = nullptr;
        }

        ELOG("Error : ID3D12Device::CreatePipelineState() Failed. errcode = 0x%x", hr);
        return false;
    }

    m_PipelineStates[hash] = pPipelineState;

    PipelineStateDesc desc;
    desc.Type       = PIPELINE_TYPE_MESH_SHADER;
    desc.MeshShader = (*pDesc);

    if (pDesc->AS.pShaderBytecode != nullptr && pDesc->AS.BytecodeLength > 0)
    {
        desc.AS.resize(pDesc->AS.BytecodeLength);
        memcpy(desc.AS.data(), pDesc->AS.pShaderBytecode, pDesc->AS.BytecodeLength);
        desc.MeshShader.AS.pShaderBytecode = desc.AS.data();
        desc.MeshShader.AS.BytecodeLength  = desc.AS.size();
    }
    if (pDesc->MS.pShaderBytecode != nullptr && pDesc->MS.BytecodeLength > 0)
    {
        desc.MS.resize(pDesc->MS.BytecodeLength);
        memcpy(desc.MS.data(), pDesc->MS.pShaderBytecode, pDesc->MS.BytecodeLength);
        desc.MeshShader.MS.pShaderBytecode = desc.MS.data();
        desc.MeshShader.MS.BytecodeLength  = desc.MS.size();
    }
    if (pDesc->PS.pShaderBytecode != nullptr && pDesc->PS.BytecodeLength > 0)
    {
        desc.PS.resize(pDesc->PS.BytecodeLength);
        memcpy(desc.PS.data(), pDesc->PS.pShaderBytecode, pDesc->PS.BytecodeLength);
        desc.MeshShader.PS.pShaderBytecode = desc.PS.data();
        desc.MeshShader.PS.BytecodeLength  = desc.PS.size();
    }

    m_Descs[hash] = desc;
    handle = hash;

    return true;
}

//-----------------------------------------------------------------------------
//      パイプラインステートを検索します.
//-----------------------------------------------------------------------------
ID3D12PipelineState* PipelineStateManager::FindPipelineState(const PipelineStateHandle& handle)
{
    auto itr = m_PipelineStates.find(handle);
    if (itr != m_PipelineStates.end())
        return itr->second;

    return nullptr;
}

//-----------------------------------------------------------------------------
//      クリア処理を行います.
//-----------------------------------------------------------------------------
void PipelineStateManager::Clear()
{
    for(auto& itr : m_PipelineStates)
    {
        auto pso = itr.second;
        itr.second = nullptr;

        if (pso != nullptr)
        {
            pso->Release();
            pso = nullptr;
        }
    }

    m_PipelineStates.clear();
    m_Descs.clear();
}

#if ASDX_ENABLE_PIPELINE_STATE_RELOAD
//-----------------------------------------------------------------------------
//      シェーダを登録します.
//-----------------------------------------------------------------------------
ShaderHandle PipelineStateManager::RegisterShader(SHADER_TYPE type, const char* path, uint32_t pipelineCounts, const PipelineStateHandle* handles)
{
    auto fullPath = ToFullPathA(path);
    auto hash     = CalcHash(fullPath.data(), fullPath.length());
    auto itr      = m_Reloads.find(hash);

    // 未登録であれば情報を設定.
    if (itr == m_Reloads.end())
    {
        m_Reloads[hash] = Reload(type, fullPath, pipelineCounts, handles);
    }
#if ASDX_DEBUG
    // 登録済みであればデバッグチェック.
    else
    {
        const auto& reload = m_Reloads[hash];
        assert(type   == reload.Type);
        assert(path   == reload.Path);
        assert(size_t(pipelineCounts) == reload.Handles.size());
        for(size_t i=0; i<reload.Handles.size(); ++i)
        { assert(handles[i] == reload.Handles[i]); }
    }
#endif

    return hash;
}

//-----------------------------------------------------------------------------
//      シェーダの登録を解除します.
//-----------------------------------------------------------------------------
void PipelineStateManager::UnregisterShader(ShaderHandle handle)
{
    auto itr = m_Reloads.find(handle);
    if (itr != m_Reloads.end())
    { m_Reloads.erase(itr); }
}

//-----------------------------------------------------------------------------
//      インクルードを登録します.
//-----------------------------------------------------------------------------
ShaderHandle PipelineStateManager::RegisterInclude(const char* path, uint32_t shaderCount, const ShaderHandle* handles)
{
    auto fullPath = ToFullPathA(path);
    auto hash     = CalcHash(fullPath.data(), fullPath.length());
    auto itr      = m_Dependencies.find(hash);

    if (itr == m_Dependencies.end())
    {
        m_Dependencies[hash] = Dependency(path, shaderCount, handles);
    }
#if ASDX_DEBUG
    else
    {
        const auto& dependency = m_Dependencies[hash];
        assert(path == dependency.Path);
        assert(size_t(shaderCount) == dependency.Shaders.size());
        for(size_t i=0; i<dependency.Shaders.size(); ++i)
        { assert(handles[i] == dependency.Shaders[i]); }
    }
#endif

    return hash;
}

//-----------------------------------------------------------------------------
//      インクルードの登録を解除します.
//-----------------------------------------------------------------------------
void PipelineStateManager::UnregisterInclude(ShaderHandle handle)
{
    auto itr = m_Dependencies.find(handle);
    if (itr != m_Dependencies.end())
    { m_Dependencies.erase(itr); }
}

//-----------------------------------------------------------------------------
//      インクルードディレクトリを追加します.
//-----------------------------------------------------------------------------
void PipelineStateManager::AddIncludeDirs(const char* dirPath)
{
    auto fullPath = ToFullPathA(dirPath);
    m_IncludeDirs.emplace_back(fullPath);
}

//-----------------------------------------------------------------------------
//      パイプラインステートを検索します.
//-----------------------------------------------------------------------------
ID3D12PipelineState* PipelineStateManager::FindPipelineStateEx(const PipelineStateHandle& handle)
{
    {
        auto itr = m_ReloadPipelineStates.find(handle);
        if (itr != m_ReloadPipelineStates.end())
            return itr->second;
    }

    return FindPipelineState(handle);
}

//-----------------------------------------------------------------------------
//      クリア処理を行います.
//-----------------------------------------------------------------------------
void PipelineStateManager::ClearEx()
{
    for(auto& itr : m_ReloadPipelineStates)
    {
        auto pso = itr.second;
        itr.second = nullptr;

        if (pso != nullptr)
        {
            pso->Release();
            pso = nullptr;
        }
    }
    m_ReloadPipelineStates.clear();
    m_Reloads             .clear();
    m_Dependencies        .clear();
    m_RequestDescs        .clear();

    Clear();
}

//-----------------------------------------------------------------------------
//      再生成をリクエストします.
//-----------------------------------------------------------------------------
void PipelineStateManager::RequestRecreate(PipelineStateHandle pipelineHandle, ShaderHandle shaderHandle)
{
    // 登録されているかチェック.
    auto itrDesc = m_Descs.find(pipelineHandle);
    if (itrDesc == m_Descs.end())
        return;

    // シェーダリロード設定に存在するかどうかチェック.
    auto itrShader = m_Reloads.find(shaderHandle);
    if (itrShader == m_Reloads.end())
        return;

    // 未登録なら構成データを設定.
    auto itrReq = m_RequestDescs.find(pipelineHandle);
    if (itrReq == m_RequestDescs.end())
    { m_RequestDescs[pipelineHandle] = itrDesc->second; }

    switch(itrShader->second.Type)
    {
        case SHADER_TYPE_VS: { CompileFromFileA(itrShader->second.Path.c_str(), m_IncludeDirs, "main", "vs_6_5", itrReq->second.VS); } break;
        case SHADER_TYPE_PS: { CompileFromFileA(itrShader->second.Path.c_str(), m_IncludeDirs, "main", "ps_6_5", itrReq->second.PS); } break;
        case SHADER_TYPE_HS: { CompileFromFileA(itrShader->second.Path.c_str(), m_IncludeDirs, "main", "hs_6_6", itrReq->second.HS); } break;
        case SHADER_TYPE_DS: { CompileFromFileA(itrShader->second.Path.c_str(), m_IncludeDirs, "main", "ds_6_5", itrReq->second.DS); } break;
        case SHADER_TYPE_GS: { CompileFromFileA(itrShader->second.Path.c_str(), m_IncludeDirs, "main", "gs_6_5", itrReq->second.GS); } break;
        case SHADER_TYPE_AS: { CompileFromFileA(itrShader->second.Path.c_str(), m_IncludeDirs, "main", "as_6_5", itrReq->second.AS); } break;
        case SHADER_TYPE_MS: { CompileFromFileA(itrShader->second.Path.c_str(), m_IncludeDirs, "main", "ms_6_5", itrReq->second.MS); } break;
        case SHADER_TYPE_CS: { CompileFromFileA(itrShader->second.Path.c_str(), m_IncludeDirs, "main", "cs_6_5", itrReq->second.CS); } break;
        default: break;
    }
}

//-----------------------------------------------------------------------------
//      ファイル変更時の処理です.
//-----------------------------------------------------------------------------
void PipelineStateManager::OnChanged(const FileEventArgs& args)
{
    auto hash = CalcHash(args.FullPath.data(), args.FullPath.length());

    // インクルードファイルに存在するかどうかチェック.
    {
        auto itr = m_Dependencies.find(hash);
        if (itr != m_Dependencies.end())
        {
            // 依存するシェーダをチェック.
            for(const auto& pipelineHandle : itr->second.Shaders)
            { RequestRecreate(pipelineHandle, itr->first); }
        }
    }

    // シェーダファイルに存在するかどうかチェック.
    {
        auto itr = m_Reloads.find(hash);
        if (itr != m_Reloads.end())
        {
            for(const auto& pipelineHandle : itr->second.Handles)
            { RequestRecreate(pipelineHandle, itr->first); }
        }
    }
}

//-----------------------------------------------------------------------------
//      グラフィックスパイプラインステートの再生成を行います.
//-----------------------------------------------------------------------------
void PipelineStateManager::Recreate(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, const PipelineStateHandle& handle)
{
    if (pDesc == nullptr)
    {
        ELOG("Error : Invalid Arguments.");
        return;
    }

    // 既に作成済みかどうかチェック.
    auto itr = m_ReloadPipelineStates.find(handle);
    if (itr != m_ReloadPipelineStates.end())
    {
        auto pso = itr->second;
        itr->second = nullptr;

        // 遅延解放.
        Dispose(pso);
    }

    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    PipelineStateDesc desc;
    desc.Type     = PIPELINE_TYPE_GRAPHICS;
    desc.Graphics = (*pDesc);

    if (pDesc->VS.pShaderBytecode != nullptr && pDesc->VS.BytecodeLength > 0)
    {
        desc.VS.resize(pDesc->VS.BytecodeLength);
        memcpy(desc.VS.data(), pDesc->VS.pShaderBytecode, pDesc->VS.BytecodeLength);
        desc.Graphics.VS.pShaderBytecode = desc.VS.data();
        desc.Graphics.VS.BytecodeLength  = desc.VS.size();
    }
    if (pDesc->PS.pShaderBytecode != nullptr && pDesc->PS.BytecodeLength > 0)
    {
        desc.PS.resize(pDesc->PS.BytecodeLength);
        memcpy(desc.PS.data(), pDesc->PS.pShaderBytecode, pDesc->PS.BytecodeLength);
        desc.Graphics.PS.pShaderBytecode = desc.PS.data();
        desc.Graphics.PS.BytecodeLength  = desc.PS.size();
    }
    if (pDesc->DS.pShaderBytecode != nullptr && pDesc->DS.BytecodeLength > 0)
    {
        desc.DS.resize(pDesc->DS.BytecodeLength);
        memcpy(desc.DS.data(), pDesc->DS.pShaderBytecode, pDesc->DS.BytecodeLength);
        desc.Graphics.DS.pShaderBytecode = desc.DS.data();
        desc.Graphics.DS.BytecodeLength  = desc.DS.size();
    }
    if (pDesc->HS.pShaderBytecode != nullptr && pDesc->HS.BytecodeLength > 0)
    {
        desc.HS.resize(pDesc->HS.BytecodeLength);
        memcpy(desc.HS.data(), pDesc->HS.pShaderBytecode, pDesc->HS.BytecodeLength);
        desc.Graphics.HS.pShaderBytecode = desc.HS.data();
        desc.Graphics.HS.BytecodeLength  = desc.HS.size();
    }
    if (pDesc->HS.pShaderBytecode != nullptr && pDesc->HS.BytecodeLength > 0)
    {
        desc.GS.resize(pDesc->GS.BytecodeLength);
        memcpy(desc.GS.data(), pDesc->GS.pShaderBytecode, pDesc->GS.BytecodeLength);
        desc.Graphics.GS.pShaderBytecode = desc.GS.data();
        desc.Graphics.GS.BytecodeLength  = desc.GS.size();
    }

    ID3D12PipelineState* pPipelineState = nullptr;
    auto hr = pDevice->CreateGraphicsPipelineState(&desc.Graphics, IID_PPV_ARGS(&pPipelineState));
    if (FAILED(hr))
    {
        if (pPipelineState != nullptr)
        {
            pPipelineState->Release();
            pPipelineState = nullptr;
        }
        ELOG("Error : ID3D12Device::CreateGraphicsState() Failed. errcode = 0x%x", hr);
        return;
    }

    m_ReloadPipelineStates[handle] = pPipelineState;

    m_Descs[handle] = desc;
}

//-----------------------------------------------------------------------------
//      コンピュートパイプラインステートを再生成します.
//-----------------------------------------------------------------------------
void PipelineStateManager::Recreate(const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc, const PipelineStateHandle& handle)
{
    if (pDesc == nullptr)
    {
        ELOG("Error : Invalid Arguments.");
        return;
    }

    auto itr = m_ReloadPipelineStates.find(handle);
    if (itr != m_ReloadPipelineStates.end())
    {
        auto pso = itr->second;
        itr->second = nullptr;

        // 遅延解放.
        Dispose(pso);
    }

    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);


    PipelineStateDesc desc;
    desc.Type    = PIPELINE_TYPE_COMPUTE;
    desc.Compute = (*pDesc);

    if (pDesc->CS.pShaderBytecode != nullptr && pDesc->CS.BytecodeLength > 0)
    {
        desc.CS.resize(pDesc->CS.BytecodeLength);
        memcpy(desc.CS.data(), pDesc->CS.pShaderBytecode, pDesc->CS.BytecodeLength);
        desc.Compute.CS.pShaderBytecode = desc.CS.data();
        desc.Compute.CS.BytecodeLength  = desc.CS.size();
    }

    ID3D12PipelineState* pPipelineState = nullptr;
    auto hr = pDevice->CreateComputePipelineState(&desc.Compute, IID_PPV_ARGS(&pPipelineState));
    if (FAILED(hr))
    {
        if (pPipelineState != nullptr)
        {
            pPipelineState->Release();
            pPipelineState = nullptr;
        }
        ELOG("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
        return;
    }

    m_ReloadPipelineStates[handle] = pPipelineState;

    m_Descs[handle] = desc;
}

//-----------------------------------------------------------------------------
//      メッシュシェーダパイプラインステートを再生成します.
//-----------------------------------------------------------------------------
void PipelineStateManager::Recreate(const MESH_SHADER_PIPELINE_STATE_DESC* pDesc, const PipelineStateHandle& handle)
{
    if (pDesc == nullptr)
    {
        ELOG("Error : Invalid Arguments.");
        return;
    }

    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

#ifdef ASDX_ENABLE_MESH_SHADER
    // シェーダモデルをチェック.
    {
        D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_5 };
        auto hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel));
        if (FAILED(hr) || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_5))
        {
            ELOG("Error : Shader Model 6.5 is not supported.");
            return;
        }
    }

    // メッシュシェーダをサポートしているかどうかチェック.
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS7 features = {};
        auto hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &features, sizeof(features));
        if (FAILED(hr) || (features.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED))
        {
            ELOG("Error : Mesh Shaders aren't supported.");
            return;
        }
    }
#endif


    PipelineStateDesc desc;
    desc.Type       = PIPELINE_TYPE_MESH_SHADER;
    desc.MeshShader = (*pDesc);

    // 既に作成済みかどうかチェック.
    auto itr = m_ReloadPipelineStates.find(handle);
    if (itr != m_ReloadPipelineStates.end())
    {
        auto pso = itr->second;
        itr->second = nullptr;

        // 遅延解放.
        Dispose(pso);
    }

    if (pDesc->AS.pShaderBytecode != nullptr && pDesc->AS.BytecodeLength > 0)
    {
        desc.AS.resize(pDesc->AS.BytecodeLength);
        memcpy(desc.AS.data(), pDesc->AS.pShaderBytecode, pDesc->AS.BytecodeLength);
        desc.MeshShader.AS.pShaderBytecode = desc.AS.data();
        desc.MeshShader.AS.BytecodeLength  = desc.AS.size();
    }
    if (pDesc->MS.pShaderBytecode != nullptr && pDesc->MS.BytecodeLength > 0)
    {
        desc.MS.resize(pDesc->MS.BytecodeLength);
        memcpy(desc.MS.data(), pDesc->MS.pShaderBytecode, pDesc->MS.BytecodeLength);
        desc.MeshShader.MS.pShaderBytecode = desc.MS.data();
        desc.MeshShader.MS.BytecodeLength  = desc.MS.size();
    }
    if (pDesc->PS.pShaderBytecode != nullptr && pDesc->PS.BytecodeLength > 0)
    {
        desc.PS.resize(pDesc->PS.BytecodeLength);
        memcpy(desc.PS.data(), pDesc->PS.pShaderBytecode, pDesc->PS.BytecodeLength);
        desc.MeshShader.PS.pShaderBytecode = desc.PS.data();
        desc.MeshShader.PS.BytecodeLength  = desc.PS.size();
    }

    MsPsoDesc msPsoDesc(&desc.MeshShader);

    D3D12_PIPELINE_STATE_STREAM_DESC pssDesc = {};
    pssDesc.SizeInBytes = sizeof(msPsoDesc);
    pssDesc.pPipelineStateSubobjectStream = &msPsoDesc;

    // パイプラインステート生成.
    ID3D12PipelineState* pPipelineState = nullptr;
    auto hr = pDevice->CreatePipelineState(&pssDesc, IID_PPV_ARGS(&pPipelineState));
    if (FAILED(hr))
    {
        if (pPipelineState != nullptr)
        {
            pPipelineState->Release();
            pPipelineState = nullptr;
        }

        ELOG("Error : ID3D12Device::CreatePipelineState() Failed. errcode = 0x%x", hr);
        return;
    }

    m_ReloadPipelineStates[handle] = pPipelineState;

    m_Descs[handle] = desc;
}

//-----------------------------------------------------------------------------
//      更新処理を行います.
//-----------------------------------------------------------------------------
void PipelineStateManager::Update()
{
    if (m_RequestDescs.empty())
        return;

    for(const auto& desc : m_RequestDescs)
    {
        switch(desc.second.Type)
        {
            case PIPELINE_TYPE_GRAPHICS    : { Recreate(&desc.second.Graphics  , desc.first); } break;
            case PIPELINE_TYPE_COMPUTE     : { Recreate(&desc.second.Compute   , desc.first); } break;
            case PIPELINE_TYPE_MESH_SHADER : { Recreate(&desc.second.MeshShader, desc.first); } break;
            default: break;
        }
    }

    m_RequestDescs.clear();
}

#endif

///////////////////////////////////////////////////////////////////////////////
// Functions.
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      SRVレンジとして初期化します.
//-----------------------------------------------------------------------------
void InitRangeAsSRV(D3D12_DESCRIPTOR_RANGE& range, UINT registerIndex, UINT count, UINT registerSpace)
{
    range.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range.NumDescriptors                    = count;
    range.BaseShaderRegister                = registerIndex;
    range.RegisterSpace                     = registerSpace;
    range.OffsetInDescriptorsFromTableStart = 0;
}

//-----------------------------------------------------------------------------
//      UAVレンジとして初期化します.
//-----------------------------------------------------------------------------
void InitRangeAsUAV(D3D12_DESCRIPTOR_RANGE& range, UINT registerIndex, UINT count, UINT registerSpace)
{
    range.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    range.NumDescriptors                    = count;
    range.BaseShaderRegister                = registerIndex;
    range.RegisterSpace                     = registerSpace;
    range.OffsetInDescriptorsFromTableStart = 0;
}

//-----------------------------------------------------------------------------
//      ルート定数として初期化します.
//-----------------------------------------------------------------------------
void InitAsConstants(D3D12_ROOT_PARAMETER& param, UINT registerIndex, UINT count, D3D12_SHADER_VISIBILITY visibility, UINT registerSpace)
{
    param.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    param.Constants.Num32BitValues  = count;
    param.Constants.ShaderRegister  = registerIndex;
    param.Constants.RegisterSpace   = registerSpace;
    param.ShaderVisibility          = visibility;
}

//-----------------------------------------------------------------------------
//      定数バッファとして初期化します.
//-----------------------------------------------------------------------------
void InitAsCBV(D3D12_ROOT_PARAMETER& param, UINT registerIndex, D3D12_SHADER_VISIBILITY visibility, UINT registerSpace)
{
    param.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    param.Descriptor.ShaderRegister = registerIndex;
    param.Descriptor.RegisterSpace  = registerSpace;
    param.ShaderVisibility          = visibility;
}

//-----------------------------------------------------------------------------
//      SRVとして初期化します.
//-----------------------------------------------------------------------------
void InitAsSRV(D3D12_ROOT_PARAMETER& param, UINT registerIndex, D3D12_SHADER_VISIBILITY visibility, UINT registerSpace)
{
    param.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_SRV;
    param.Descriptor.ShaderRegister = registerIndex;
    param.Descriptor.RegisterSpace  = registerSpace;
    param.ShaderVisibility          = visibility;
}

//-----------------------------------------------------------------------------
//      UAVとして初期化します.
//-----------------------------------------------------------------------------
void InitAsUAV(D3D12_ROOT_PARAMETER& param, UINT registerIndex, D3D12_SHADER_VISIBILITY visibility, UINT registerSpace)
{
    param.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_UAV;
    param.Descriptor.ShaderRegister = registerIndex;
    param.Descriptor.RegisterSpace  = registerSpace;
    param.ShaderVisibility          = visibility;
}

//-----------------------------------------------------------------------------
//      ディスクリプタテーブルとして初期化します.
//-----------------------------------------------------------------------------
void InitAsTable(
    D3D12_ROOT_PARAMETER&           param,
    UINT                            count,
    const D3D12_DESCRIPTOR_RANGE*   range,
    D3D12_SHADER_VISIBILITY         visibility
)
{
    param.ParameterType                         = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param.DescriptorTable.NumDescriptorRanges   = 1;
    param.DescriptorTable.pDescriptorRanges     = range;
    param.ShaderVisibility                      = visibility;
}

//-----------------------------------------------------------------------------
//      ルートシグニチャを初期化します.
//-----------------------------------------------------------------------------
bool InitRootSignature
(
    ID3D12Device*                       pDevice,
    const D3D12_ROOT_SIGNATURE_DESC*    pDesc,
    ID3D12RootSignature**               ppRootSig
)
{
    asdx::RefPtr<ID3DBlob> blob;
    asdx::RefPtr<ID3DBlob> errorBlob;
    auto hr = D3D12SerializeRootSignature(
        pDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, blob.GetAddress(), errorBlob.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : D3D12SerializeRootSignature() Failed. errcode = 0x%x", hr);
        if (errorBlob) {
            ELOG("Error : msg = %s", reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
        }
        return false;
    }

    hr = pDevice->CreateRootSignature(
        0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(ppRootSig));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateRootSignature() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

} // namespace asdx
