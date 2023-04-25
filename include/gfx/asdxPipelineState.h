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
#include <map>
#include <d3d12.h>
#include <fnd/asdxRef.h>
#include <gfx/asdxView.h>


#ifdef __ID3D12GraphicsCommandList6_INTERFACE_DEFINED__
#define ASDX_ENABLE_MESH_SHADER
#endif//__ID3D12GraphicsCommandList6_INTERFACE_DEFINED__

#if defined(DEBUG) || defined(_DEBUG)
#define D3DCOMPILE_DEBUG 1      // デバッグ情報がシェーダー BLOB に出力されるようする.
#endif//defined(DEBUG) || defined(_DEBUG)


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// SHADER_TYPE
///////////////////////////////////////////////////////////////////////////////
enum SHADER_TYPE
{
    SHADER_TYPE_VS,      // Vertex Shader.
    SHADER_TYPE_PS,      // Pixel Shader.
    SHADER_TYPE_AS,      // Amplification Shader.
    SHADER_TYPE_MS,      // Mesh Shader.
    SHADER_TYPE_CS       // Compute Shader.
};

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
// DEPTH_STENCIL_DESC structure
///////////////////////////////////////////////////////////////////////////////
struct DEPTH_STENCIL_DESC : public D3D12_DEPTH_STENCIL_DESC
{
    DEPTH_STENCIL_DESC(
        DEPTH_STATE_TYPE      type,
        D3D12_COMPARISON_FUNC depthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL);
};

///////////////////////////////////////////////////////////////////////////////
// RASTERIZER_DESC structure
///////////////////////////////////////////////////////////////////////////////
struct RASTERIZER_DESC : public D3D12_RASTERIZER_DESC
{
    RASTERIZER_DESC(RASTERIZER_STATE_TYPE type);
};

///////////////////////////////////////////////////////////////////////////////
// BLEN_DESC structure
///////////////////////////////////////////////////////////////////////////////
struct BLEND_DESC : public D3D12_BLEND_DESC
{
    BLEND_DESC(BLEND_STATE_TYPE type);
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
    bool Init(ID3D12Device8* pDevice, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc);

    //-------------------------------------------------------------------------
    //! @brief      コンピュートパイプラインとして初期化します.
    //!
    //! @param[in]      pDevice     デバイスです.
    //! @param[in]      pDesc       構成設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(ID3D12Device8* pDevice, const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc);

    //-------------------------------------------------------------------------
    //! @brief      ジオメトリパイプラインとして初期化します.
    //!
    //! @param[in]      pDevice     デバイスです.
    //! @param[in]      pDesc       構成設定です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(ID3D12Device8* pDevice, const GEOMETRY_PIPELINE_STATE_DESC* pDesc);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      シェーダを差し替えます.
    //! 
    //! @param[in]      type        シェーダタイプ.
    //! @param[in]      pBinary     シェーダバイナリ.
    //! @param[in]      binarySize  バイナリサイズ.
    //-------------------------------------------------------------------------
    void ReplaceShader(SHADER_TYPE type, const void* pBinary, size_t binarySize);

    //-------------------------------------------------------------------------
    //! @brief      パイプラインステートを再生成します.
    //-------------------------------------------------------------------------
    void Rebuild();

    //-------------------------------------------------------------------------
    //! @brief      パイプラインタイプを取得します.
    //! 
    //! @return     パイプラインタイプを返却します.
    //-------------------------------------------------------------------------
    PIPELINE_TYPE GetType() const;

    //-------------------------------------------------------------------------
    //! @brief      パイプラインステートを設定します.
    //! 
    //! @param[in]      pCmdList        グラフィックスコマンドリスト.
    //-------------------------------------------------------------------------
    void SetState(ID3D12GraphicsCommandList* pCmdList);

    //-------------------------------------------------------------------------
    //! @brief      ルート定数を設定します.
    //! 
    //! @param[in]      pCmdList        グラフィックスコマンドリスト.
    //! @param[in]      type            シェーダタイプです.
    //! @param[in]      registerIndex   レジスタ番号.
    //! @param[in]      paramCount      設定するパラメータ数.
    //! @param[in]      params          設定するパラメータ.
    //! @param[in]      offset          設定先先頭からのオフセット.
    //-------------------------------------------------------------------------
    void SetConstants(
        ID3D12GraphicsCommandList*  pCmdList,
        SHADER_TYPE                 type,
        uint32_t                    registerIndex,
        uint32_t                    paramCount,
        const void*                 params,
        uint32_t                    offset);

