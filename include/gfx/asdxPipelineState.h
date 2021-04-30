//-----------------------------------------------------------------------------
// File : asdxPipelineState.h
// Desc : Pipeline State.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <vector>
#include <d3d12.h>
#include <asdxRef.h>


#ifdef __ID3D12GraphicsCommandList6_INTERFACE_DEFINED__
#define ASDX_ENABLE_MESH_SHADER
#endif//__ID3D12GraphicsCommandList6_INTERFACE_DEFINED__

#if defined(DEBUG) || defined(_DEBUG)
#define D3DCOMPILE_DEBUG 1      // デバッグ情報がシェーダー BLOB に出力されるようする.
#endif//defined(DEBUG) || defined(_DEBUG)


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// PIPELINE_TYPE
///////////////////////////////////////////////////////////////////////////////
enum PIPELINE_TYPE
{
    PIPELINE_TYPE_GRAPHICS,     //!< Legacy Graphics Pipeline.
    PIPELINE_TYPE_COMPUTE,      //!< Compute Pipeline.
    PIPELINE_TYPE_GEOMETRY,     //!< Mesh Shader Pipeline.
};

///////////////////////////////////////////////////////////////////////////////
// DEPTH_STATE_TYPE
///////////////////////////////////////////////////////////////////////////////
enum DEPTH_STATE_TYPE
{
    DEPTH_STATE_DEFAULT,
    DEPTH_STATE_NONE,
    DEPTH_STATE_READ_ONLY,
    DEPTH_STATE_WRITE_ONLY,

    MAX_COUNT_DEPTH_STATE_TYPE
};

///////////////////////////////////////////////////////////////////////////////
// RASTERIZER_STATE_TYPE
///////////////////////////////////////////////////////////////////////////////
enum RASTERIZER_STATE_TYPE
{
    RASTERIZER_STATE_CULL_NONE,
    RASTERIZER_STATE_CULL_BACK,
    RASTERIZER_STATE_CULL_FRONT,
    RASTERIZER_STATE_WIREFRAME,

    MAX_COUNT_RASTERIZER_STATE_TYPE
};

///////////////////////////////////////////////////////////////////////////////
// BLEND_STATE_TYPE
///////////////////////////////////////////////////////////////////////////////
enum BLEND_STATE_TYPE
{
    BLEND_STATE_OPAQUE,
    BLEND_STATE_ALPHABLEND,
    BLEND_STATE_ADDITIVE,
    BLEND_STATE_SUBTRACT,
    BLEND_STATE_PREMULTIPLIED,
    BLEND_STATE_MULTIPLY,
    BLEND_STATE_SCREEN,

    MAX_COUNT_BLEND_STATE_TYPE
};

