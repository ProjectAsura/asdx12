//-----------------------------------------------------------------------------
// File : asdxPipelineState.h
// Desc : Pipeline State.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

// リロード有効定義.
#ifndef ASDX_ENABLE_PIPELINE_STATE_RELOAD
#define ASDX_ENABLE_PIPELINE_STATE_RELOAD       (1)
#endif//ASDX_ENABLE_PIPELINE_STATE_RELOAD

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <unordered_map>
#include <d3d12.h>
#if ASDX_ENABLE_PIPELINE_STATE_RELOAD
#include <edit/asdxFileWatcher.h>
#endif//ASDX_ENABLE_PIPELINE_STATE_RELOAD


#if defined(DEBUG) || defined(_DEBUG)
#define D3DCOMPILE_DEBUG 1      // デバッグ情報がシェーダー BLOB に出力されるようする.
#endif//defined(DEBUG) || defined(_DEBUG)


namespace asdx {

//-----------------------------------------------------------------------------
// Type Aliasing.
//-----------------------------------------------------------------------------
using PipelineStateHandle = uint64_t;
using ShaderHandle        = uint64_t;


///////////////////////////////////////////////////////////////////////////////
// SHADER_TYPE
///////////////////////////////////////////////////////////////////////////////
enum SHADER_TYPE
{
    SHADER_TYPE_VS,      // Vertex Shader.
    SHADER_TYPE_PS,      // Pixel Shader.
    SHADER_TYPE_DS,      // Domain Shader.
    SHADER_TYPE_HS,      // Hull Shader.
    SHADER_TYPE_GS,      // Geometry Shader.
    SHADER_TYPE_CS,      // Compute Shader.
    SHADER_TYPE_AS,      // Amplification Shader.
    SHADER_TYPE_MS,      // Mesh Shader.
};

///////////////////////////////////////////////////////////////////////////////
// PIPELINE_TYPE
///////////////////////////////////////////////////////////////////////////////
enum PIPELINE_TYPE
{
    PIPELINE_TYPE_GRAPHICS,     //!< Legacy Graphics Pipeline.
    PIPELINE_TYPE_COMPUTE,      //!< Compute Pipeline.
    PIPELINE_TYPE_MESH_SHADER,  //!< Mesh Shader Pipeline.
};

///////////////////////////////////////////////////////////////////////////////
// GEOMETRY_PIPELINE_STATE_DESC structure
///////////////////////////////////////////////////////////////////////////////
struct MESH_SHADER_PIPELINE_STATE_DESC
{
    ID3D12RootSignature*        pRootSignature;         //!< ルートシグニチャ.
    D3D12_SHADER_BYTECODE       AS;                     //!< 増幅シェーダ.
    D3D12_SHADER_BYTECODE       MS;                     //!< メッシュシェーダ.
    D3D12_SHADER_BYTECODE       PS;                     //!< ピクセルシェーダ.
    D3D12_BLEND_DESC            BlendState;             //!< ブレンドステート.
    UINT                        SampleMask;             //!< サンプルマスク.
    D3D12_RASTERIZER_DESC       RasterizerState;        //!< ラスタライザーステート.
    D3D12_DEPTH_STENCIL_DESC    DepthStencilState;      //!< 深度ステンシルステート.
    D3D12_RT_FORMAT_ARRAY       RTVFormats;             //!< レンダーターゲットフォーマット.
    DXGI_FORMAT                 DSVFormat;              //!< 深度ステンシルビューフォーマット.
    DXGI_SAMPLE_DESC            SampleDesc;             //!< サンプル設定.
    UINT                        NodeMask;               //!< ノードマスク.
    D3D12_CACHED_PIPELINE_STATE CachedPSO;              //!< キャッシュ済みPSO.
    D3D12_PIPELINE_STATE_FLAGS  Flags;                  //!< フラグ.
};

///////////////////////////////////////////////////////////////////////////////
// PipelineStateManager 
///////////////////////////////////////////////////////////////////////////////
class PipelineStateManager 
#if ASDX_ENABLE_PIPELINE_STATE_RELOAD
: public IFileListener
#endif
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      シングルトンインスタンスを取得します.
    //! 
    //! @return     シングルトンインスタンスを返却します.
    //-------------------------------------------------------------------------
    static PipelineStateManager& Instance();

