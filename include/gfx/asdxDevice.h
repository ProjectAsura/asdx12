﻿//-----------------------------------------------------------------------------
// File : asdxDevice.h
// Desc : Graphics Device.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <d3d12.h>
#include <dxgi1_6.h>
#include <res/asdxResTexture.h>
#include <vector>


namespace asdx {

//-----------------------------------------------------------------------------
// Forward Declarations.
//-----------------------------------------------------------------------------
class CommandQueue;

///////////////////////////////////////////////////////////////////////////////
// COMMAND_SIGNATURE_TYPE enum
///////////////////////////////////////////////////////////////////////////////
enum COMMAND_SIGNATURE_TYPE
{
    COMMAND_SIGNATURE_TYPE_DRAW,
    COMMAND_SIGNATURE_TYPE_DRAW_INDEXED,
    COMMAND_SIGNATURE_TYPE_DISPATCH,
    COMMAND_SIGNATURE_TYPE_DISPATCH_RAYS,
    COMMAND_SIGNATURE_TYPE_DISPATCH_MESH,
    MAX_COUNT_COMMAND_SIGNATURE_TYPE,
};

///////////////////////////////////////////////////////////////////////////////
// DisplayInfo structure
///////////////////////////////////////////////////////////////////////////////
struct DisplayInfo
{
    uint32_t        Width;
    uint32_t        Height;
    DXGI_RATIONAL   RefreshRate;
};

///////////////////////////////////////////////////////////////////////////////
// DeviceDesc structure
///////////////////////////////////////////////////////////////////////////////
struct DeviceDesc
{
    uint32_t    MaxShaderResourceCount;         //!< 最大シェーダリソース数です.
    uint32_t    MaxSamplerCount;                //!< 最大サンプラー数です.
    uint32_t    MaxColorTargetCount;            //!< 最大カラーターゲット数です.
    uint32_t    MaxDepthTargetCount;            //!< 最大深度ターゲット数です.
    bool        EnableDebug          = false;   //!< デバッグモードを有効にします.
    bool        EnableDRED           = true;    //!< DREDを有効にします
    bool        EnableCapture        = false;   //!< PIXキャプチャーを有効にします.
    bool        EnableBreakOnWarning = false;   //!< 警告時にブレークするなら true.
    bool        EnableBreakOnError   = true;    //!< エラー時にブレークするなら true.
};

//-----------------------------------------------------------------------------
//! @brief      システムの初期化を行います.
//-----------------------------------------------------------------------------
bool SystemInit(const DeviceDesc& desc);

//-----------------------------------------------------------------------------
//! @brief      システムの終了処理を行います.
//-----------------------------------------------------------------------------
void SystemTerm();

//-----------------------------------------------------------------------------
//! @brief      システムがアイドル状態になるまで待機します.
//-----------------------------------------------------------------------------
void SystemWaitIdle();

//-----------------------------------------------------------------------------
//! @brief      フレーム同期します.
//-----------------------------------------------------------------------------
void FrameSync();

//-----------------------------------------------------------------------------
//! @brief      オブジェクトを破棄します.
//-----------------------------------------------------------------------------
void DisposeObject(ID3D12Object*& pResource);

//-----------------------------------------------------------------------------
//! @brief      ディスポーザーをクリアします.
//-----------------------------------------------------------------------------
void ClearDisposer();

//-----------------------------------------------------------------------------
//! @brief      ディスクリプタヒープを設定します.
//-----------------------------------------------------------------------------
void SetDescriptorHeaps(ID3D12GraphicsCommandList* pCmdList);

//-----------------------------------------------------------------------------
//! @brief      サブリソースを更新します.
//-----------------------------------------------------------------------------
void UpdateSubResources(
    ID3D12GraphicsCommandList*      pCmdList,
    ID3D12Resource*                 pDstResource,
    uint32_t                        subResourceCount,
    uint32_t                        subResourceOffset,
    const D3D12_SUBRESOURCE_DATA*   pSubResources);

//-----------------------------------------------------------------------------
//! @brief      バッファを更新します.
//-----------------------------------------------------------------------------
void UpdateBuffer(
    ID3D12GraphicsCommandList*      pCmdList,
    ID3D12Resource*                 pDstResource,
    const void*                     pSrcResource);

//-----------------------------------------------------------------------------
//! @brief      テクスチャを更新します.
//-----------------------------------------------------------------------------
void UpdateTexture(
    ID3D12GraphicsCommandList*      pCmdList,
    ID3D12Resource*                 pDstResource,
    const ResTexture*               pSrcResource);

//-----------------------------------------------------------------------------
//! @brief      フルスクリーン矩形用入力レイアウトを取得します.
//-----------------------------------------------------------------------------
D3D12_INPUT_LAYOUT_DESC GetQuadLayout();

//-----------------------------------------------------------------------------
//! @brief      フルスクリーン矩形を描画します.
//-----------------------------------------------------------------------------
void DrawQuad(ID3D12GraphicsCommandList* pCmd);

//-----------------------------------------------------------------------------
//! @brief      フルスクリーン矩形用頂点シェーダを取得します.
//-----------------------------------------------------------------------------
D3D12_SHADER_BYTECODE GetQuadVS();

//-----------------------------------------------------------------------------
//! @brief      グラフィックスキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GetGraphicsQueue();

//-----------------------------------------------------------------------------
//! @brief      コンピュートキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GetComputeQueue();

//-----------------------------------------------------------------------------
//! @brief      コピーキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GetCopyQueue();

//-----------------------------------------------------------------------------
//! @brief      ビデオプロセスキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GetVideoProcessQueue();

//-----------------------------------------------------------------------------
//! @brief      ビデオエンコードキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GetVideoEncodeQueue();

//-----------------------------------------------------------------------------
//! @brief      ビデオデコードキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GetVideoDecodeQueue();

//-----------------------------------------------------------------------------
//! @brief      デバイスを取得します.
//-----------------------------------------------------------------------------
ID3D12Device8* GetD3D12Device();

//-----------------------------------------------------------------------------
//! @brief      DXGIファクトリを取得します.
//-----------------------------------------------------------------------------
IDXGIFactory7* GetDXGIFactory();

//-----------------------------------------------------------------------------
//! @brief      コマンドシグニチャを取得します.
//-----------------------------------------------------------------------------
ID3D12CommandSignature* GetCommandSignature(COMMAND_SIGNATURE_TYPE type);

//-----------------------------------------------------------------------------
//! @brief      スタティックサンプラーを取得します.
//-----------------------------------------------------------------------------
const D3D12_STATIC_SAMPLER_DESC* GetStaticSamplers();

//-----------------------------------------------------------------------------
//! @brief      スタティックサンプラー数を取得します.
//-----------------------------------------------------------------------------
uint32_t GetStaticSamplerCounts();

//-----------------------------------------------------------------------------
//! @brief      ディスプレイ情報を取得します.
//-----------------------------------------------------------------------------
void GetDisplayInfo(DXGI_FORMAT format, std::vector<DisplayInfo>& result);

//-----------------------------------------------------------------------------
//! @brief      破棄処理を行います.
//-----------------------------------------------------------------------------
template<typename T>
inline void Dispose(T*& ptr)
{
    auto casted = reinterpret_cast<ID3D12Object*>(ptr);
    DisposeObject(casted);
}

} // namespace asdx
