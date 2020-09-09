﻿//-----------------------------------------------------------------------------
// File : asdxGraphicsDevice.h
// Desc : Graphics Device.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <mutex>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <asdxRef.h>
#include <asdxCommandQueue.h>
#include <asdxDescriptor.h>
#include <asdxResourceUploader.h>
#include <asdxDisposer.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// GraphicsDevice class
///////////////////////////////////////////////////////////////////////////////
class GraphicsDevice
{
    //========================================================================
    // list of friend classes and methods.
    //========================================================================
    /* NOTHING */

public:
    ///////////////////////////////////////////////////////////////////////////
    // Desc structure
    ///////////////////////////////////////////////////////////////////////////
    struct Desc
    {
        uint32_t    MaxShaderResourceCount;         //!< 最大シェーダリソース数です.
        uint32_t    MaxSamplerCount;                //!< 最大サンプラー数です.
        uint32_t    MaxColorTargetCount;            //!< 最大カラーターゲット数です.
        uint32_t    MaxDepthTargetCount;            //!< 最大深度ターゲット数です.
        bool        EnableDebug;                    //!< デバッグモードを有効にします.
    };

    //=========================================================================
    // public variables.
    //=========================================================================
    static const uint8_t kDefaultLiftTime = 4;

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      唯一のインスタンスを取得します.
    //!
    //! @return     唯一のインスタンスを返却します.
    //-------------------------------------------------------------------------
    static GraphicsDevice& Instance();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      desc        構成設定です.
    //! @retval true    初期化に成功.
    //! @retval fasle   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(const Desc* desc);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      ID3D12Device8を取得します.
    //!
    //! @return     ID3D12Device8を返却します.
    //-------------------------------------------------------------------------
    ID3D12Device8* GetDevice() const;

    //-------------------------------------------------------------------------
    //! @brief      IDXGIFactory7を取得します.
    //!
    //! @return     IDXGIFactory7を返却します.
    //-------------------------------------------------------------------------
    IDXGIFactory7* GetFactory() const;

    //-------------------------------------------------------------------------
    //! @brief      グラフィックスキューを取得します.
    //!
    //! @return     グラフィックスキューを返却します.
    //-------------------------------------------------------------------------
    CommandQueue* GetGraphicsQueue() const;

    //-------------------------------------------------------------------------
    //! @brief      コンピュートキューを取得します.
    //!
    //! @return     コンピュートキューを返却します.
    //-------------------------------------------------------------------------
    CommandQueue* GetComputeQueue() const;

    //-------------------------------------------------------------------------
    //! @brief      コピーキューを取得します.
    //!
    //! @return     コピーキューを返却します.
    //-------------------------------------------------------------------------
    CommandQueue* GetCopyQueue() const;

    //-------------------------------------------------------------------------
    //! @brief      ビデオデコードキューを取得します.
    //!
    //! @return     ビデオデコードキューを返却します.
    //-------------------------------------------------------------------------
    CommandQueue* GetVideoDecodeQueue() const;

    //-------------------------------------------------------------------------
    //! @brief      ビデオプロセスキューを取得します.
    //!
    //! @return     ビデオプロセスキューを返却します.
    //-------------------------------------------------------------------------
    CommandQueue* GetVideoProcessQueue() const;

    //-------------------------------------------------------------------------
    //! @brief      ビデオエンコードキューを取得します.
    //!
    //! @return     ビデオエンコードキューを返却します.
    //-------------------------------------------------------------------------
    CommandQueue* GetVideoEncodeQueue() const;

    //-------------------------------------------------------------------------
    //! @brief      ディスクリプターを確保します.
    //!
    //! @param[in]      index       ディスクリプタタイプです.
    //! @param[out]     ppResult    ディスクリプタの確保先です.
    //! @retval true    確保に成功.
    //! @retval false   確保に失敗.
    //-------------------------------------------------------------------------
    bool AllocHandle(int index, Descriptor** ppResult);

    //-------------------------------------------------------------------------
    //! @brief      アロー演算子です.
    //-------------------------------------------------------------------------
    ID3D12Device8* operator-> () const;