    //-------------------------------------------------------------------------
    //! @brief      リセット処理を行います.
    //-------------------------------------------------------------------------
    void Reset()
    {
    #if ASDX_ENABLE_PIPELINE_STATE_RELOAD
        ClearEx();
    #else
        Clear();
    #endif
    }

    //-------------------------------------------------------------------------
    //! @brief      パイプラインステートを生成します.
    //! 
    //! @param[in]      pDesc       構成設定.
    //! @param[out]     handle      パイプラインステートハンドルの格納先.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //-------------------------------------------------------------------------
    bool Create(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, PipelineStateHandle& handle);

    //-------------------------------------------------------------------------
    //! @brief      パイプラインステートを生成します.
    //! 
    //! @param[in]      pDesc       構成設定.
    //! @param[out]     handle      パイプラインステートハンドルの格納先.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //-------------------------------------------------------------------------
    bool Create(const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc, PipelineStateHandle& handle);

    //-------------------------------------------------------------------------
    //! @brief      パイプラインステートを生成します.
    //! 
    //! @param[in]      pDesc       構成設定.
    //! @param[out]     handle      パイプラインステートハンドルの格納先.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //-------------------------------------------------------------------------
    bool Create(const MESH_SHADER_PIPELINE_STATE_DESC* pDesc, PipelineStateHandle& handle);

    //-------------------------------------------------------------------------
    //! @brief      パイプラインステートを設定します.
    //! 
    //! @param[in]      pCmd        グラフィックスコマンドリストです.
    //! @param[in]      handle      パイプラインステートハンドルです.
    //-------------------------------------------------------------------------
    void SetState(ID3D12GraphicsCommandList* pCmd, const PipelineStateHandle& handle)
    {
    #if ASDX_ENABLE_PIPELINE_STATE_RELOAD
        auto pipelineState = FindPipelineStateEx(handle);
    #else
        auto pipelineState = FindPipelineState(handle);
    #endif
        if (pipelineState == nullptr)
            return;

        pCmd->SetPipelineState(pipelineState);
    }

#if ASDX_ENABLE_PIPELINE_STATE_RELOAD
    //-------------------------------------------------------------------------
    //! @brief      リロードするシェーダを登録します.
    //! 
    //! @param[in]      type            シェーダタイプです.
    //! @param[in]      path            ファイルパスです.
    //! @param[in]      pipelineCount   パイプライン数です.
    //! @param[in]      handles         パイプラインハンドルの配列です.
    //! @return     シェーダハンドルを返却します.
    //-------------------------------------------------------------------------
    ShaderHandle RegisterShader(SHADER_TYPE type, const char* path, uint32_t pipelineCounts, const PipelineStateHandle* handles);

    //-------------------------------------------------------------------------
    //! @brief      リロードするシェーダを登録します.
    //! 
    //! @param[in]      type            シェーダタイプです.
    //! @param[in]      path            ファイルパスです.
    //! @param[in]      handle          パイプラインハンドル
    //! @return     シェーダハンドルを返却します.
    //-------------------------------------------------------------------------
    ShaderHandle RegisterShader(SHADER_TYPE type, const char* path, PipelineStateHandle handle)
    { return RegisterShader(type, path, 1, &handle); }

    //-------------------------------------------------------------------------
    //! @brief      リロードするシェーダを登録解除します.
    //! 
    //! @param[in]      handle      登録解除するシェーダハンドルです.
    //-------------------------------------------------------------------------
    void UnregisterShader(ShaderHandle handle);

    //-------------------------------------------------------------------------
    //! @brief      リロードするシェーダインクルードファイルを登録します.
    //! 
    //! @param[in]      path                ファイルパス.
    //! @param[in]      dependencyCount     依存シェーダファイル数.
    //! @param[in]      handles             依存シェーダファイルの配列.
    //! @return     依存ファイルハンドルを返却します.
    //! @note       主にインクルードファイルの変更によるシェーダの変更を通知するために使用します.
    //-------------------------------------------------------------------------
    ShaderHandle RegisterInclude(const char* path, uint32_t shaderCount, const ShaderHandle* handles);

    //-------------------------------------------------------------------------
    //! @brief      リロードするシェーダインクルードファイルを登録します.
    //! 
    //! @param[in]      path                ファイルパス.
    //! @param[in]      handle              依存シェーダファイル.
    //! @return     依存ファイルハンドルを返却します.
    //! @note       主にインクルードファイルの変更によるシェーダの変更を通知するために使用します.
    //-------------------------------------------------------------------------
    ShaderHandle RegisterInclude(const char* path, ShaderHandle handle)
    { return RegisterInclude(path, 1, &handle); }

