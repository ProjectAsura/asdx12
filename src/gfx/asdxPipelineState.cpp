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
#include <list>
#include <d3d12shader.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMisc.h>
#include <fnd/asdxPath.h>
#include <gfx/asdxPipelineState.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxShaderCompiler.h>
#include <edit/asdxFileWatcher.h>


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

///////////////////////////////////////////////////////////////////////////////
// PipelineStateListener class
///////////////////////////////////////////////////////////////////////////////
class PipelineStateListener : public asdx::IFileListener
{
public:
    PipelineStateListener()
    { /* DO_NOTHING */ }

    ~PipelineStateListener()
    { Reset(); }

    void Reset()
    {
        m_Graphics   .clear();
        m_Computes   .clear();
        m_MeshShaders.clear();
        m_RayTracings.clear();
    }

    void AddGraphics(asdx::GraphicsPipelineState* item)
    {
        m_Graphics.push_back(item);
    }

    void AddCompute(asdx::ComputePipelineState* item)
    {
        m_Computes.push_back(item);
    }

    void AddMeshShader(asdx::MeshShaderPipelineState* item)
    {
        m_MeshShaders.push_back(item);
    }

    void AddRayTracing(asdx::RayTracingPipelineState* item)
    {
        m_RayTracings.push_back(item);
    }

    void RemoveGraphics(asdx::GraphicsPipelineState* item)
    {
        m_Graphics.remove(item);
    }

    void RemoveCompute(asdx::ComputePipelineState* item)
    {
        m_Computes.remove(item);
    }

    void RemoveMeshShader(asdx::MeshShaderPipelineState* item)
    {
        m_MeshShaders.remove(item);
    }

    void RemoveRayTracing(asdx::RayTracingPipelineState* item)
    {
        m_RayTracings.remove(item);
    }

    void OnChanged(const std::vector<asdx::FileEventArgs>& args) override
    {
        for(const auto& arg : args)
        {
            if (arg.Type == asdx::FileEventArgs::Modified ||
                arg.Type == asdx::FileEventArgs::RenamedNewName)
            {
                for(auto& itr : m_Graphics)
                { itr->OnReload(arg.FullPath); }

                for(auto& itr : m_Computes)
                { itr->OnReload(arg.FullPath); }

                for(auto& itr : m_MeshShaders)
                { itr->OnReload(arg.FullPath); }

                for(auto& itr : m_RayTracings)
                { itr->OnReload(arg.FullPath); }
            }
        }
    }

private:
    std::list<asdx::GraphicsPipelineState*>   m_Graphics;
    std::list<asdx::ComputePipelineState*>    m_Computes;
    std::list<asdx::MeshShaderPipelineState*> m_MeshShaders;
    std::list<asdx::RayTracingPipelineState*> m_RayTracings;
};

std::vector<std::string>    g_Includes;
asdx::FileWatcher           g_Watcher;
PipelineStateListener       g_Listener;
bool                        g_Initialized = false;

//-----------------------------------------------------------------------------
//      シェーダコードからBlobを設定します.
//-----------------------------------------------------------------------------
void SetBlob(asdx::ShaderInfo& info, D3D12_SHADER_BYTECODE& code)
{
    if (code.pShaderBytecode == nullptr || code.BytecodeLength == 0)
        return;

    info.Blob.resize(code.BytecodeLength);
    memcpy(info.Blob.data(), code.pShaderBytecode, code.BytecodeLength);
    code.pShaderBytecode = info.Blob.data();
}

//-----------------------------------------------------------------------------
//      Blobでシェーダコードを置き換えます.
//-----------------------------------------------------------------------------
void ReplaceCode(asdx::ShaderInfo& info, D3D12_SHADER_BYTECODE& code)
{
    code.pShaderBytecode = info.Blob.data();
    code.BytecodeLength  = info.Blob.size();
}