    //-------------------------------------------------------------------------
    //! @brief      ディスクリプタヒープを設定します.
    //-------------------------------------------------------------------------
    void SetDescriptorHeaps(ID3D12GraphicsCommandList* pCmdList);

    //-------------------------------------------------------------------------
    //! @brief      コマンドキューの実行完了を待機します.
    //-------------------------------------------------------------------------
    void WaitIdle();

    //-------------------------------------------------------------------------
    //! @brief      リソースアップローダーに追加します.
    //!
    //! @param[in]      pResource       アップロードリソース.
    //! @param[in]      lifeTime        生存フレーム数です.
    //-------------------------------------------------------------------------
    void PushToResourceUploader(
        IUploadResource* pResource,
        uint8_t lifeTime = kDefaultLiftTime);

    //-------------------------------------------------------------------------
    //! @brief      リソースディスポーザーに追加します.
    //!
    //! @param[in]      pResource       破棄リソース.
    //! @param[in]      lifeTime        生存フレーム数です.
    //--------------------------------------------------------------------------
    void PushToResourceDisposer(
        ID3D12Resource* pResource, 
        uint8_t lifeTime = kDefaultLiftTime);

    //-------------------------------------------------------------------------
    //! @brief      ディスクリプタディスポーザーに追加します.
    //!
    //! @param[in]      pDescriptor     破棄ディスクリプタ.
    //! @param[in]      lifeTime        生存フレーム数です.
    //-------------------------------------------------------------------------
    void PushToDescriptorDisposer(
        Descriptor* pDescriptor,
        uint8_t lifeTime = kDefaultLiftTime);

    //-------------------------------------------------------------------------
    //! @brief      アップロードコマンドを設定します.
    //!
    //! @param[in]      pCmdList        コマンドリストです.
    //-------------------------------------------------------------------------
    void SetUploadCommand(ID3D12GraphicsCommandList* pCmdList);

    //-------------------------------------------------------------------------
    //! @brief      フレーム同期を行います.
    //-------------------------------------------------------------------------
    void FrameSync();

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    static GraphicsDevice       s_Instance;                 //!< シングルトンインスタンス.
    RefPtr<IDXGIFactory7>       m_pFactory;                 //!< DXGIファクトリーです.
    RefPtr<ID3D12Debug3>        m_pDebug;                   //!< デバッグオブジェクト.
    RefPtr<ID3D12InfoQueue>     m_pInfoQueue;               //!< インフォキュー.
    RefPtr<ID3D12Device8>       m_pDevice;                  //!< デバイス.
    RefPtr<CommandQueue>        m_pGraphicsQueue;           //!< グラフィックスキュー.
    RefPtr<CommandQueue>        m_pComputeQueue;            //!< コンピュートキュー.
    RefPtr<CommandQueue>        m_pCopyQueue;               //!< コピーキュー.
    RefPtr<CommandQueue>        m_pVideoDecodeQueue;        //!< ビデオデコードキュー.
    RefPtr<CommandQueue>        m_pVideoProcessQueue;       //!< ビデオプロセスキュー.
    RefPtr<CommandQueue>        m_pVideoEncodeQueue;        //!< ビデオエンコードキュー.
    DescriptorHeap              m_DescriptorHeap[4];        //!< ディスクリプタヒープ.
    ResourceUploader            m_ResourceUploader;         //!< リソースアップローダー.
    Disposer<ID3D12Resource>    m_ResourceDisposer;         //!< リソースディスポーザー.
    Disposer<Descriptor>        m_DescriptorDisposer;       //!< ディスクリプタディスポーザー.
    std::mutex                  m_Mutex;

    //=========================================================================
    // private methods
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    GraphicsDevice();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~GraphicsDevice();

    GraphicsDevice              (const GraphicsDevice&) = delete;   // アクセス禁止.
    GraphicsDevice& operator =  (const GraphicsDevice&) = delete;   // アクセス禁止.
};

//-----------------------------------------------------------------------------
//! @brief      グラフィックスデバイスを取得します.
//!
//! @return     グラフィックスデバイスを返却します.
//-----------------------------------------------------------------------------
inline GraphicsDevice& GfxDevice()
{ return GraphicsDevice::Instance(); }

} // namespace asdx
