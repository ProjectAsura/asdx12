//-----------------------------------------------------------------------------
// File : asdxSprite.h
// Desc : Sprite Renderer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <vector>
#include <d3d12.h>
#include <fnd/asdxRef.h>
#include <fnd/asdxMath.h>
#include <gfx/asdxAllocationHolder.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// SpriteRenderer
///////////////////////////////////////////////////////////////////////////////
class SpriteRenderer
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    static const uint32_t CBV0      = 0;    // Constants 16, VS
    static const uint32_t SRV0      = 1;    // DescriptorTable, PS
    static const uint32_t Sampler0  = 2;    // DescriptorTable, PS.
    static const uint32_t CBV1      = 3;    // Constants 4, PS
    static const uint32_t SRV1      = 4;    // DescriptorTable, PS.

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    SpriteRenderer();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~SpriteRenderer();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      screenWidth     スクリーンの横幅.
    //! @param[in]      screenHeight    スクリーンの縦幅.
    //! @param[in]      maxSpriteCount  最大スプライト数.
    //! @param[in]      maxBatchCount   最大バッチ数(最大テクスチャの切り替え可能数).
    //! @param[in]      rtvFormat       レンダーターゲットビューのフォーマット.
    //! @param[in]      dsvFroamt       深度ステンシルビューのフォーマット.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(
        uint32_t        screenWidth,
        uint32_t        srceenHeight,
        uint32_t        maxSpriteCount,
        uint32_t        maxBatchCount,
        DXGI_FORMAT     rtvFormat,
        DXGI_FORMAT     dsvFormat);

    //-------------------------------------------------------------------------
    //! @brief      終了処理です.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      リセット処理です.
    //! 
    //! @note       フレーム開始時に呼び出してしてください.
    //-------------------------------------------------------------------------
    void Reset();

    //-------------------------------------------------------------------------
    //! @brief      バッチを変更します.
    //! 
    //! @param[in]      pPipelineState      パイプラインステートです.
    //! @param[in]      handleSRV           シェーダリソースビューハンドルです.
    //! @param[in]      handleSampler       サンプラーハンドルです.
    //-------------------------------------------------------------------------
    void ChangeBatch(ID3D12PipelineState* pPipelineState, D3D12_GPU_DESCRIPTOR_HANDLE handleSRV, D3D12_GPU_DESCRIPTOR_HANDLE handleSampler);

    //-------------------------------------------------------------------------
    //! @brief      パイプラインステートを変更します.
    //! 
    //! @param[in]      pPipelineState      パイプラインステートです.
    //! @note       内部で ChangeBatch() をコールします.
    //-------------------------------------------------------------------------
    void SetPipelineState(ID3D12PipelineState* pPipelineState);

    //-------------------------------------------------------------------------
    //! @brief      テクスチャを変更します.
    //! 
    //! @param[in]      handleSRV       シェーダリソースビューハンドル.
    //! @param[in]      handleSampler   サンプラーハンドル.
    //! @note       内部で ChangeBatch() をコールします.
    //-------------------------------------------------------------------------
    void SetTexture(D3D12_GPU_DESCRIPTOR_HANDLE handleSRV, D3D12_GPU_DESCRIPTOR_HANDLE handleSampler);

    //-------------------------------------------------------------------------
    //! @brief      カラーを設定します.
    //! 
    //! @param[in]      r       R成分.
    //! @param[in]      g       G成分.
    //! @param[in]      b       B成分.
    //! @param[in]      a       A成分.,
    //-------------------------------------------------------------------------
    void SetColor(float r, float g, float b, float a);

    //-------------------------------------------------------------------------
    //! @brief      カラーを設定します.
    //! 
    //! @param[in]      r       R成分.
    //! @param[in]      g       G成分.
    //! @param[in]      b       B成分.
    //! @param[in]      a       A成分.,
    //-------------------------------------------------------------------------
    void SetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    //-------------------------------------------------------------------------
    //! @brief      スクリーンサイズを設定します.
    //! 
    //! @note       Draw()時に設定されるため，Add()ごとに設定することはできません.
    //-------------------------------------------------------------------------
    void SetScreenSize(uint32_t w, uint32_t h);

    //-------------------------------------------------------------------------
    //! @brief      スプライトを追加します.
    //!
    //! @param[in]      x           描画開始位置Xです.
    //! @param[in]      y           描画開始位置Yです.
    //! @param[in]      w           スプライトの横幅です.
    //! @param[in]      h           スプライトの縦幅です.
    //! @param[in]      layer       レイヤーです(Z座標).
    //! @param[in]      uv0         左上のUV座標です.
    //! @param[in]      uv1         右下のUV座標です.
    //-------------------------------------------------------------------------
    void Add(int x, int y, int w, int h, int layer, const Vector2& uv0, const Vector2& uv1);

    //-------------------------------------------------------------------------
    //! @brief      スプライトを追加します.
    //!
    //! @param[in]      x           描画開始位置Xです.
    //! @param[in]      y           描画開始位置Yです.
    //! @param[in]      w           スプライトの横幅です.
    //! @param[in]      h           スプライトの縦幅です.
    //-------------------------------------------------------------------------
    void Add(int x, int y, int w, int h)
    { Add(x, y, w, h, 0, Vector2(0.0f, 1.0f), Vector2(1.0f, 0.0f)); }

    //-------------------------------------------------------------------------
    //! @brief      スプライトを追加します.
    //!
    //! @param[in]      x           描画開始位置Xです.
    //! @param[in]      y           描画開始位置Yです.
    //! @param[in]      w           スプライトの横幅です.
    //! @param[in]      h           スプライトの縦幅です.
    //! @param[in]      layer       レイヤーです(Z座標).
    //-------------------------------------------------------------------------
    void Add(int x, int y, int w, int h, int layer)
    { Add(x, y, w, h, layer, Vector2(0.0f, 1.0f), Vector2(1.0f, 0.0f)); }

    //-------------------------------------------------------------------------
    //! @brief      スプライトを追加します.
    //!
    //! @param[in]      x           描画開始位置Xです.
    //! @param[in]      y           描画開始位置Yです.
    //! @param[in]      w           スプライトの横幅です.
    //! @param[in]      h           スプライトの縦幅です.
    //! @param[in]      uv0         左上のUV座標です.
    //! @param[in]      uv1         右下のUV座標です.
    //-------------------------------------------------------------------------
    void Add(int x, int y, int w, int h, const Vector2& uv0, const Vector2& uv1)
    { Add(x, y, w, h, 0, uv0, uv1); }

    //-------------------------------------------------------------------------
    //! @brief      回転したスプライトを追加します.
    //! 
    //! @param[in]      x           描画開始位置Xです.
    //! @param[in]      y           描画開始位置Yです.
    //! @param[in]      w           スプライトの横幅です.
    //! @param[in]      h           スプライトの縦幅です.
    //! @param[in]      layer       レイヤーです(Z座標).
    //! @param[in]      rad         回転量(単位:ラジアン).
    //! @param[in]      uv0         左上のUV座標です.
    //! @param[in]      uv1         右下のUV座標です.
    //-------------------------------------------------------------------------
    void Add(int x, int y, int w, int h, int layer, float rad, const Vector2& uv0, const Vector2& uv);

    //-------------------------------------------------------------------------
    //! @brief      回転したスプライトを追加します.
    //!
    //! @param[in]      x           描画開始位置Xです.
    //! @param[in]      y           描画開始位置Yです.
    //! @param[in]      w           スプライトの横幅です.
    //! @param[in]      h           スプライトの縦幅です.
    //! @param[in]      layer       レイヤーです(Z座標).
    //! @param[in]      rad         回転量(単位:ラジアン).
    //-------------------------------------------------------------------------
    void Add(int x, int y, int w, int h, int layer, float rad)
    { Add(x, y, w, h, layer, rad, Vector2(0.0f, 1.0f), Vector2(1.0f, 0.0f)); }

    //-------------------------------------------------------------------------
    //! @brief      回転したスプライトを追加します.
    //! 
    //! @param[in]      x           描画開始位置Xです.
    //! @param[in]      y           描画開始位置Yです.
    //! @param[in]      w           スプライトの横幅です.
    //! @param[in]      h           スプライトの縦幅です.
    //! @param[in]      rad         回転量(単位:ラジアン).
    //-------------------------------------------------------------------------
    void Add(int x, int y, int w, int h, float rad)
    { Add(x, y, w, h, 0, rad); }

    //-------------------------------------------------------------------------
    //! @brief      描画処理を行います.
    //! 
    //! @param[in]      pCmdList        グラフィックスコマンドリストです.
    //-------------------------------------------------------------------------
    void Draw(ID3D12GraphicsCommandList* pCmdList);

    //-------------------------------------------------------------------------
    //! @brief      スクリーンサイズを取得します.
    //! 
    //! @return     スクリーンサイズを返却します.
    //-------------------------------------------------------------------------
    const Vector2& GetScreenSize() const;

    //-------------------------------------------------------------------------
    //! @brief      カラーを取得します.
    //! 
    //! @return     カラーを返却します.
    //-------------------------------------------------------------------------
    Vector4 GetColor() const;

    //-------------------------------------------------------------------------
    //! @brief      スプライト描画用パイプラインステートを生成します.
    //! 
    //! @param[in]      pDevice                 デバイス.
    //! @param[in]      pixelShader             ピクセルシェーダ
    //! @param[in]      preMultipliedAlpha      事前乗算済みアルファを使用する場合は true, そうでない場合はアルファブレンドになります.
    //! @param[out]     ppResult                パイプラインステートの格納先です.
    //! @retval true    生成に成功.
    //! @retval false   生成に失敗.
    //-------------------------------------------------------------------------
    bool CreatePipelineState(
        ID3D12Device*                   pDevice,
        const D3D12_SHADER_BYTECODE&    pixelShader,
        bool                            preMultipliedAlpha,
        ID3D12PipelineState**           pResult);

    //-------------------------------------------------------------------------
    //! @brief      ユーザーパラメータを設定します.
    //! 
    //! @param[in]      count       32bitパラメータの数.
    //! @param[in]      param       32bitパラメータ.
    //! @param[in]      destOffset  書き込みオフセット.
    //! @note      ChangeBatch() コール時に反映されるため，ChangeBatch() よりも先に呼び出してください.
    //-------------------------------------------------------------------------
    void SetParam(uint32_t count, const void* param, uint32_t destOffset);

    //-------------------------------------------------------------------------
    //! @brief      デフォルトのパイプラインステートを取得します.
    //!
    //! @return     デフォルトのパイプラインステートを返却します.
    //-------------------------------------------------------------------------
    ID3D12PipelineState* GetDefaultState() const;

    //-------------------------------------------------------------------------
    //! @brief      現在のパイプラインステートを取得します.
    //! 
    //! @return     現在のパイプラインステートを返却します.
    //-------------------------------------------------------------------------
    ID3D12PipelineState* GetCurrentState() const;

    //-------------------------------------------------------------------------
    //! @brief      シェーダリソースビューハンドルを取得します.
    //! 
    //! @return     シェーダリソースビューハンドルを返却します.
    //-------------------------------------------------------------------------
    D3D12_GPU_DESCRIPTOR_HANDLE GetHandleSRV() const;

    //-------------------------------------------------------------------------
    //! @brief      サンプラーハンドルを取得します.
    //! 
    //! @return     サンプラーハンドルを返却します.
    //-------------------------------------------------------------------------
    D3D12_GPU_DESCRIPTOR_HANDLE GetHandleSampler() const;

    //-------------------------------------------------------------------------
    //! @brief      ルートシグニチャを取得します.
    //! 
    //! @return     ルートシグニチャを返却します.
    //-------------------------------------------------------------------------
    ID3D12RootSignature* GetRootSignature() const;

