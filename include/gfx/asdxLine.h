//-----------------------------------------------------------------------------
// File : asdxLine.h
// Desc : Line Renderer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include <d3d12.h>
#include <fnd/asdxMath.h>
#include <fnd/asdxRef.h>
#include <gfx/asdxAllocationHolder.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// LineRenderer class
///////////////////////////////////////////////////////////////////////////////
class LineRenderer
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
    LineRenderer();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~LineRenderer();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //! 
    //! @param[in]      maxLineCount        最大ライン数.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(uint32_t maxLineCount, DXGI_FORMAT rtvFormat, DXGI_FORMAT dstFormat);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      リセット処理を行います.
    //! 
    //! @note       フレーム開始前に呼び出してください.
    //-------------------------------------------------------------------------
    void Reset();

    //-------------------------------------------------------------------------
    //! @brief      ラインを追加します.
    //! 
    //! @param[in]      v0          頂点0の位置座標.
    //! @param[in]      v1          頂点1の位置座標.
    //! @param[in]      c0          頂点0のカラー.
    //! @param[in]      c1          頂点1のカラー.
    //--------------------------------------------------------------------------
    void Add(const Vector3& v0, const Vector3& v1, const Vector4& c0, const Vector4& c1);

    //-------------------------------------------------------------------------
    //! @brief      ラインを追加します.
    //! 
    //! @param[in]      v0          頂点0の位置座標.
    //! @param[in]      v1          頂点1の位置座標.
    //! @param[in]      color       頂点カラー.
    //-------------------------------------------------------------------------
    void Add(const Vector3& v0, const Vector3& v1, const Vector4& color)
    { Add(v0, v1, color, color); }

    //-------------------------------------------------------------------------
    //! @brief      パイプラインステートを設定します.
    //! 
    //! @param[in]      pCmdList        グラフィックスコマンドリストです.
    //! @param[in]      pPipelineState  パイプラインステートです(nullptrの場合はデフォルトのパイプラインステートが設定されます).
    //- ------------------------------------------------------------------------
    void SetPipelineState(ID3D12GraphicsCommandList* pCmd, ID3D12PipelineState* pPipelineState);

    //-------------------------------------------------------------------------
    //! @brief      パイプラインステートを設定します.
    //!
    //! @param[in]      pCmd        グラフィックスコマンドリスト.
    //--------------------------------------------------------------------------
    void SetPipelineState(ID3D12GraphicsCommandList* pCmd)
    { SetPipelineState(pCmd, nullptr); }

    //-------------------------------------------------------------------------
    //! @brief      描画処理を行います.
    //! 
    //! @param[in]      pCmd        グラフィックスコマンドリスト.
    //! @note       描画前に，RootIndex = 0 にビュー行列と射影行列を持つ定数バッファを設定してください.
    //--------------------------------------------------------------------------
    void Draw(ID3D12GraphicsCommandList* pCmd);

private:
    ///////////////////////////////////////////////////////////////////////////
    // Vertex structure
    ///////////////////////////////////////////////////////////////////////////
    struct Vertex
    {
        Vector3 Position;   //!< 位置座標.
        Unorm4  Color;      //!< 頂点カラー.
    };

    //=========================================================================
    // private variables.
    //=========================================================================
    RefPtr<ID3D12Resource>      m_VB[2];
    RefPtr<ID3D12RootSignature> m_RootSig;
    RefPtr<ID3D12PipelineState> m_PSO;
    Vertex*                     m_pVertices[2]      = {};
    uint32_t                    m_LineCount         = 0;
    uint32_t                    m_SubmitCount       = 0;
    uint32_t                    m_MaxLineCount      = 0;
    AllocationHolder            m_AllocationVB[2]   = {};
    uint8_t                     m_BufferIndex       = 0;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

//-----------------------------------------------------------------------------
//! @brief      ワイヤーフレームボックスを描画します.
//! 
//! @param[in]      renderer        線分レンダラー.
//! @param[in]      mini            最小座標.
//! @param[in]      maxi            最大座標.
//! @param[in]      color           カラー.
//-----------------------------------------------------------------------------
void DrawWireBox(
    LineRenderer&   renderer,
    const Vector3&  mini,
    const Vector3&  maxi,
    const Vector4&  color);