    //-------------------------------------------------------------------------
    //! @brief      定数バッファを設定します.
    //! 
    //! @param[in]      pCmdList        コマンドリストです.
    //! @param[in]      type            シェーダタイプです.
    //! @param[in]      registerIndex   レジスタ番号.
    //! @param[in]      pView           ビューです.
    //-------------------------------------------------------------------------
    void SetCBV(ID3D12GraphicsCommandList* pCmdList, SHADER_TYPE type, uint32_t registerIndex, IConstantBufferView* pView);

    //-------------------------------------------------------------------------
    //! @brief      シェーダリソースビューを設定します.
    //! 
    //! @param[in]      pCmdList        コマンドリストです.
    //! @param[in]      type            シェーダタイプです.
    //! @param[in]      registerIndex   レジスタ番号.
    //! @param[in]      pView           ビューです.
    //-------------------------------------------------------------------------
    void SetSRV(ID3D12GraphicsCommandList* pCmdList, SHADER_TYPE type, uint32_t registerIndex, IShaderResourceView* pView);

    //-------------------------------------------------------------------------
    //! @brief      アンオーダードアクセスビューを設定します.
    //! 
    //! @param[in]      pCmdList        コマンドリストです.
    //! @param[in]      type            シェーダタイプです.
    //! @param[in]      registerIndex   レジスタ番号.
    //! @param[in]      pView           ビューです.
    //-------------------------------------------------------------------------
    void SetUAV(ID3D12GraphicsCommandList* pCmdList, SHADER_TYPE type, uint32_t registerIndex, IUnorderedAccessView* pView);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    RefPtr<ID3D12RootSignature>     m_pRootSig;
    RefPtr<ID3D12RootSignature>     m_pRecreateRootSig;
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

    std::map<uint16_t, uint16_t>    m_RootParameterIndices;

    //=========================================================================
    // private methods.
    //=========================================================================

    uint32_t FindIndex(SHADER_TYPE type, uint8_t kind, uint32_t registerIndex) const;

    bool EnumerateRootParameter(
        SHADER_TYPE type,
        const void* binary,
        size_t binarySize,
        std::vector<D3D12_DESCRIPTOR_RANGE*>& ranges,
        std::vector<D3D12_ROOT_PARAMETER>& params,
        std::vector<D3D12_STATIC_SAMPLER_DESC>& samplers);

    bool CreateGraphicsRootSignature(ID3D12Device8* pDevice, ID3D12RootSignature** ppRootSig);
    bool CreateGeometryRootSignature(ID3D12Device8* pDevice, ID3D12RootSignature** ppRootSig);
    bool CreateComputeRootSignature(ID3D12Device8* pDevice, ID3D12RootSignature** ppRootSig);
};

void InitRangeAsSRV(D3D12_DESCRIPTOR_RANGE& range, UINT registerIndex, UINT count = 1);
void InitRangeAsUAV(D3D12_DESCRIPTOR_RANGE& range, UINT registerIndex, UINT count = 1);
void InitAsConstants(D3D12_ROOT_PARAMETER& param, UINT registerIndex, UINT count, D3D12_SHADER_VISIBILITY visibility);
void InitAsCBV(D3D12_ROOT_PARAMETER& param, UINT registerIndex, D3D12_SHADER_VISIBILITY visibility);
void InitAsSRV(D3D12_ROOT_PARAMETER& param, UINT registerIndex, D3D12_SHADER_VISIBILITY visibility);
void InitAsTable(D3D12_ROOT_PARAMETER& param, UINT count, const D3D12_DESCRIPTOR_RANGE* range, D3D12_SHADER_VISIBILITY visiblity);
bool InitRootSignature(ID3D12Device* pDevice, const D3D12_ROOT_SIGNATURE_DESC* pDesc, ID3D12RootSignature** ppRootSig);

} // namespace asdx