private:
    ///////////////////////////////////////////////////////////////////////////
    // Unorm4 structure
    ///////////////////////////////////////////////////////////////////////////
    struct Unorm4
    {
        uint8_t R;      //!< 赤.
        uint8_t G;      //!< 緑.
        uint8_t B;      //!< 青.
        uint8_t A;      //!< 透明度.
    };

    ///////////////////////////////////////////////////////////////////////////
    // Vertex structure
    ///////////////////////////////////////////////////////////////////////////
    struct Vertex
    {
        Vector3 Position;   //!< 位置座標.
        Vector2 TexCoord;   //!< テクスチャ座標.
        Unorm4  Color;      //!< 頂点カラー.
    };

    ///////////////////////////////////////////////////////////////////////////
    // Batch structure
    ///////////////////////////////////////////////////////////////////////////
    struct Batch
    {
        uint32_t                    IndexCount;     //!< インデックス数.
        uint32_t                    IndexOffset;    //!< インデックスオフセット.
        D3D12_GPU_DESCRIPTOR_HANDLE SRV;            //!< シェーダリソースビュー.
        D3D12_GPU_DESCRIPTOR_HANDLE Sampler;        //!< サンプラーステート.
        ID3D12PipelineState*        pState;         //!< パイプラインステート.
        uint32_t                    Param[4];       //!< ユーザーパラメータ.
    };

    //=========================================================================
    // private variables.
    //=========================================================================
    RefPtr<ID3D12Resource>      m_VB[2];                                            //!< 頂点バッファ.
    RefPtr<ID3D12Resource>      m_IB;                                               //!< インデックスバッファ.
    RefPtr<ID3D12RootSignature> m_RootSig;                                          //!< ルートシグニチャ.
    RefPtr<ID3D12PipelineState> m_DefaultState;                                     //!< パイプラインステート.
    std::vector<Batch>          m_Batches;                                          //!< バッチ.
    Vertex*                     m_pVertices[2]      = {};                           //!< マップ済みメモリ.
    uint32_t                    m_SpriteCount       = 0;                            //!< 描画スプライト数.
    uint32_t                    m_BatchCount        = 0;                            //!< 描画バッチ数.
    uint32_t                    m_SubmitCount       = 0;                            //!< サブミット数.
    uint32_t                    m_MaxSpriteCount    = 0;                            //!< 最大スプライト数.
    uint32_t                    m_MaxBatchCount     = 0;                            //!< 最大バッチ数.
    uint8_t                     m_BufferIndex       = 0;                            //!< バッファインデックス.
    Matrix                      m_Transform         = Matrix::CreateIdentity();     //!< 変換行列.
    Vector2                     m_ScreenSize        = Vector2(0, 0);                //!< スクリーンサイズ.
    Unorm4                      m_Color             = {};                           //!< 頂点カラー.
    D3D12_GPU_DESCRIPTOR_HANDLE m_HandleSRV         = {};                           //!< シェーダリソースビュー.
    D3D12_GPU_DESCRIPTOR_HANDLE m_HandleSampler     = {};                           //!< サンプラー
    DXGI_FORMAT                 m_ColorFormat       = DXGI_FORMAT_UNKNOWN;
    DXGI_FORMAT                 m_DepthFormat       = DXGI_FORMAT_UNKNOWN;
    uint32_t                    m_Param[4]          = {};
    AllocationHolder            m_AllocationVB[2];
    AllocationHolder            m_AllocationIB;
    ID3D12PipelineState*        m_pCurrentState     = nullptr;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