//-----------------------------------------------------------------------------
//! @brief      ワイヤーフレームスフィアを描画します.
//! 
//! @param[in]      renderer        線分レンダラー.
//! @param[in]      center          中心座標.
//! @param[in]      radius          半径.
//! @param[in]      color           カラー.
//! @param[in]      segment         分割数.
//-----------------------------------------------------------------------------
void DrawWireSphere(
    LineRenderer&   renderer,
    const Vector3&  center,
    float           radius,
    const Vector4&  color,
    uint32_t        segment = 16);

//-----------------------------------------------------------------------------
//! @brief      ワイヤーフレーム半球を描画します.
//! 
//! @param[in]      renderer        線分レンダラー.
//! @param[in]      center          中心座標.
//! @param[in]      radius          半径.
//! @param[in]      upward          上方向ベクトル.
//! @param[in]      color           カラー.
//! @param[in]      segment         分割数.
//-----------------------------------------------------------------------------
void DrawWireHemisphere(
    LineRenderer&   renderer,
    const Vector3&  center,
    float           radius,
    const Vector3&  up,
    const Vector4&  color,
    uint32_t        segment = 16);

//-----------------------------------------------------------------------------
//! @brief      ワイヤーフレーム円錐を描画します.
//! 
//! @param[in]      renderer        線分レンダラー.
//! @param[in]      apex            天頂座標.
//! @param[in]      baseCenter      底面中心座標.
//! @param[in]      baseRadius      底面半径.
//! @param[in]      color           カラー.
//! @param[in]      segment         分割数.
//-----------------------------------------------------------------------------
void DrawWireCone(
    LineRenderer&   renderer,
    const Vector3&  apex,
    const Vector3&  baseCenter,
    float           baseRadius,
    const Vector4&  color,
    uint32_t        segment = 16);

//-----------------------------------------------------------------------------
//! @brief      ワイヤーフレーム四角錐を描画します.
//! 
//! @param[in]      renderer        線分レンダラー.
//! @param[in]      apex            天頂座標.
//! @param[in]      baseCenter      底面中心座標.
//! @param[in]      baseRadius      底面半径.
//! @param[in]      color           カラー.
//! -----------------------------------------------------------------------------
void DrawWirePyramid(
    LineRenderer&   renderer,
    const Vector3&  apex,
    const Vector3&  baseCenter,
    float           baseRadius,
    const Vector4&  color);

//-----------------------------------------------------------------------------
//! @brief      ワイヤーフレーム円柱を描画します.
//!
//! @param[in]      renderer        線分レンダラー.
//! @param[in]      basecenter      円柱の底面中心.
//! @param[in]      topCenter       円柱の天面中心.
//! @param[in]      radius          半径.
//! @param[in]      color           カラー.
//! @param[in]      segment         分割数.
//-----------------------------------------------------------------------------
void DrawWireCylinder(
    LineRenderer&   renderer,
    const Vector3&  baseCenter,
    const Vector3&  topCenter,
    float           radius,
    const Vector4&  color,
    uint32_t        segment = 16);

//-----------------------------------------------------------------------------
//! @brief      ワイヤーフレーム平面を描画します.
//! 
//! @param[in]      renderer        線分レンダラー.
//! @param[in]      center          中心座標.
//! @param[in]      normal          法線ベクトル.
//! @param[in]      size            サイズ.
//! @param[in]      color           カラー.
//-----------------------------------------------------------------------------
void DrawWirePlane(
    LineRenderer&   renderer,
    const Vector3&  center,
    const Vector3&  normal,
    float           size,
    const Vector4&  color);

//-----------------------------------------------------------------------------
//! @brief      ワイヤーフレームカプセルを描画します.
//! 
//! @param[in]      renderer        線分レンダラー.
//! @param[in]      baseCenter      カプセルの下側中心.
//! @param[in]      topCenter       カプセルの上側中心.
//! @param[in]      radius          半径.
//! @param[in]      color           カラー.
//! @param[in]      segment         分割数.
//-----------------------------------------------------------------------------
void DrawWireCapsule(
    LineRenderer&   renderer,
    const Vector3&  baseCenter,
    const Vector3&  topCenter,
    float           radius,
    const Vector4&  color,
    uint32_t        segment = 16);