//-----------------------------------------------------------------------------
//      シェーダテーブルを生成します.
//-----------------------------------------------------------------------------
bool CreateShaderTable
(
    ID3D12Device*           pDevice,
    std::vector<void*>      shaderIdentifiers,
    ID3D12Resource**        ppResource
)
{
    auto isGpuUploadHeap = false;

    D3D12_FEATURE_DATA_D3D12_OPTIONS16 options16 = {};
    if (SUCCEEDED(pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS16, &options16, sizeof(options16))))
    {
        if (options16.GPUUploadHeapSupported)
        { isGpuUploadHeap = true; }
    }

    auto recordSize = asdx::RoundUp(
        uint32_t(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES),
        uint32_t(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT));

    auto bufferSize = recordSize * shaderIdentifiers.size();

    D3D12_HEAP_PROPERTIES props = {};
    props.Type                  = (isGpuUploadHeap) ? D3D12_HEAP_TYPE_GPU_UPLOAD : D3D12_HEAP_TYPE_UPLOAD;
    props.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
    props.CreationNodeMask      = 1;
    props.VisibleNodeMask       = 1;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment          = 0;
    desc.Width              = bufferSize;
    desc.Height             = 1;
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.Format             = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    auto hr = pDevice->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(ppResource));
    if (FAILED(hr))
    {
        ELOGA("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
        return false;
    }

    uint8_t* ptr = nullptr;
    hr = (*ppResource)->Map(0, nullptr, reinterpret_cast<void**>(&ptr));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
        return false;
    }

    for(size_t i=0u; i<shaderIdentifiers.size(); ++i)
    {
        memcpy(ptr, shaderIdentifiers[i], D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
        ptr += recordSize;
    }
    (*ppResource)->Unmap(0, nullptr);

    return true;
}


} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// GraphicsPipelineState class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
GraphicsPipelineState::GraphicsPipelineState()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
GraphicsPipelineState::~GraphicsPipelineState()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool GraphicsPipelineState::Init(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc)
{
    auto pDevice = GetD3D12Device();

    auto hr = pDevice->CreateGraphicsPipelineState(pDesc, IID_PPV_ARGS(m_State.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
        return false;
    }

    m_Desc = *pDesc;
    SetBlob(m_VS, m_Desc.VS);
    SetBlob(m_PS, m_Desc.PS);
    SetBlob(m_HS, m_Desc.HS);
    SetBlob(m_DS, m_Desc.DS);
    SetBlob(m_GS, m_Desc.GS);

    g_Listener.AddGraphics(this);
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void GraphicsPipelineState::Term()
{
    g_Listener.RemoveGraphics(this);
    m_State.Reset();
    m_ReloadState.Reset();
}

//-----------------------------------------------------------------------------
//      パイプラインステートを設定します.
//-----------------------------------------------------------------------------
void GraphicsPipelineState::SetState(ID3D12GraphicsCommandList* pCmd)
{
    if (pCmd == nullptr)
        return;

    if (m_Dirty)
    { Recreate(); }

    if (m_ReloadState.GetPtr() != nullptr)
    {
        pCmd->SetPipelineState(m_ReloadState.GetPtr());
        return;
    }
 
    pCmd->SetPipelineState(m_State.GetPtr());
}

//-----------------------------------------------------------------------------
//      頂点シェーダのリロードファイルパスを設定します.
//-----------------------------------------------------------------------------
void GraphicsPipelineState::SetReloadPathVS(const std::string& value)
{ m_VS.Path = ToFullPath(value).string(); }

//-----------------------------------------------------------------------------
//      ピクセルシェーダのリロードファイルパスを設定します.
//-----------------------------------------------------------------------------
void GraphicsPipelineState::SetReloadPathPS(const std::string& value)
{ m_PS.Path = ToFullPath(value).string(); }

//-----------------------------------------------------------------------------
//      ハルシェーダのリロードファイルパスを設定します.
//-----------------------------------------------------------------------------
void GraphicsPipelineState::SetReloadPathHS(const std::string& value)
{ m_HS.Path = ToFullPath(value).string(); }

//-----------------------------------------------------------------------------
//      ドメインシェーダのリロードファイルパスを設定します.
//-----------------------------------------------------------------------------
void GraphicsPipelineState::SetReloadPathDS(const std::string& value)
{ m_DS.Path = ToFullPath(value).string(); }

//-----------------------------------------------------------------------------
//      ジオメトリシェーダのリロードファイルパスを設定します.
//-----------------------------------------------------------------------------
void GraphicsPipelineState::SetReloadPathGS(const std::string& value)
{ m_GS.Path = ToFullPath(value).string(); }

//-----------------------------------------------------------------------------
//      リロード時の処理です.
//-----------------------------------------------------------------------------
void GraphicsPipelineState::OnReload(const std::string& fullPath)
{
    if (fullPath.empty())
        return;

    if (m_VS.Path == fullPath)
    {
        std::vector<uint8_t> binary;
        if (CompileFromFileA(fullPath.c_str(), g_Includes, "main", "vs_6_6", binary))
        {
            m_VS.Blob = std::move(binary);
            ReplaceCode(m_VS, m_Desc.VS);
            m_Dirty = true;
        }
    }
    if (m_PS.Path == fullPath)
    {
        std::vector<uint8_t> binary;
        if (CompileFromFileA(fullPath.c_str(), g_Includes, "main", "ps_6_6", binary))
        {
            m_PS.Blob = std::move(binary);
            ReplaceCode(m_PS, m_Desc.PS);
            m_Dirty = true;
        }
    }
    if (m_HS.Path == fullPath)
    {
        std::vector<uint8_t> binary;
        if (CompileFromFileA(fullPath.c_str(), g_Includes, "main", "hs_6_6", binary))
        {
            m_HS.Blob = std::move(binary);
            ReplaceCode(m_HS, m_Desc.HS);
            m_Dirty = true;
        }
    }
    if (m_DS.Path == fullPath)
    {
        std::vector<uint8_t> binary;
        if (CompileFromFileA(fullPath.c_str(), g_Includes, "main", "ds_6_6", binary))
        {
            m_DS.Blob = std::move(binary);
            ReplaceCode(m_DS, m_Desc.DS);
            m_Dirty = true;
        }
    }
    if (m_GS.Path == fullPath)
    {
        std::vector<uint8_t> binary;
        if (CompileFromFileA(fullPath.c_str(), g_Includes, "main", "gs_6_6", binary))
        {
            m_GS.Blob = std::move(binary);
            ReplaceCode(m_GS, m_Desc.GS);
            m_Dirty = true;
        }
    }
}

//-----------------------------------------------------------------------------
//      強制リロードを行います.
//-----------------------------------------------------------------------------
void GraphicsPipelineState::ForceReload()
{
    auto changed = false;

    if (!m_VS.Path.empty())
    {
        std::vector<uint8_t> binary;
        if (CompileFromFileA(m_VS.Path.c_str(), g_Includes, "main", "vs_6_6", binary))
        {
            m_VS.Blob = std::move(binary);
            ReplaceCode(m_VS, m_Desc.VS);
            changed = true;
        }
    }
    if (!m_PS.Path.empty())
    {
        std::vector<uint8_t> binary;
        if (CompileFromFileA(m_PS.Path.c_str(), g_Includes, "main", "ps_6_6", binary))
        {
            m_PS.Blob = std::move(binary);
            ReplaceCode(m_PS, m_Desc.PS);
            changed = true;
        }
    }
    if (!m_HS.Path.empty())
    {
        std::vector<uint8_t> binary;
        if (CompileFromFileA(m_HS.Path.c_str(), g_Includes, "main", "hs_6_6", binary))
        {
            m_HS.Blob = std::move(binary);
            ReplaceCode(m_HS, m_Desc.HS);
            changed = true;
        }
    }
    if (!m_DS.Path.empty())
    {
        std::vector<uint8_t> binary;
        if (CompileFromFileA(m_DS.Path.c_str(), g_Includes, "main", "ds_6_6", binary))
        {
            m_DS.Blob = std::move(binary);
            ReplaceCode(m_DS, m_Desc.DS);
            changed = true;
        }
    }
    if (!m_GS.Path.empty())
    {
        std::vector<uint8_t> binary;
        if (CompileFromFileA(m_GS.Path.c_str(), g_Includes, "main", "gs_6_6", binary))
        {
            m_GS.Blob = std::move(binary);
            ReplaceCode(m_GS, m_Desc.GS);
            changed = true;
        }
    }

    if (changed)
    {
        m_Dirty = true;
    }
}

//-----------------------------------------------------------------------------
//      再生成処理です.
//-----------------------------------------------------------------------------
void GraphicsPipelineState::Recreate()
{
    if (!m_Dirty)
        return;

    ID3D12PipelineState* pPipelineState = nullptr;

    auto pDevice = GetD3D12Device();
    auto hr = pDevice->CreateGraphicsPipelineState(&m_Desc, IID_PPV_ARGS(&pPipelineState));
    if (FAILED(hr))
    {
        ELOGA("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);

        // 失敗した場合もダーティフラグを下す.
        m_Dirty = false;

        // おしまい.
        return;
    }

    // 遅延解放.
    auto pso = m_ReloadState.Detach();
    Dispose(pso);

    // 差し替え.
    m_ReloadState.Attach(pPipelineState);

    // ダーティフラグを下す.
    m_Dirty = false;
}

///////////////////////////////////////////////////////////////////////////////
// ComputePipelineState class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ComputePipelineState::ComputePipelineState()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ComputePipelineState::~ComputePipelineState()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool ComputePipelineState::Init(const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc)
{
    auto pDevice = GetD3D12Device();

    auto hr = pDevice->CreateComputePipelineState(pDesc, IID_PPV_ARGS(m_State.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
        return false;
    }

    m_Desc = (*pDesc);
    SetBlob(m_CS, m_Desc.CS);
    g_Listener.AddCompute(this);
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理です.
//-----------------------------------------------------------------------------
void ComputePipelineState::Term()
{
    g_Listener.RemoveCompute(this);
    m_ReloadState.Reset();
    m_State.Reset();
}

//-----------------------------------------------------------------------------
//      パイプラインステートを設定します.
//-----------------------------------------------------------------------------
void ComputePipelineState::SetState(ID3D12GraphicsCommandList* pCmd)
{
    if (pCmd == nullptr)
        return;

    if (m_Dirty)
    { Recreate(); }

    if (m_ReloadState.GetPtr() != nullptr)
    {
        pCmd->SetPipelineState(m_ReloadState.GetPtr());
        return;
    }
 
    pCmd->SetPipelineState(m_State.GetPtr());
}

//-----------------------------------------------------------------------------
//      コンピュートシェーダのリロードファイルパスを設定します.
//-----------------------------------------------------------------------------
void ComputePipelineState::SetReloadPathCS(const std::string& value)
{ m_CS.Path = ToFullPath(value).string(); }

//-----------------------------------------------------------------------------
//      リロード時の処理です.
//-----------------------------------------------------------------------------
void ComputePipelineState::OnReload(const std::string& fullPath)
{
    if (fullPath.empty())
        return;

    if (m_CS.Path == fullPath)
    {
        std::vector<uint8_t> binary;
        if (CompileFromFileA(fullPath.c_str(), g_Includes, "main", "cs_6_6", binary))
        {
            m_CS.Blob = std::move(binary);
            ReplaceCode(m_CS, m_Desc.CS);
            m_Dirty = true;
        }
    }
}

//-----------------------------------------------------------------------------
//      強制リロードを行います.
//-----------------------------------------------------------------------------
void ComputePipelineState::ForceReload()
{
    if (!m_CS.Path.empty())
        return;

    std::vector<uint8_t> binary;
    if (CompileFromFileA(m_CS.Path.c_str(), g_Includes, "main", "cs_6_6", binary))
    {
        m_CS.Blob = std::move(binary);
        ReplaceCode(m_CS, m_Desc.CS);
        m_Dirty = true;
    }
}

//-----------------------------------------------------------------------------
//      再生成処理です.
//-----------------------------------------------------------------------------
void ComputePipelineState::Recreate()
{
    if (!m_Dirty)
        return;

    auto pDevice = GetD3D12Device();

    // パイプラインステートを生成.
    ID3D12PipelineState* pPipelineState = nullptr;
    auto hr = pDevice->CreateComputePipelineState(&m_Desc, IID_PPV_ARGS(&pPipelineState));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
        m_Dirty = false;
        return;
    }

    // 遅延解放.
    auto pso = m_ReloadState.Detach();
    Dispose(pso);

    // 差し替え.
    m_ReloadState.Attach(pPipelineState);

    // ダーティフラグを下す.
    m_Dirty = false;
}

///////////////////////////////////////////////////////////////////////////////
// MeshShaderPipelineState class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
MeshShaderPipelineState::MeshShaderPipelineState()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
MeshShaderPipelineState::~MeshShaderPipelineState()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool MeshShaderPipelineState::Init(const MESH_SHADER_PIPELINE_STATE_DESC* pDesc)
{
    auto pDevice = GetD3D12Device();

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
    MsPsoDesc msPsoDesc(pDesc);

    D3D12_PIPELINE_STATE_STREAM_DESC pssDesc = {};
    pssDesc.SizeInBytes = sizeof(msPsoDesc);
    pssDesc.pPipelineStateSubobjectStream = &msPsoDesc;

    // パイプラインステート生成.
    auto hr = pDevice->CreatePipelineState(&pssDesc, IID_PPV_ARGS(m_State.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreatePipelineState() Failed. errcode = 0x%x", hr);
        return false;
    }

    m_Desc = (*pDesc);
    SetBlob(m_AS, m_Desc.AS);
    SetBlob(m_MS, m_Desc.MS);
    SetBlob(m_PS, m_Desc.PS);
    g_Listener.AddMeshShader(this);
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void MeshShaderPipelineState::Term()
{
    g_Listener.RemoveMeshShader(this);
    m_ReloadState.Reset();
    m_State.Reset();
}

//-----------------------------------------------------------------------------
//      パイプラインステートを設定します.
//-----------------------------------------------------------------------------
void MeshShaderPipelineState::SetState(ID3D12GraphicsCommandList* pCmd)
{
    if (m_Dirty)
        Recreate();

    if (m_ReloadState.GetPtr() != nullptr)
    {
        pCmd->SetPipelineState(m_ReloadState.GetPtr());
        return;
    }

    pCmd->SetPipelineState(m_State.GetPtr());
}

//-----------------------------------------------------------------------------
//      増幅シェーダのリロードファイルパスを設定します.
//-----------------------------------------------------------------------------
void MeshShaderPipelineState::SetReloadPathAS(const std::string& value)
{ m_AS.Path = ToFullPath(value).string(); }

//-----------------------------------------------------------------------------
//      メッシュシェーダのリロードファイルパスを設定します.
//-----------------------------------------------------------------------------
void MeshShaderPipelineState::SetReloadPathMS(const std::string& value)
{ m_MS.Path = ToFullPath(value).string(); }

//-----------------------------------------------------------------------------
//      ピクセルシェーダのリロードファイルパスを設定します.
//-----------------------------------------------------------------------------
void MeshShaderPipelineState::SetRelaodPathPS(const std::string& value)
{ m_PS.Path = ToFullPath(value).string(); }

//-----------------------------------------------------------------------------
//      リロード時の処理です.
//-----------------------------------------------------------------------------
void MeshShaderPipelineState::OnReload(const std::string& fullPath)
{
    if (fullPath.empty())
        return;

    if (m_AS.Path == fullPath)
    {
        std::vector<uint8_t> binary;
        if (CompileFromFileA(fullPath.c_str(), g_Includes, "main", "as_6_6", binary))
        {
            m_AS.Blob = std::move(binary);
            ReplaceCode(m_AS, m_Desc.AS);
            m_Dirty = true;
        }
    }
    if (m_MS.Path == fullPath)
    {
        std::vector<uint8_t> binary;
        if (CompileFromFileA(fullPath.c_str(), g_Includes, "main", "ms_6_6", binary))
        {
            m_MS.Blob = std::move(binary);
            ReplaceCode(m_MS, m_Desc.MS);
            m_Dirty = true;
        }
    }
    if (m_PS.Path == fullPath)
    {
        std::vector<uint8_t> binary;
        if (CompileFromFileA(fullPath.c_str(), g_Includes, "main", "ps_6_6", binary))
        {
            m_PS.Blob = std::move(binary);
            ReplaceCode(m_PS, m_Desc.PS);
            m_Dirty = true;
        }
    }
}

//-----------------------------------------------------------------------------
//      強制リロードを行います.
//-----------------------------------------------------------------------------
void MeshShaderPipelineState::ForceReload()
{
    auto changed = false;

    if (!m_AS.Path.empty())
    {
        std::vector<uint8_t> binary;
        if (CompileFromFileA(m_AS.Path.c_str(), g_Includes, "main", "as_6_6", binary))
        {
            m_AS.Blob = std::move(binary);
            ReplaceCode(m_AS, m_Desc.AS);
            changed = true;
        }
    }
    if (!m_MS.Path.empty())
    {
        std::vector<uint8_t> binary;
        if (CompileFromFileA(m_MS.Path.c_str(), g_Includes, "main", "ms_6_6", binary))
        {
            m_MS.Blob = std::move(binary);
            ReplaceCode(m_MS, m_Desc.MS);
            changed = true;
        }
    }
    if (!m_PS.Path.empty())
    {
        std::vector<uint8_t> binary;
        if (CompileFromFileA(m_PS.Path.c_str(), g_Includes, "main", "ps_6_6", binary))
        {
            m_PS.Blob = std::move(binary);
            ReplaceCode(m_PS, m_Desc.PS);
            changed = true;
        }
    }

    if (changed)
    {
        m_Dirty = true;
    }
}

//-----------------------------------------------------------------------------
//      再生成処理です.
//-----------------------------------------------------------------------------
void MeshShaderPipelineState::Recreate()
{
    if (!m_Dirty)
        return;

    auto pDevice = GetD3D12Device();

    MsPsoDesc msPsoDesc(&m_Desc);

    D3D12_PIPELINE_STATE_STREAM_DESC pssDesc = {};
    pssDesc.SizeInBytes = sizeof(msPsoDesc);
    pssDesc.pPipelineStateSubobjectStream = &msPsoDesc;

    // パイプラインステート生成.
    ID3D12PipelineState* pPipelineState = nullptr;
    auto hr = pDevice->CreatePipelineState(&pssDesc, IID_PPV_ARGS(&pPipelineState));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
        m_Dirty = false;
        return;
    }

    // 遅延解放.
    auto pso = m_ReloadState.Detach();
    Dispose(pso);

    // 差し替え.
    m_ReloadState.Attach(pPipelineState);

    // ダーティフラグを下す.
    m_Dirty = false;
}


///////////////////////////////////////////////////////////////////////////////
// RayTracingPipelineState class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      リセット処理です.
//-----------------------------------------------------------------------------
void RayTracingPipelineState::State::Reset()
{
    // ステートオブジェクトを遅延解放.
    {
        auto item = Object.Detach();
        Dispose(item);
    }

    // プロパティを遅延解放.
    {
        auto item = Props.Detach();
        Dispose(item);
    }

    // レイ生成テーブルを遅延解放.
    {
        auto item = RayGenTable.Detach();
        Dispose(item);
    }

    // ミステーブルを遅延解放.
    {
        auto item = MissTable.Detach();
        Dispose(item);
    }

    // ヒットグループを遅延解放.
    {
        auto item = HitGroupTable.Detach();
        Dispose(item);
    }
}

//-----------------------------------------------------------------------------
//      有効かどうかチェックします.
//-----------------------------------------------------------------------------
bool RayTracingPipelineState::State::IsValid() const
{ return (Object.GetPtr() != nullptr) && (RayGenTable.GetPtr() != nullptr); }

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
RayTracingPipelineState::RayTracingPipelineState()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
RayTracingPipelineState::~RayTracingPipelineState()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool RayTracingPipelineState::Init(const RAYTRACING_PIPELINE_STATE_DESC& desc)
{
    if (!IsSupportDXR())
    {
        ELOG("Error : Not Support DXR");
        return false;
    }

    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    auto objCount = 5u * desc.HitGroups.size();
    std::vector<D3D12_STATE_SUBOBJECT> objDescs;
    objDescs.resize(objCount);

    std::vector<D3D12_EXPORT_DESC> exports;
    {
        exports.push_back({desc.RayGeneration.c_str(), nullptr, D3D12_EXPORT_FLAG_NONE});
        for(auto i=0u; i<desc.HitGroups.size(); ++i)
        {
            auto& hit = desc.HitGroups[i];

            if (hit.AnyHitShaderImport)
            { exports.push_back({hit.AnyHitShaderImport, nullptr, D3D12_EXPORT_FLAG_NONE}); }

            if (hit.ClosestHitShaderImport)
            { exports.push_back({hit.ClosestHitShaderImport, nullptr, D3D12_EXPORT_FLAG_NONE}); }

            if (hit.IntersectionShaderImport)
            { exports.push_back({hit.IntersectionShaderImport, nullptr, D3D12_EXPORT_FLAG_NONE}); }
        }

        for(auto i=0u; i<desc.MissTables.size(); ++i)
        {
            exports.push_back({desc.MissTables[i].c_str(), nullptr, D3D12_EXPORT_FLAG_NONE});
        }
    }

    auto index = 0;

    D3D12_GLOBAL_ROOT_SIGNATURE globalRootSignature = {};
    globalRootSignature.pGlobalRootSignature = desc.pRootSignature;

    objDescs[index].Type  = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    objDescs[index].pDesc = &globalRootSignature;
    index++;

    D3D12_DXIL_LIBRARY_DESC libDesc = {};
    libDesc.DXILLibrary = desc.Shader;
    libDesc.NumExports  = UINT(exports.size());
    libDesc.pExports    = exports.data();

    objDescs[index].Type     = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    objDescs[index].pDesc    = &libDesc;
    index++;

    D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
    shaderConfig.MaxAttributeSizeInBytes = desc.MaxAttributeSize;
    shaderConfig.MaxPayloadSizeInBytes   = desc.MaxPayloadSize;

    objDescs[index].Type     = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
    objDescs[index].pDesc    = &shaderConfig;
    index++;

    D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
    pipelineConfig.MaxTraceRecursionDepth = desc.MaxTraceDepth;

    objDescs[index].Type     = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
    objDescs[index].pDesc    = &pipelineConfig;
    index++;

    for(auto i=0u; i<desc.HitGroups.size(); ++i)
    {
        objDescs[index].Type  = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
        objDescs[index].pDesc = &desc.HitGroups[i];
        index++;
    }

    D3D12_STATE_OBJECT_DESC stateObjectDesc = {};
    stateObjectDesc.Type            = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    stateObjectDesc.NumSubobjects   = index;
    stateObjectDesc.pSubobjects     = objDescs.data();

    auto hr = pDevice->CreateStateObject(&stateObjectDesc, IID_PPV_ARGS(m_State.Object.GetAddress()));

    // メモリ解放.
    objDescs.clear();

    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device5::CreateStateObject() Failed. errcode = 0x%x", hr);
        return false;
    }

    hr = m_State.Object->QueryInterface(IID_PPV_ARGS(m_State.Props.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12StateObject::QueryInterface() Failed. errcode = 0x%x", hr);
        return false;
    }

    // レイ生成テーブル.
    {
        std::vector<void*> shaderIdentifers;
        shaderIdentifers.resize(1);
        shaderIdentifers[0] = m_State.Props->GetShaderIdentifier(desc.RayGeneration.c_str());

        if (!CreateShaderTable(
            pDevice,
            shaderIdentifers,
            m_State.RayGenTable.GetAddress()))
        {
            ELOG("Error : RayGeneration Table Init Failed.");
            return false;
        }
    }

    // ミステーブル.
    {
        std::vector<void*> shaderIdentifers;
        shaderIdentifers.resize(desc.MissTables.size());
        for(size_t i=0; i<shaderIdentifers.size(); ++i)
        {
            shaderIdentifers[i] = m_State.Props->GetShaderIdentifier(desc.MissTables[i].c_str());
        }

        if (!CreateShaderTable(
            pDevice,
            shaderIdentifers,
            m_State.MissTable.GetAddress()))
        {
            ELOG("Error : Miss Shader Table Init Failed.");
            return false;
        }
    }

    // ヒットグループ.
    {
        std::vector<void*> shaderIdentifers;
        shaderIdentifers.resize(desc.HitGroups.size());
        for(size_t i=0; i<shaderIdentifers.size(); ++i)
        {
            shaderIdentifers[i] = m_State.Props->GetShaderIdentifier(desc.HitGroups[i].HitGroupExport);
        }

        if (!CreateShaderTable(
            pDevice,
            shaderIdentifers,
            m_State.HitGroupTable.GetAddress()))
        {
            ELOG("Error : HitGroup Table Init Failed.");
            return false;
        }
    }

    m_Desc = desc;
    SetBlob(m_Lib, m_Desc.Shader);
    g_Listener.AddRayTracing(this);
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void RayTracingPipelineState::Term()
{
    g_Listener.RemoveRayTracing(this);
    m_State.Reset();
    m_ReloadState.Reset();
}

//-----------------------------------------------------------------------------
//      レイトレーシングパイプラインを起動します.
//-----------------------------------------------------------------------------
void RayTracingPipelineState::DispatchRays
(
    ID3D12GraphicsCommandList4* pCmd,
    uint32_t                    width,
    uint32_t                    height
)
{
    assert(pCmd != nullptr);
    assert(width  > 0);
    assert(height > 0);

    if (m_Dirty)
        Recreate();

    auto recordSize = asdx::RoundUp(
        uint32_t(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES),
        uint32_t(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT));

    auto state = (m_ReloadState.IsValid()) ? &m_ReloadState : &m_State;

    D3D12_DISPATCH_RAYS_DESC desc = {};
    desc.RayGenerationShaderRecord.StartAddress = state->RayGenTable->GetGPUVirtualAddress();
    desc.RayGenerationShaderRecord.SizeInBytes  = state->RayGenTable->GetDesc().Width;

    desc.MissShaderTable.StartAddress   = state->MissTable->GetGPUVirtualAddress();
    desc.MissShaderTable.SizeInBytes    = state->MissTable->GetDesc().Width;
    desc.MissShaderTable.StrideInBytes  = recordSize;

    desc.HitGroupTable.StartAddress     = state->HitGroupTable->GetGPUVirtualAddress();
    desc.HitGroupTable.SizeInBytes      = state->HitGroupTable->GetDesc().Width;
    desc.HitGroupTable.StrideInBytes    = recordSize;

    desc.Width  = width;
    desc.Height = height;
    desc.Depth  = 1;

    pCmd->SetPipelineState1(state->Object.GetPtr());
    pCmd->DispatchRays(&desc);
}

//-----------------------------------------------------------------------------
//      DXILライブラリのリロードファイルパスを設定します.
//-----------------------------------------------------------------------------
void RayTracingPipelineState::SetReloadPath(const std::string& path)
{ m_Lib.Path = ToFullPath(path).string(); }

//-----------------------------------------------------------------------------
//      リロード時の処理です.
//-----------------------------------------------------------------------------
void RayTracingPipelineState::OnReload(const std::string& fullPath)
{
    if (fullPath.empty())
        return;

    if (m_Lib.Path == fullPath)
    {
        std::vector<uint8_t> binary;
        if (CompileFromFileA(fullPath.c_str(), g_Includes, "", "lib_6_6", binary))
        {
            m_Lib.Blob = std::move(binary);
            ReplaceCode(m_Lib, m_Desc.Shader);
            m_Dirty = true;
        }
    }
}

//-----------------------------------------------------------------------------
//      強制リロードを行います.
//-----------------------------------------------------------------------------
void RayTracingPipelineState::ForceReload()
{
    if (m_Lib.Path.empty())
        return;

    std::vector<uint8_t> binary;
    if (CompileFromFileA(m_Lib.Path.c_str(), g_Includes, "", "lib_6_6", binary))
    {
        m_Lib.Blob = std::move(binary);
        ReplaceCode(m_Lib, m_Desc.Shader);
        m_Dirty = true;
    }
}

//-----------------------------------------------------------------------------
//      再生成処理です.
//-----------------------------------------------------------------------------
void RayTracingPipelineState::Recreate()
{
    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    auto objCount = 5u * m_Desc.HitGroups.size();
    std::vector<D3D12_STATE_SUBOBJECT> objDescs;
    objDescs.resize(objCount);

    std::vector<D3D12_EXPORT_DESC> exports;
    {
        exports.push_back({m_Desc.RayGeneration.c_str(), nullptr, D3D12_EXPORT_FLAG_NONE});
        for(auto i=0u; i<m_Desc.HitGroups.size(); ++i)
        {
            auto& hit = m_Desc.HitGroups[i];

            if (hit.AnyHitShaderImport)
            { exports.push_back({hit.AnyHitShaderImport, nullptr, D3D12_EXPORT_FLAG_NONE}); }

            if (hit.ClosestHitShaderImport)
            { exports.push_back({hit.ClosestHitShaderImport, nullptr, D3D12_EXPORT_FLAG_NONE}); }

            if (hit.IntersectionShaderImport)
            { exports.push_back({hit.IntersectionShaderImport, nullptr, D3D12_EXPORT_FLAG_NONE}); }
        }

        for(auto i=0u; i<m_Desc.MissTables.size(); ++i)
        {
            exports.push_back({m_Desc.MissTables[i].c_str(), nullptr, D3D12_EXPORT_FLAG_NONE});
        }
    }

    auto index = 0;

    D3D12_GLOBAL_ROOT_SIGNATURE globalRootSignature = {};
    globalRootSignature.pGlobalRootSignature = m_Desc.pRootSignature;

    objDescs[index].Type  = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    objDescs[index].pDesc = &globalRootSignature;
    index++;

    D3D12_DXIL_LIBRARY_DESC libDesc = {};
    libDesc.DXILLibrary = m_Desc.Shader;
    libDesc.NumExports  = UINT(exports.size());
    libDesc.pExports    = exports.data();

    objDescs[index].Type     = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    objDescs[index].pDesc    = &libDesc;
    index++;

    D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
    shaderConfig.MaxAttributeSizeInBytes = m_Desc.MaxAttributeSize;
    shaderConfig.MaxPayloadSizeInBytes   = m_Desc.MaxPayloadSize;

    objDescs[index].Type     = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
    objDescs[index].pDesc    = &shaderConfig;
    index++;

    D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
    pipelineConfig.MaxTraceRecursionDepth = m_Desc.MaxTraceDepth;

    objDescs[index].Type     = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
    objDescs[index].pDesc    = &pipelineConfig;
    index++;

    for(auto i=0u; i<m_Desc.HitGroups.size(); ++i)
    {
        objDescs[index].Type  = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
        objDescs[index].pDesc = &m_Desc.HitGroups[i];
        index++;
    }

    State state;

    D3D12_STATE_OBJECT_DESC stateObjectDesc = {};
    stateObjectDesc.Type            = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    stateObjectDesc.NumSubobjects   = index;
    stateObjectDesc.pSubobjects     = objDescs.data();

    auto hr = pDevice->CreateStateObject(&stateObjectDesc, IID_PPV_ARGS(state.Object.GetAddress()));

    // メモリ解放.
    objDescs.clear();

    m_Dirty = false;

    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device5::CreateStateObject() Failed. errcode = 0x%x", hr);
    }

    hr = m_ReloadState.Object->QueryInterface(IID_PPV_ARGS(state.Props.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12StateObject::QueryInterface() Failed. errcode = 0x%x", hr);
    }

    // レイ生成テーブル.
    {
        std::vector<void*> shaderIdentifers;
        shaderIdentifers.resize(1);
        shaderIdentifers[0] = state.Props->GetShaderIdentifier(m_Desc.RayGeneration.c_str());

        if (!CreateShaderTable(
            pDevice,
            shaderIdentifers,
            state.RayGenTable.GetAddress()))
        {
            ELOG("Error : RayGeneration Table Init Failed.");
        }
    }

    // ミステーブル.
    {
        std::vector<void*> shaderIdentifers;
        shaderIdentifers.resize(m_Desc.MissTables.size());
        for(size_t i=0; i<shaderIdentifers.size(); ++i)
        {
            shaderIdentifers[i] = state.Props->GetShaderIdentifier(m_Desc.MissTables[i].c_str());
        }

        if (!CreateShaderTable(
            pDevice,
            shaderIdentifers,
            state.MissTable.GetAddress()))
        {
            ELOG("Error : Miss Shader Table Init Failed.");
        }
    }

    // ヒットグループ.
    {
        std::vector<void*> shaderIdentifers;
        shaderIdentifers.resize(m_Desc.HitGroups.size());
        for(size_t i=0; i<shaderIdentifers.size(); ++i)
        {
            shaderIdentifers[i] = state.Props->GetShaderIdentifier(m_Desc.HitGroups[i].HitGroupExport);
        }

        if (!CreateShaderTable(
            pDevice,
            shaderIdentifers,
            state.HitGroupTable.GetAddress()))
        {
            ELOG("Error : HitGroup Table Init Failed.");
        }
    }

    m_ReloadState.Reset();
    m_ReloadState = state;
}

///////////////////////////////////////////////////////////////////////////////
// Functions.
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      パイプラインステートウォッチャーの初期化処理.
//-----------------------------------------------------------------------------
bool InitPipelineStateWatcher(const std::vector<std::string>& dirs, const std::vector<std::string>& includes)
{
    if (g_Initialized)
        return false;

    FileWatcher::Desc desc;
    desc.Dirs = dirs;
    desc.pListeners.push_back(&g_Listener);
    if (!g_Watcher.Init(desc))
    {
        ELOG("Error : FileWatcher::Init() Failed.");
        return false;
    }

    g_Includes = includes;
    g_Initialized = true;
    return true;
}

//-----------------------------------------------------------------------------
//      パイプラインステートウォッチャーの終了処理.
//-----------------------------------------------------------------------------
void TermPipelineStateWatcher()
{
    if (!g_Initialized)
        return;

    g_Watcher .Term();
    g_Listener.Reset();
    g_Includes.clear();
    g_Includes.shrink_to_fit();
    g_Initialized = false;
}

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