//-----------------------------------------------------------------------------
//! @brief      9 slices を描画します.
//! 
//! @param[in]      renderer    スプライトレンダラー.
//! @param[in]      x           左上のX座標.
//! @param[in]      y           左上のY座標.
//! @param[in]      w           描画する横幅.
//! @param[in]      h           描画する縦幅.
//! @param[in]      l           左フレーム枠終了位置へのオフセット.
//! @param[in]      r           右フレーム枠開始位置へのオフセット.
//! @param[in]      t           上フレーム枠終了位置へのオフセット.
//! @param[in]      b           下フレーム枠開始位置へのオフセット.
//! @param[in]      texW        テクスチャの横幅.
//! @param[in]      texH        テクスチャの縦幅.
//! @param[in]      texL        テクスチャ内の左フレーム枠終了位置へのオフセット.
//! @param[in]      texR        テクスチャ内の右フレーム枠開始位置へのオフセット.
//! @param[in]      texT        テクスチャ内の上フレーム枠終了位置へのオフセット.
//! @param[in]      texB        テクスチャ内の下フレーム枠終了位置へのオフセット.
//-----------------------------------------------------------------------------
void Draw9Slices(
    SpriteRenderer& renderer,
    int x, int y, int w, int h,
    int l, int r, int t, int b,
    int texW, int texH,
    int texL, int texR, int texT, int texB);