//-----------------------------------------------------------------------------
//! @brief      ワイヤーフレーム円盤を描画します.
//! 
//! @param[in]      renderer        線分レンダラー.
//! @param[in]      center          中心座標.
//! @param[in]      normal          法線ベクトル.
//! @param[in]      radius          半径.
//! @param[in]      color           カラー.
//! @param[in]      segment         分割数.
//-----------------------------------------------------------------------------
void DrawWireDisk(
    LineRenderer&   renderer,
    const Vector3&  center,
    const Vector3&  normal,
    float           radius,
    const Vector4&  color,
    uint32_t        segment = 16);

//-----------------------------------------------------------------------------
//! @brief      ワイヤーフレーム扇形を描画します.
//!
//! @param[in]      renderer        線分レンダラー.
//! @param[in]      center          中心座標.
//! @param[in]      normal          法線ベクトル.
//! @param[in]      radius          半径.
//! @param[in]      angleDegree     角度(度数法).
//! @param[in]      color           カラー.
//! @param[in]      segment         分割数.
//-----------------------------------------------------------------------------
void DrawWireFan(
    LineRenderer&   renderer,
    const Vector3&  center,
    const Vector3&  normal,
    float           radius,
    float           angleDegree,
    const Vector4&  color,
    uint32_t        segment = 16);

//-----------------------------------------------------------------------------
//! @brief      ワイヤーフレームボーンを描画します.
//! 
//! @param[in]      renderer        線分レンダラー.
//! @param[in]      start           開始座標.
//! @param[in]      end             終了座標.
//! @param[in]      color           カラー.
//-----------------------------------------------------------------------------
void DrawWireBone(
    LineRenderer&   renderer,
    const Vector3&  start,
    const Vector3&  end,
    const Vector4&  color);

//-----------------------------------------------------------------------------
//! @brief      ワイヤーフレーム錐台を描画します.
//! 
//! @param[in]      renderer        線分レンダラー.
//! @param[in]      invViewProj     ビュー射影行列の逆行列.
//! @param[in]      color           カラー.
//-----------------------------------------------------------------------------
void DrawWireFrustum(
    LineRenderer&       renderer,
    const Matrix4x4&    invViewProj,
    const Vector4&      color);

//-----------------------------------------------------------------------------
//! @brief      ワイヤーフレーム軸を描画します.
//! 
//! @param[in]      renderer        線分レンダラー.
//! @param[in]      world           ワールド行列.
//! @param[in]      length          軸の長さ.
//! @param[in]      colorX          X軸のカラー.
//! @param[in]      colorY          Y軸のカラー.
//! @param[in]      colorZ          Z軸のカラー.
//-----------------------------------------------------------------------------
void DrawWireAxis(
    LineRenderer&       renderer,
    const Matrix4x4&    world,
    float               length,
    const Vector4&      colorX,
    const Vector4&      colorY,
    const Vector4&      colorZ);

//-----------------------------------------------------------------------------
//! @brief      六角形を描画します.
//! 
//! @param[in]      renderer        線分レンダラー.
//! @param[in]      center          中心座標.
//! @param[in]      length          長さ(半径相当).
//! @param[in]      color           カラー.
//-----------------------------------------------------------------------------
void DrawWireHexagon(
    LineRenderer&   renderer,
    const Vector3&  center,
    float           length,
    const Vector4&  color);

//-----------------------------------------------------------------------------
//! @brief      四角形グリッドを描画します.
//! 
//! @param[in]      renderer        線分レンダラー.
//! @param[in]      halfRange       片側範囲.
//! @param[in]      size            サイズ.
//! @param[in]      color           カラー.
//-----------------------------------------------------------------------------
void DrawSquareGrid(
    LineRenderer&   renderer,
    int             halfRange,
    float           size,
    const Vector4&  color);

//-----------------------------------------------------------------------------
//! @brief      六角形グリッドを描画します.
//! 
//! @param[in]      renderer        線分レンダラー.
//! @param[in]      halfRange       片側範囲.
//! @param[in]      size            サイズ.
//! @param[in]      color           カラー.
//-----------------------------------------------------------------------------
void DrawHexGrid(
    LineRenderer&   renderer,
    int             halfRange,
    float           size,
    const Vector4&  color);

} // namespace asdx