    //-------------------------------------------------------------------------
    //! @brief      リロードするシェーダインクルードファイルの登録解除します.
    //! 
    //! @param[in]      handle      登録解除する依存ファイルハンドルです.
    //-------------------------------------------------------------------------
    void UnregisterInclude(ShaderHandle handle);

    //-------------------------------------------------------------------------
    //! @brief      インクルードディレクトリを追加します.
    //! 
    //! @param[in]      dirPath     追加するインクルードディレクトリです.
    //-------------------------------------------------------------------------
    void AddIncludeDirs(const char* dirPath);

    //-------------------------------------------------------------------------
    //! @brief      ファイル変更時の処理です.
    //! 
    //! @param[in]      args        ファイルイベント引数.
    //-------------------------------------------------------------------------
    void OnChanged(const FileEventArgs& args) override;

    //-------------------------------------------------------------------------
    //! @brief      更新処理を行います.
    //-------------------------------------------------------------------------
    void Update();
#endif

private:
    ///////////////////////////////////////////////////////////////////////////
    // PipelineStateDesc structure
    ///////////////////////////////////////////////////////////////////////////
    struct PipelineStateDesc
    {
        PIPELINE_TYPE   Type;
        union {
            D3D12_GRAPHICS_PIPELINE_STATE_DESC Graphics;
            D3D12_COMPUTE_PIPELINE_STATE_DESC  Compute;
            MESH_SHADER_PIPELINE_STATE_DESC    MeshShader;
        };
        std::vector<uint8_t>    VS;
        std::vector<uint8_t>    PS;
        std::vector<uint8_t>    HS;
        std::vector<uint8_t>    DS;
        std::vector<uint8_t>    GS;
        std::vector<uint8_t>    CS;
        std::vector<uint8_t>    AS;
        std::vector<uint8_t>    MS;
    };

#if ASDX_ENABLE_PIPELINE_STATE_RELOAD
    ///////////////////////////////////////////////////////////////////////////
    // Reload structure
    ///////////////////////////////////////////////////////////////////////////
    struct Reload
    {
        SHADER_TYPE                         Type;
        std::string                         Path;
        std::vector<PipelineStateHandle>    Handles;

        Reload() = default;

