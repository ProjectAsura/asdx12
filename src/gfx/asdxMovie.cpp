//-----------------------------------------------------------------------------
// File : asdxMovie.cpp
// Desc : Movie Stream.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Inlcudes
//-----------------------------------------------------------------------------
#include <cassert>
#include <atomic>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMisc.h>
#include <gfx/asdxMovie.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxDescriptorHeap.h>

#include <mfapi.h>
#include <mfidl.h>
#include <Mferror.h>
#include <mfreadwrite.h>
#include <propvarutil.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// MovieStream class
///////////////////////////////////////////////////////////////////////////////
class MovieStream : public IMovieStream
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
    //      生成処理を行います.
    //-------------------------------------------------------------------------
    static bool Create(IMovieStream** ppResult)
    {
        auto instance = new(std::nothrow) MovieStream();
        if (instance == nullptr)
        {
            ELOGA("Error : Out of Memory.");
            return false;
        }

        return *ppResult = instance;
    }

    //-------------------------------------------------------------------------
    //      参照カウントを上げます.
    //-------------------------------------------------------------------------
    void AddRef() override
    { m_RefCount++; }

    //-------------------------------------------------------------------------
    //      解放処理を行います.
    //-------------------------------------------------------------------------
    void Release() override
    {
        m_RefCount--;
        if (m_RefCount == 0)
        { delete this; }
    }

    //-------------------------------------------------------------------------
    //      参照カウントを取得します.
    //-------------------------------------------------------------------------
    uint32_t GetRefCount() const override
    { return m_RefCount; }

    //-------------------------------------------------------------------------
    //      オープンします.
    //-------------------------------------------------------------------------
    bool Open(const char* path) override
    {
        // ファイル名チェック.
        if (path == nullptr)
        {
            ELOG("Error : Invalid Argument.");
            return false;
        }

        // Media Foundation の初期化.
        {
            auto hr = MFStartup(MF_VERSION);
            if (FAILED(hr))
            {
                ELOGA("Error : MFStartup() Failed. errcode = 0x%x", hr);
                return false;
            }
        }

        // 動画を読み込み.
        {
            RefPtr<IMFAttributes> attributes;
            auto attrCount = 3;
            auto hr = MFCreateAttributes(attributes.GetAddress(), attrCount);
            if (FAILED(hr))
            {
                ELOGA("Error : MFCreateAttributes() Failed. errcode = 0x%x", hr);
                return false;
            }

            // ビデオ処理を有効化.
            hr = attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
            assert(SUCCEEDED(hr));

            // コード変換用に最適化された Media Foundation 変換を使用.
            hr = attributes->SetUINT32(MF_SOURCE_READER_ENABLE_TRANSCODE_ONLY_TRANSFORMS, TRUE);
            assert(SUCCEEDED(hr));

            // DirectX Video Acceleration を有効.
            hr = attributes->SetUINT32(MF_SOURCE_READER_DISABLE_DXVA, FALSE);
            assert(SUCCEEDED(hr));

            // ワイド文字列に変換.
            std::wstring widePath = asdx::ToStringW(path);

            // ソースコードリーダーを生成.
            hr = MFCreateSourceReaderFromURL(widePath.c_str(), attributes.GetPtr(), m_Reader.GetAddress());
            if (FAILED(hr))
            {
                ELOGA("Error : MFCreateSourceReaderFromURL() Failed. errcode = 0x%x, path = %s", hr, path);
                return false;
            }
        }

        // ビデオデコーダーの設定.
        if (!ConfigureVideoDecoder(MFVideoFormat_NV12))
        {
            ELOG("Error : ConfigureVideoDecoder() Failed.");
            return false;
        }

        // ビデオ情報を取得.
        if (!GetVideoInfo())
        {
            ELOG("Error : GetVideoInfo() Failed.");
            return false;
        }

        m_Finished    = false;
        m_FrameIndex  = 0;
        m_Format      = DXGI_FORMAT_NV12;
        m_PlayTimeSec = 0.0;

        // 正常終了.
        return true;
    }

    //-------------------------------------------------------------------------
    //      クローズします.
    //-------------------------------------------------------------------------
    void Close() override
    {
        m_Reader.Reset();

        m_Width             = 0;
        m_Height            = 0;
        m_Format            = DXGI_FORMAT_UNKNOWN;
        m_FrameRate         = 0;
        m_DurationSec       = 0;
        m_FrameCount        = 0;
        m_FrameIndex        = 0;
        m_LastTimeStamp     = 0;
        m_Pause             = false;
        m_Finished          = true;
        m_PlayTimeSec       = 0.0;

        auto hr = MFShutdown();
        if (FAILED(hr))
        { ELOGA("Error : MFShutdown() Failed. errcode = 0x%x", hr); }
    }

    //-------------------------------------------------------------------------
    //      フレームシークを行います.
    //-------------------------------------------------------------------------
    bool Seek(int64_t time) override
    {
        if (m_Reader.GetPtr() == nullptr)
            return false;

        // TODO : シーク時間の設定値が合っているかどうかチェック.

        PROPVARIANT pos = {};
        InitPropVariantFromInt64(time, &pos);

        auto hr = m_Reader->SetCurrentPosition(GUID_NULL, pos);
        PropVariantClear(&pos);
        if (FAILED(hr))
        {
            ELOGA("Error : IMFSourceReader::SetCurrentPosition() failed. errcode = 0x%x", hr);
            return false;
        }

        // シークしたので終了フラグは外す.
        m_Finished = false;

        // 再生時刻を設定.
        m_PlayTimeSec = time / 10000000.0;

        return true;
    }

    //-------------------------------------------------------------------------
    //      フレームデータを読み込みます.
    //-------------------------------------------------------------------------
    bool Read(ID3D12Resource* pDstResource, double elapsedSec) override
    {
        if (pDstResource == nullptr)
        {
            ELOGA("Error : Invalid Argument.");
            return false;
        }

        if (m_Reader.GetPtr() == nullptr)
        {
            ELOGA("Error : Invalid Operation.");
            return false;
        }

        // 再生完了状態.
        if (m_Finished)
        { return false; }

        DWORD    flags     = 0;
        LONGLONG timeStamp = m_LastTimeStamp;
        HRESULT  hr        = S_OK;

        // サンプルを読み込み.
        if (m_Sample.GetPtr() == nullptr)
        {
            hr = m_Reader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, &flags, &timeStamp, m_Sample.GetAddress());
            if (FAILED(hr))
            {
                WLOGA("Warning : IMFSourceReader::ReadSample() failed. errcode = 0x%x", hr);
                return false;
            }
        }

        // 一時停止でなければ時間を進める.
        if (!m_Pause)
        { m_PlayTimeSec += elapsedSec; }

        auto timeSec = double(timeStamp) / 10000000.0;
        auto isShow  = (m_PlayTimeSec >= timeSec);

        // フレーム情報を更新.
        m_LastTimeStamp = timeStamp;
        m_FrameIndex    = uint64_t(m_FrameRate * timeStamp);

        // 表示タイミングでなければ終了.
        if (!isShow)
        { return false; }

        if (m_CalcFrameCount)
        { m_FrameCount++; }

        // 再生完了判定.
        if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
        {
            m_Finished = true;
            if (m_CalcFrameCount)
            {
                m_DurationSec    = double(m_FrameCount) / m_FrameRate;
                m_CalcFrameCount = false;
            }

            if (m_Loop)
            { Seek(0); }

            return false;
        }

        // バッファ取得.
        RefPtr<IMFMediaBuffer> buffer;
        hr = m_Sample->GetBufferByIndex(0, buffer.GetAddress());
        if (FAILED(hr))
        {
            WLOGA("Warning : IMFSample::GetBufferByIndex() failed. errcode = 0x%x", hr);
            return false;
        }

        RefPtr<IMF2DBuffer> buffer2D;
        hr = buffer->QueryInterface(IID_PPV_ARGS(buffer2D.GetAddress()));
        if (FAILED(hr))
        {
            WLOGA("Warning : IMFMediaBuffer::QueryInterface() failed. errcode = 0x%x", hr);
            return false;
        }

        // メモリマッピング.
        BYTE* pSrcPtr  = nullptr;
        LONG  srcPitch = 0;
        hr = buffer2D->Lock2D(&pSrcPtr, &srcPitch);
        if (FAILED(hr))
        {
            WLOGA("Warning : IMFMediaBuffer::Lock() failed. errcode = 0x%x", hr);
            return false;
        }

        auto pDevice = asdx::GetD3D12Device();
        auto resDesc = pDstResource->GetDesc();

        D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout = {};
        UINT rows = 0;
        UINT64 rowSize = 0;
        UINT64 totalSize = 0;
        pDevice->GetCopyableFootprints(&resDesc, 0, 1, 0, &layout, &rows, &rowSize, &totalSize);

        if (resDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            void* dstBuffer = nullptr;
            hr = pDstResource->Map(0, nullptr, &dstBuffer);
            if (FAILED(hr))
            {
                buffer2D->Unlock2D();
                WLOGA("Warning : ID3D12Resource::Map() Failed.");
                return false;
            }

            CopyNV12(dstBuffer, layout.Footprint.RowPitch, pSrcPtr, srcPitch, m_Width, m_Height);

            pDstResource->Unmap(0, nullptr);
        }
        else if (resDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            auto srcY  = pSrcPtr;
            hr = pDstResource->WriteToSubresource(
                0,
                nullptr,
                srcY,
                srcPitch,
                srcPitch * m_Height);
            if (FAILED(hr))
            {
                buffer2D->Unlock2D();
                WLOGA("Warning : ID3D12Resource::WriteToSubResource() Failed. errcode = 0x%x", hr);
                return false;
            }

            // アライメントさせる.
            auto alignH = (m_Height + 15) & ~15;
            auto srcUV  = pSrcPtr + srcPitch * alignH;
            hr = pDstResource->WriteToSubresource(
                1,
                nullptr,
                srcUV,
                srcPitch,
                srcPitch * (m_Height / 2));
            if (FAILED(hr))
            {
                buffer2D->Unlock2D();
                WLOGA("Warning : ID3D12resource::WriteToSubresource() Failed. errcode = 0x%x", hr);
                return false;
            }
        }
        else
        {
            // ここには来ないはず.
            assert(false);
        }

        // メモリマッピング解除.
        hr = buffer2D->Unlock2D();
        if (FAILED(hr))
        {
            WLOGA("Warning : IMFMediaBuffer::Unlock() failed. errcode = 0x%x", hr);
            return false;
        }

        // 不要になったサンプルを解放.
        m_Sample.Reset();

        // 正常終了.
        return true;
    }

    //-------------------------------------------------------------------------
    //      横幅を取得します.
    //-------------------------------------------------------------------------
    uint32_t GetWidth() const override
    { return m_Width; }

    //-------------------------------------------------------------------------
    //      縦幅を取得します.
    //-------------------------------------------------------------------------
    uint32_t GetHeight() const override
    { return m_Height; }

    //-------------------------------------------------------------------------
    //      フォーマットを取得します.
    //-------------------------------------------------------------------------
    DXGI_FORMAT GetFormat() const override
    { return m_Format; }

    //-------------------------------------------------------------------------
    //      フレームレートを取得します.
    //-------------------------------------------------------------------------
    double GetFrameRate() const override
    { return m_FrameRate; }

    //-------------------------------------------------------------------------
    //      総フレーム時間を取得します.
    //-------------------------------------------------------------------------
    double GetDurationSec() const override
    {
        // 計算中は正しい値が返せないため、0を返す.
        if (m_CalcFrameCount)
        { return 0.0; }

        return m_DurationSec;
    }

    //-------------------------------------------------------------------------
    //      総フレーム数を取得します.
    //-------------------------------------------------------------------------
    uint64_t GetFrameCount() const override
    {
        // 計算中は正しい値がかえせないため、0を返す.
        if (m_CalcFrameCount)
        { return 0; }

        return m_FrameCount;
    }

    //-------------------------------------------------------------------------
    //      現在のフレーム番号を取得します.
    //-------------------------------------------------------------------------
    uint64_t GetFrameIndex() const override
    { return m_FrameIndex; }

    //-------------------------------------------------------------------------
    //      一時停止を設定します.
    //-------------------------------------------------------------------------
    void SetPause(bool pause) override
    { m_Pause = pause; }

    //-------------------------------------------------------------------------
    //      ループ再生を設定します.
    //-------------------------------------------------------------------------
    void SetLoop(bool loop) override
    { m_Loop = loop; }

    //-------------------------------------------------------------------------
    //      一時停止中かどうかチェックします.
    //-------------------------------------------------------------------------
    bool IsPause() const override
    { return m_Pause; }

    //-------------------------------------------------------------------------
    //      ループ再生中かどうかチェックします.
    //-------------------------------------------------------------------------
    bool IsLoop() const override
    { return m_Loop; }

    //-------------------------------------------------------------------------
    //      再生が完了したかどうかチェックします.
    //-------------------------------------------------------------------------
    bool IsFinished() const override
    { return m_Finished; }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::atomic<uint32_t>   m_RefCount          = {};                   //!< 参照カウント.
    RefPtr<IMFSourceReader> m_Reader            = {};                   //!< ソースリーダー.
    RefPtr<IMFSample>       m_Sample            = {};                   //!< サンプル.
    uint32_t                m_Width             = 0;                    //!< 横幅.
    uint32_t                m_Height            = 0;                    //!< 縦幅.
    DXGI_FORMAT             m_Format            = DXGI_FORMAT_UNKNOWN;  //!< フォーマット.
    double                  m_FrameRate         = 0;                    //!< 1秒あたりのフレーム数.
    double                  m_DurationSec       = 0;                    //!< 総フレーム時間(秒単位).
    uint64_t                m_FrameCount        = 0;                    //!< 総フレーム数.
    uint64_t                m_FrameIndex        = 0;                    //!< フレーム番号.
    LONGLONG                m_LastTimeStamp     = 0;                    //!< 最後にReadSample()で取得したタイムスタンプ.
    bool                    m_Pause             = false;                //!< 一時停止フラグ.
    bool                    m_Loop              = false;                //!< ループ再生フラグ.
    bool                    m_Finished          = false;                //!< 再生完了フラグ.
    bool                    m_CalcFrameCount    = false;                //!< フレーム数を計算フラグ.
    double                  m_PlayTimeSec       = 0.0;                  //!< 再生時間.

    //=========================================================================
    // private variables.
    //=========================================================================

    //-------------------------------------------------------------------------
    //      コンストラクタです.
    //-------------------------------------------------------------------------
    MovieStream()
    : m_RefCount(1)
    , m_Reader  (nullptr)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //      デストラクタです.
    //-------------------------------------------------------------------------
    ~MovieStream()
    { Close(); }

    //-------------------------------------------------------------------------
    //      ビデオデコーダー設定を行います.
    //-------------------------------------------------------------------------
    bool ConfigureVideoDecoder(GUID format)
    {
        RefPtr<IMFMediaType> mediaType;
        auto hr = MFCreateMediaType(mediaType.GetAddress());
        if (FAILED(hr))
        {
            ELOGA("Error : MFCreateMediaType() Failed. errcode = 0x%x", hr);
            return false;
        }

        hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        if (FAILED(hr))
        {
            ELOGA("Error : IMFMediaType::SetGUID() Failed. errcode = 0x%x", hr);
            return false;
        }

        hr = mediaType->SetGUID(MF_MT_SUBTYPE, format);
        if (FAILED(hr))
        {
            ELOGA("Error : IMFMediaType::SetGUID() Failed. errcode = 0x%x", hr);
            return false;
        }

        hr = m_Reader->SetCurrentMediaType(DWORD(MF_SOURCE_READER_FIRST_VIDEO_STREAM), NULL, mediaType.GetPtr());
        if (FAILED(hr))
        {
            ELOGA("Error : IMFSourceReader::SetCurrentMediaType() Failed. errcode = 0x%x", hr);
            return false;
        }

        return true;
    }

    //-------------------------------------------------------------------------
    //      ビデオ情報を取得します.
    //-------------------------------------------------------------------------
    bool GetVideoInfo()
    {
        RefPtr<IMFMediaType> mediaType;
        auto hr = m_Reader->GetCurrentMediaType(DWORD(MF_SOURCE_READER_FIRST_VIDEO_STREAM), mediaType.GetAddress());
        if (FAILED(hr))
        {
            ELOGA("Error : IMFSourceReader::GetCurrentMediaType() Failed. errcode = 0x%x", hr);
            return false;
        }

        uint32_t w, h;
        hr = MFGetAttributeSize(mediaType.GetPtr(), MF_MT_FRAME_SIZE, &w, &h);
        if (FAILED(hr))
        {
            ELOGA("Error : MFGetAttributeSize() Failed. errcode = 0x%x", hr);
            return false;
        }

        uint32_t numerator, denominator;
        hr = MFGetAttributeRatio(mediaType.GetPtr(), MF_MT_FRAME_RATE, &numerator, &denominator);
        if (FAILED(hr))
        {
            ELOGA("Error : MFGetAttributeRatio() Failed. errcode = 0x%x", hr);
            return false;
        }

        double fps = double(numerator) / double(denominator);

        PROPVARIANT prop = {};
        PropVariantInit(&prop);

        hr = m_Reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &prop);
        if (SUCCEEDED(hr))
        {
            LONGLONG duration = {};
            PropVariantToInt64(prop, &duration);
            m_DurationSec = double(duration) / 10000000.0; // 100ns単位を秒単位に変換.
            m_FrameCount  = uint64_t(ceil(m_DurationSec * fps));
        }
        else
        {
            // へぼい動画の場合はデータがないので、一回デコードして数える他ないので計算フラグを立てる.
            m_CalcFrameCount = true;
        }
        PropVariantClear(&prop);

        m_Width      = w;
        m_Height     = h;
        m_FrameRate  = fps;

        return true;
    }
};