///////////////////////////////////////////////////////////////////////////////
// GEOMETRY_PIPELINE_STATE_DESC structure
///////////////////////////////////////////////////////////////////////////////
struct GEOMETRY_PIPELINE_STATE_DESC
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
// PipelineState class
///////////////////////////////////////////////////////////////////////////////
class PipelineState
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
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    PipelineState();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~PipelineState();

    //-------------------------------------------------------------------------
    //! @brief      グラフィックスパイプラインとして初期化します.
    //!
    //! @param[in]      pDevice     デバイスです.
    //! @param[in]      pDesc       構成設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(ID3D12Device* pDevice, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc);

    //-------------------------------------------------------------------------
    //! @brief      コンピュートパイプラインとして初期化します.
    //!
    //! @param[in]      pDevice     デバイスです.
    //! @param[in]      pDesc       構成設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(ID3D12Device* pDevice, const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc);

    //-------------------------------------------------------------------------
    //! @brief      ジオメトリパイプラインとして初期化します.
    //!
    //! @param[in]      pDevice     デバイスです.
    //! @param[in]      pDesc       構成設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(ID3D12Device2* pDevice, const GEOMETRY_PIPELINE_STATE_DESC* pDesc);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      頂点シェーダを差し替えます.
    //-------------------------------------------------------------------------
    void ReplaceVS(const void* pBinary, size_t binarySize);

    //-------------------------------------------------------------------------
    //! @brief      ピクセルシェーダを差し替えます.
    //-------------------------------------------------------------------------
    void ReplacePS(const void* pBinary, size_t binarySize);

    //-------------------------------------------------------------------------
    //! @brief      コンピュートシェーダを差し替えます.
    //-------------------------------------------------------------------------
    void ReplaceCS(const void* pBinary, size_t binarySize);

    //-------------------------------------------------------------------------
    //! @brief      メッシュシェーダを差し替えます.
    //-------------------------------------------------------------------------
    void ReplaceMS(const void* pBinary, size_t binarySize);

    //-------------------------------------------------------------------------
    //! @brief      増幅シェーダを差し替えます.
    //-------------------------------------------------------------------------
    void ReplaceAS(const void* pBinary, size_t binarySize);

    //-------------------------------------------------------------------------
    //! @brief      パイプラインステートを再生成します.
    //-------------------------------------------------------------------------
    void Recreate();

    //-------------------------------------------------------------------------
    //! @brief      パイプラインステートを取得します.
    //!
    //! @return     パイプラインステートを返却します.
    //-------------------------------------------------------------------------
    ID3D12PipelineState* GetPtr() const;

    //-------------------------------------------------------------------------
    //! @brief      パイプラインタイプを取得します.
    //!
    //! @return     パイプラインタイプを返却します.
    //-------------------------------------------------------------------------
    PIPELINE_TYPE GetType() const;

    //-------------------------------------------------------------------------
    //! @brief      深度ステンシルステートを取得します.
    //!
    //! @param[in]      type        深度ステンシルステートタイプ.
    //! @param[in]      func        深度比較関数です.
    //! @return     深度ステンシルステートを返却します.
    //-------------------------------------------------------------------------
    static D3D12_DEPTH_STENCIL_DESC GetDepthStencilState(
        DEPTH_STATE_TYPE        type,
        D3D12_COMPARISON_FUNC   func);

    //-------------------------------------------------------------------------
    //! @brief      ラスタライザーステートを取得します.
    //!
    //! @param[in]      type        ラスタライザーステートタイプ.
    //! @return     ラスタライザーステートを返却します.
    //-------------------------------------------------------------------------
    static D3D12_RASTERIZER_DESC GetRasterizerState(RASTERIZER_STATE_TYPE type);

    //-------------------------------------------------------------------------
    //! @brief      ブレンドステートを取得します.
    //!
    //! @param[in]      type        ブレンドステートタイプ.
    //! @return     ブレンドステートを返却します.
    //-------------------------------------------------------------------------
    static D3D12_BLEND_DESC GetBlendState(BLEND_STATE_TYPE type);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    RefPtr<ID3D12PipelineState>     m_pPSO;
    RefPtr<ID3D12PipelineState>     m_pRecreatePSO;
    PIPELINE_TYPE                   m_Type;

    union Desc
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC  Graphics;
        D3D12_COMPUTE_PIPELINE_STATE_DESC   Compute;
        GEOMETRY_PIPELINE_STATE_DESC        Geometry;
    } m_Desc;

    std::vector<uint8_t>    m_VS;
    std::vector<uint8_t>    m_PS;
    std::vector<uint8_t>    m_CS;
    std::vector<uint8_t>    m_MS;
    std::vector<uint8_t>    m_AS;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* DO_NOTHING */
};

inline D3D12_DEPTH_STENCIL_DESC GetDSS
(
    DEPTH_STATE_TYPE      type,
    D3D12_COMPARISON_FUNC func = D3D12_COMPARISON_FUNC_LESS_EQUAL
)
{ return PipelineState::GetDepthStencilState(type, func); }

inline D3D12_RASTERIZER_DESC GetRS(RASTERIZER_STATE_TYPE type)
{ return PipelineState::GetRasterizerState(type); }

inline D3D12_BLEND_DESC GetBS(BLEND_STATE_TYPE type)
{ return PipelineState::GetBlendState(type); }

} // namespace asdx