        Reload(SHADER_TYPE type, const std::string& path, uint32_t pipelineCount, const PipelineStateHandle* handles)
        : Type  (type)
        , Path  (path)
        {
            if (handles != nullptr)
            {
                Handles.resize(pipelineCount);
                for(auto i=0u; i<pipelineCount; ++i)
                { Handles[i] = handles[i]; }
            }
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // Dependency structure
    ///////////////////////////////////////////////////////////////////////////
    struct Dependency
    {
        std::string                 Path;
        std::vector<ShaderHandle>   Shaders;

        Dependency() = default;

        Dependency(const std::string& path, uint32_t shaderCount, const ShaderHandle* handles)
            : Path(path)
        {
            if (handles != nullptr)
            {
                Shaders.resize(shaderCount);
                for(auto i=0u; i<shaderCount; ++i)
                { Shaders[i] = handles[i]; }
            }
        }
    };
#endif

    //=========================================================================
    // private variables.
    //=========================================================================
    static PipelineStateManager s_Instance; //!< シングルトンインスタンスです.
    std::unordered_map<PipelineStateHandle, PipelineStateDesc>      m_Descs;                //!< パイプラインステート設定です.
    std::unordered_map<PipelineStateHandle, ID3D12PipelineState*>   m_PipelineStates;       //!< パイプラインステートです.
#if ASDX_ENABLE_PIPELINE_STATE_RELOAD
    std::vector<std::string>                                        m_IncludeDirs;          //!< インクルードディレクトリ.
    std::unordered_map<ShaderHandle, Reload>                        m_Reloads;              //!< リロード情報.
    std::unordered_map<ShaderHandle, Dependency>                    m_Dependencies;         //!< 依存情報.
    std::unordered_map<PipelineStateHandle, ID3D12PipelineState*>   m_ReloadPipelineStates; //!< リロードで生成したパイプラインステートです.
    std::unordered_map<PipelineStateHandle, PipelineStateDesc>      m_RequestDescs;         //!< 再生成リクエスト設定です.
#endif

    //=========================================================================
    // private methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      パイプラインステートを検索します.
    //! 
    //! @param[in]      handle      パイプラインステートハンドル.
    //! @return     パイプラインステートを返却します.
    //!             見つからなかった場合は nullptr を返却します.
    //-------------------------------------------------------------------------
    ID3D12PipelineState* FindPipelineState(const PipelineStateHandle& handle);

    //-------------------------------------------------------------------------
    //! @brief      クリア処理を行います.
    //-------------------------------------------------------------------------
    void Clear();

#if ASDX_ENABLE_PIPELINE_STATE_RELOAD
    //-------------------------------------------------------------------------
    //! @brief      パイプラインステートを検索します.
    //! 
    //! @param[in]      handle      パイプラインステートハンドル.
    //! @return     パイプラインステートを返却します.
    //!             見つからなかった場合は nullptr を返却します.
    //-------------------------------------------------------------------------
    ID3D12PipelineState* FindPipelineStateEx(const PipelineStateHandle& handle);

    //-------------------------------------------------------------------------
    //! @brief      クリア処理を行います.
    //-------------------------------------------------------------------------
    void ClearEx();

    //-------------------------------------------------------------------------
    //! @brief      再生成要求を発行します.
    //! 
    //! @param[in]      pipelineHandle      パイプラインステートハンドル.
    //! @param[in]      shaderHandle        シェーダハンドル.
    //-------------------------------------------------------------------------
    void RequestRecreate(PipelineStateHandle pipelineHandle, ShaderHandle shaderHandle);

    //-------------------------------------------------------------------------
    //! @brief      パイプラインステートを生成します.
    //! 
    //! @param[in]      pDesc       構成設定.
    //! @param[in]      handle      パイプラインステートハンドル.
    //-------------------------------------------------------------------------
    void Recreate(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, const PipelineStateHandle& handle);

    //-------------------------------------------------------------------------
    //! @brief      パイプラインステートを生成します.
    //! 
    //! @param[in]      pDesc       構成設定.
    //! @param[in]      handle      パイプラインステートハンドル.
    //-------------------------------------------------------------------------
    void Recreate(const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc, const PipelineStateHandle& handle);

    //-------------------------------------------------------------------------
    //! @brief      パイプラインステートを生成します.
    //! 
    //! @param[in]      pDesc       構成設定.
    //! @param[int]     handle      パイプラインステートハンドル.
    //-------------------------------------------------------------------------
    void Recreate(const MESH_SHADER_PIPELINE_STATE_DESC* pDesc, const PipelineStateHandle& handle);
#endif

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    PipelineStateManager()
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~PipelineStateManager()
    { Clear(); }

    PipelineStateManager             (const PipelineStateManager&) = delete;
    PipelineStateManager& operator = (const PipelineStateManager&) = delete;
};


void InitRangeAsSRV(D3D12_DESCRIPTOR_RANGE& range, UINT registerIndex, UINT count = 1, UINT registerSpace = 0);
void InitRangeAsUAV(D3D12_DESCRIPTOR_RANGE& range, UINT registerIndex, UINT count = 1, UINT registerSpace = 0);
void InitAsConstants(D3D12_ROOT_PARAMETER& param, UINT registerIndex, UINT count, D3D12_SHADER_VISIBILITY visibility, UINT registerSpace = 0);
void InitAsCBV(D3D12_ROOT_PARAMETER& param, UINT registerIndex, D3D12_SHADER_VISIBILITY visibility, UINT registerSpace = 0);
void InitAsSRV(D3D12_ROOT_PARAMETER& param, UINT registerIndex, D3D12_SHADER_VISIBILITY visibility, UINT registerSpace = 0);
void InitAsUAV(D3D12_ROOT_PARAMETER& param, UINT registerIndex, D3D12_SHADER_VISIBILITY visibility, UINT registerSpace = 0);
void InitAsTable(D3D12_ROOT_PARAMETER& param, UINT count, const D3D12_DESCRIPTOR_RANGE* range, D3D12_SHADER_VISIBILITY visiblity);
bool InitRootSignature(ID3D12Device* pDevice, const D3D12_ROOT_SIGNATURE_DESC* pDesc, ID3D12RootSignature** ppRootSig);

} // namespace asdx