///////////////////////////////////////////////////////////////////////////////
// MovieTexture class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
MovieTexture::MovieTexture()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
MovieTexture::~MovieTexture()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool MovieTexture::Init(const char* path)
{
    if (!CreateMovieStream(m_Stream.GetAddress()))
    {
        ELOGA("Error : CreateMovieStream() Failed.");
        return false;
    }

    if (!m_Stream->Open(path))
    {
        ELOGA("Error : IMovieStream::Open() Failed. path = %s", path);
        return false;
    }

    auto pDevice = asdx::GetD3D12Device();

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Width              = m_Stream->GetWidth();
    desc.Height             = m_Stream->GetHeight();
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.Format             = m_Stream->GetFormat();
    desc.SampleDesc         = { 1, 0 };
    desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    auto heapType = D3D12_HEAP_TYPE_UPLOAD;
    if (asdx::IsSupportGpuUploadHeap())
        heapType = D3D12_HEAP_TYPE_GPU_UPLOAD;

    D3D12_HEAP_PROPERTIES props = {};
    props.Type                  = heapType;
    props.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
    props.CreationNodeMask      = 1;
    props.VisibleNodeMask       = 1;

    for(auto i=0; i<2; ++i)
    {
        auto hr = pDevice->CreateCommittedResource(
            &props,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(m_Texture[i].GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
            return false;
        }

        D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
        viewDesc.Format                     = DXGI_FORMAT_R8_UNORM;
        viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_TEXTURE2D;
        viewDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        viewDesc.Texture2D.MipLevels        = 1;
        viewDesc.Texture2D.MostDetailedMip  = 0;
        viewDesc.Texture2D.PlaneSlice       = 0;

        m_HandleSRV[i] = asdx::DescriptorHolder(DescriptorHolder::HEAP_RES, GetResourceDescriptorHeap()->Alloc(2));
        if (!m_HandleSRV[i].IsValid())
        {
            ELOGA("Error : Descriptor Allocate Failed.");
            return false;
        }

        auto handleSRV0 = m_HandleSRV[i].GetHandleCPU(0);
        pDevice->CreateShaderResourceView(m_Texture[i].GetPtr(), &viewDesc, handleSRV0);

        // Plane 1にして，フォーマットを変更.
        viewDesc.Format               = DXGI_FORMAT_R8G8_UNORM;
        viewDesc.Texture2D.PlaneSlice = 1;

        auto handleSRV1 = m_HandleSRV[i].GetHandleCPU(1); 
        pDevice->CreateShaderResourceView(m_Texture[i].GetPtr(), &viewDesc, handleSRV1);
    }

    m_CurrIndex = 0;
    m_PrevIndex = 1;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void MovieTexture::Term()
{
    if (!!m_Stream)
    { m_Stream.Reset(); }

    for(auto i=0; i<2; ++i)
    {
        m_HandleSRV[i].Reset();
        m_Texture  [i].Reset();
    }

    m_CurrIndex = 0;
    m_PrevIndex = 0;
}

//-----------------------------------------------------------------------------
//      更新処理を行います.
//-----------------------------------------------------------------------------
bool MovieTexture::Update(double elapsedTimeSec)
{
    auto decoded = false;
    auto nextId  = (m_CurrIndex + 1) & 0x1;
    m_PrevIndex  = m_CurrIndex;
    if (m_Stream->Read(m_Texture[nextId].GetPtr(), elapsedTimeSec))
    {
        m_CurrIndex = nextId;
        decoded     = true;
    }

    return decoded;
}

//-----------------------------------------------------------------------------
//      CPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE MovieTexture::GetHandleCPU(uint32_t offset) const
{ return m_HandleSRV[m_PrevIndex].GetHandleCPU(offset); }

//-----------------------------------------------------------------------------
//      GPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE MovieTexture::GetHandleGPU(uint32_t offset) const
{ return m_HandleSRV[m_PrevIndex].GetHandleGPU(offset); }

//-----------------------------------------------------------------------------
//      フレームレートを取得します.
//-----------------------------------------------------------------------------
double MovieTexture::GetFrameRate() const
{
    if (!m_Stream)
        return 0.0;

    return m_Stream->GetFrameRate();
}

//-----------------------------------------------------------------------------
//      動画の長さを取得します.
//-----------------------------------------------------------------------------
double MovieTexture::GetDurationSec() const
{
    if (!m_Stream)
        return 0.0;

    return m_Stream->GetDurationSec();
}

//-----------------------------------------------------------------------------
//      総フレーム数を取得します.
//-----------------------------------------------------------------------------
uint64_t MovieTexture::GetFrameCount() const
{
    if (!m_Stream)
        return 0;

    return m_Stream->GetFrameCount();
}

//-----------------------------------------------------------------------------
//      現在のフレーム番号を取得します.
//-----------------------------------------------------------------------------
uint64_t MovieTexture::GetFrameIndex() const
{
    if (!m_Stream)
        return 0;

    return m_Stream->GetFrameIndex();
}

//-----------------------------------------------------------------------------
//      一時停止フラグを設定します.
//-----------------------------------------------------------------------------
void MovieTexture::SetPause(bool pause)
{
    if (!m_Stream)
        return;

    m_Stream->SetPause(pause);
}

//-----------------------------------------------------------------------------
//      ループ再生フラグを設定します.
//-----------------------------------------------------------------------------
void MovieTexture::SetLoop(bool loop)
{
    if (!m_Stream)
        return;

    m_Stream->SetLoop(loop);
}

//-----------------------------------------------------------------------------
//      一時停止フラグを取得します.
//-----------------------------------------------------------------------------
bool MovieTexture::IsPause() const
{
    if (!m_Stream)
        return false;

    return m_Stream->IsPause();
}

//-----------------------------------------------------------------------------
//      ループ再生フラグを取得します.
//-----------------------------------------------------------------------------
bool MovieTexture::IsLoop() const
{
    if (!m_Stream)
        return false;

    return m_Stream->IsLoop();
}

//-----------------------------------------------------------------------------
//      再生完了フラグを取得します.
//-----------------------------------------------------------------------------
bool MovieTexture::IsFinished() const
{
    if (!m_Stream)
        return true;

    return m_Stream->IsFinished();
}


///////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      ムービーストリームを作成します.
//-----------------------------------------------------------------------------
bool CreateMovieStream(IMovieStream** ppResult)
{ return MovieStream::Create(ppResult); }

//-----------------------------------------------------------------------------
//      NV12形式のコピー処理を行います.
//-----------------------------------------------------------------------------
void CopyNV12
(
    void*       dstPtr,
    uint32_t    dstPitch,
    const void* srcPtr,
    uint32_t    srcPitch,
    uint32_t    width,
    uint32_t    height
)
{
    // アライメントを取る.
    auto alignH = (height + 15) & ~15;

    auto dstY  = reinterpret_cast<uint8_t*>(dstPtr);
    auto dstUV = dstY + dstPitch * alignH;

    auto srcY  = reinterpret_cast<const uint8_t*>(srcPtr);
    auto srcUV = srcY + srcPitch * alignH;

    // Y Plane.
    for (uint32_t y = 0; y < height; ++y)
    {
        std::memcpy(
            dstY + y * dstPitch,
            srcY + y * srcPitch,
            width);
    }
    // UV Plane.
    for (uint32_t y = 0; y < height / 2; ++y)
    {
        std::memcpy(
            dstUV + y * dstPitch,
            srcUV + y * srcPitch,
            width);
    }
}

} // namespace asdx
