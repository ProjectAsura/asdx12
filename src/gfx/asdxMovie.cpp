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
#include <array>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <propvarutil.h>
#include <fnd/asdxLogger.h>
#include <gfx/asdxMovie.h>


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
            auto size = MultiByteToWideChar(CP_UTF8, 0, path, int(strlen(path)), NULL, 0);
            wchar_t* widePath = new wchar_t[size + 1];
            MultiByteToWideChar(CP_UTF8, 0, path, int(strlen(path)), widePath, size);

            // ソースコードリーダーを生成.
            hr = MFCreateSourceReaderFromURL(widePath, attributes.GetPtr(), m_Reader.GetAddress());
            delete [] widePath;
            if (FAILED(hr))
            {
                ELOG("Error : MFCreateSourceReaderFromURL() Failed. errcode = 0x%x, path = %s", hr, path);
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

        m_Finished   = false;
        m_FrameIndex = 0;
        m_Format     = DXGI_FORMAT_NV12;

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
        m_Duration          = 0;
        m_FrameCount        = 0;
        m_FrameIndex        = 0;
        m_LastTimeStamp     = 0;
        m_Pause             = false;
        m_Finished          = true;
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

        return true;
    }

    //-------------------------------------------------------------------------
    //      フレームデータを読み込みます.
    //-------------------------------------------------------------------------
    bool Read(MovieFrame& frame) override
    {
        if (m_Reader.GetPtr() == nullptr)
        {
            ELOG("Error : Invalid Operation.");
            return false;
        }

        // 再生完了状態.
        if (m_Finished)
        { return false; }

        DWORD             flags     = 0;
        LONGLONG          timeStamp = {};
        RefPtr<IMFSample> sample;

        // サンプルを読み込み.
        auto hr = m_Reader->ReadSample(DWORD(MF_SOURCE_READER_FIRST_VIDEO_STREAM), 0, nullptr, &flags, &timeStamp, sample.GetAddress());
        if (FAILED(hr))
        {
            WLOGA("Warning : IMFSourceReader::ReadSample() failed. errcode = 0x%x", hr);
            return false;
        }

        // バッファ取得.
        RefPtr<IMFMediaBuffer> buffer;
        hr = sample->GetBufferByIndex(0, buffer.GetAddress());
        if (FAILED(hr))
        {
            WLOGA("Warning : IMFSample::GetBufferByIndex() failed. errcode = 0x%x", hr);
            return false;
        }

        // メモリマッピング.
        BYTE* pSrc    = nullptr;
        DWORD maxSize = 0;
        DWORD curSize = 0;
        hr = buffer->Lock(&pSrc, &maxSize, &curSize);
        if (FAILED(hr))
        {
            WLOGA("Warning : IMFMediaBuffer::Lock() failed. errcode = 0x%x", hr);
            return false;
        }

        // バッファリング.
        uint8_t idx = (m_BufferIndex + 1) % uint8_t(m_FrameBuffer.size());

        // メモリが無ければここで確保.
        if (m_FrameBuffer[idx].MaxSize < maxSize)
        {
            if (m_FrameBuffer[idx].pData != nullptr)
            {
                operator delete (m_FrameBuffer[idx].pData);
                m_FrameBuffer[idx].pData = nullptr;
            }
            m_FrameBuffer[idx].pData   = operator new (maxSize);
            m_FrameBuffer[idx].MaxSize = maxSize;
        }

        // デコードデータをコピー.
        memcpy(m_FrameBuffer[idx].pData, pSrc, curSize);
        m_FrameBuffer[idx].CopySize = curSize;

        // メモリマッピング解除.
        hr = buffer->Unlock();
        if (FAILED(hr))
        {
            WLOGA("Warning : IMFMediaBuffer::Unlock() failed. errcode = 0x%x", hr);
            return false;
        }

        // フレーム情報を更新.
        m_BufferIndex   = idx;
        m_LastTimeStamp = timeStamp;
        m_FrameIndex    = uint64_t(m_FrameRate * timeStamp);

        // デコード結果を設定.
        frame.pData = m_FrameBuffer[idx].pData;
        frame.Size  = m_FrameBuffer[idx].CopySize;

        // 再生完了判定.
        if (m_FrameIndex >= m_FrameCount)
        { m_Finished = true; }

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
    int64_t GetDuration() const override
    { return m_Duration; }

    //-------------------------------------------------------------------------
    //      総フレーム数を取得します.
    //-------------------------------------------------------------------------
    uint64_t GetFrameCount() const override
    { return m_FrameCount; }

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
    ///////////////////////////////////////////////////////////////////////////
    // FrameBuffer structure
    ///////////////////////////////////////////////////////////////////////////
    struct FrameBuffer
    {
        void*  pData    = nullptr;
        size_t MaxSize  = 0;
        size_t CopySize = 0;
    };

    //=========================================================================
    // private variables.
    //=========================================================================
    std::atomic<uint32_t>       m_RefCount      = {};
    RefPtr<IMFSourceReader>     m_Reader        = {};
    uint32_t                    m_Width         = 0;
    uint32_t                    m_Height        = 0;
    DXGI_FORMAT                 m_Format        = DXGI_FORMAT_UNKNOWN;
    double                      m_FrameRate     = 0;
    int64_t                     m_Duration      = 0;
    uint64_t                    m_FrameCount    = 0;
    uint64_t                    m_FrameIndex    = 0;
    int64_t                     m_LastTimeStamp = 0;
    bool                        m_Pause         = false;
    bool                        m_Loop          = false;
    bool                        m_Finished      = false;
    std::array<FrameBuffer, 2>  m_FrameBuffer   = {};
    uint8_t                     m_BufferIndex   = 0;

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
    {
        m_Reader.Reset();
        for(auto i=0; i<m_FrameBuffer.size(); ++i)
        {
            if (m_FrameBuffer[i].pData != nullptr)
            {
                delete[] m_FrameBuffer[i].pData;
                m_FrameBuffer[i].pData    = nullptr;
                m_FrameBuffer[i].CopySize = 0;
                m_FrameBuffer[i].MaxSize  = 0;
            }
        }
    }

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
        hr = m_Reader->GetPresentationAttribute(DWORD(MF_SOURCE_READER_MEDIASOURCE), MF_PD_DURATION, &prop);
        if (FAILED(hr))
        {
            ELOGA("Error : IMFSourceReader::GetPresentationAttribute() Failed. errcode = 0x%x", hr);
            return false;
        }

        LONGLONG duration = {};
        PropVariantToInt64(prop, &duration);
        PropVariantClear(&prop);

        m_Width      = w;
        m_Height     = h;
        m_FrameRate  = fps;
        m_Duration   = duration;
        m_FrameCount = uint64_t(ceil(duration * fps)); // TODO : 100ns単位なので問題ないかどうかチェック.

        return true;
    }
};

//-----------------------------------------------------------------------------
//      ムービーストリームを作成します.
//-----------------------------------------------------------------------------
bool CreateMovieStream(IMovieStream** ppResult)
{ return MovieStream::Create(ppResult); }

} // namespace asdx