//-----------------------------------------------------------------------------
//! @brief      9 slices を描画します.
//! 
//! @param[in]      renderer    スプライトレンダラー.
//! @param[in]      x           左上のX座標.
//! @param[in]      y           左上のY座標.
//! @param[in]      w           描画する横幅.
//! @param[in]      h           描画する縦幅.
//! @param[in]      texW        テクスチャの横幅.
//! @param[in]      texH        テクスチャの縦幅.
//! @param[in]      texSX       フレーム枠のテクスチャ上でのX方向のピクセルサイズ.
//! @param[in]      texSY       フレーム枠のテクスチャ上でのY方向のピクセルサイズ.
//-----------------------------------------------------------------------------
void Draw9Slices(
    SpriteRenderer& renderer,
    int x, int y, int w, int h,
    int texW, int texH, int texSX, int texSY);

//-----------------------------------------------------------------------------
//! @brief      9 slices を描画します.
//! 
//! @param[in]      renderer    スプライトレンダラー.
//! @param[in]      x           左上のX座標.
//! @param[in]      y           左上のY座標.
//! @param[in]      w           描画する横幅.
//! @param[in]      h           描画する縦幅.
//! @param[in]      texW        テクスチャの横幅.
//! @param[in]      texH        テクスチャの縦幅.
//! @note       テクスチャが均等に分割されていることを前提として描画を行います.
//-----------------------------------------------------------------------------
void Draw9Slices(
    SpriteRenderer& renderer,
    int x, int y, int w, int h, int texW, int texH);

} // namespace asdx

