//-----------------------------------------------------------------------------
// File : asdxAsyncFileIO.cpp
// Desc : Async File I/O.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxAsyncFileIO.h>
#include <cassert>
#include <thread>
#include <mutex>
#include <string>
#include <queue>
#include <atomic>
#include <unordered_map>
#include <condition_variable>


namespace {

///////////////////////////////////////////////////////////////////////////////
// AsyncFileIO class
///////////////////////////////////////////////////////////////////////////////
class AsyncFileIO
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
    //      引数付きコンストラクタです.
    //-------------------------------------------------------------------------
    AsyncFileIO()
    : m_Running     (true)
    , m_NextHandle  (1)
    {
        m_Worker = std::thread([this]() { this->OnExecute(); });
    }

    //-------------------------------------------------------------------------
    //      デストラクタです.
    //-------------------------------------------------------------------------
    ~AsyncFileIO()
    {
        {
            std::lock_guard<std::mutex> locker(m_Mutex);
            m_Running = false;
        }
        m_Cond.notify_all();
        if (m_Worker.joinable())
        { m_Worker.join(); }
    }

    //-------------------------------------------------------------------------
    //      ロードリクエストをします.
    //-------------------------------------------------------------------------
    uint32_t RequestLoad(const char* path)
    {
        auto handle = m_NextHandle++;
        auto req = std::make_shared<Request>();
        req->Path = path;
        req->Save = false;

        {
            std::lock_guard<std::mutex> locker(m_Mutex);
            m_Requests[handle] = req;
            m_RequestQueue.push(handle);
        }

        m_Cond.notify_one();
        return handle;
    }

    //-------------------------------------------------------------------------
    //      セーブリクエストをします.
    //-------------------------------------------------------------------------
    uint32_t RequestSave(const char* path, std::vector<uint8_t>& blob)
    {
        auto handle = m_NextHandle++;
        auto req = std::make_shared<Request>();
        req->Path = path;
        req->Save = true;
        req->Blob = blob;

        {
            std::lock_guard<std::mutex> locker(m_Mutex);
            m_Requests[handle] = req;
            m_RequestQueue.push(handle);
        }

        m_Cond.notify_one();
        return handle;
    }

    //-------------------------------------------------------------------------
    //      リクエストが完了したかどうかチェックします.
    //-------------------------------------------------------------------------
    bool IsFinished(uint32_t handle)
    {
        std::lock_guard<std::mutex> locker(m_Mutex);
        auto itr = m_Requests.find(handle);
        return (itr != m_Requests.end()) && (itr->second->Done.load());
    }

    //-------------------------------------------------------------------------
    //      読み込み結果を取得します.
    //-------------------------------------------------------------------------
    std::vector<uint8_t> GetLoadResult(uint32_t handle)
    {
        std::lock_guard<std::mutex> locker(m_Mutex);
        auto itr = m_Requests.find(handle);
        if (itr == m_Requests.end() || !itr->second->Done.load() || itr->second->Save)
            return {};

        std::vector<uint8_t> blob;
        blob.swap(itr->second->Blob);
        m_Requests.erase(itr);
        return blob;
    }

    //-------------------------------------------------------------------------
    //      書き込み結果を取得します.
    //-------------------------------------------------------------------------
    bool GetSaveResult(uint32_t handle)
    {
        std::lock_guard<std::mutex> locker(m_Mutex);
        auto itr = m_Requests.find(handle);
        if (itr == m_Requests.end() || !itr->second->Done.load() || !itr->second->Save)
            return {};

        bool result = itr->second->Success.load();
        m_Requests.erase(itr);
        return result;
    }

    //-------------------------------------------------------------------------
    //      解放処理を行います.
    //-------------------------------------------------------------------------
    void Release()
    { delete this; }

private:
    ///////////////////////////////////////////////////////////////////////////
    // Request structure
    ///////////////////////////////////////////////////////////////////////////
    struct Request
    {
        std::string             Path;               // ファイルパス.
        std::vector<uint8_t>    Blob;               // バイナリデータ.
        std::atomic<bool>       Done    = false;    // 処理したかどうか.
        std::atomic<bool>       Success = false;    // 成功したかどうか.
        bool                    Save    = false;    // 書き込みなら true, 読み込みなら false.
    };

    //=========================================================================
    // private variables.
    //=========================================================================
    std::atomic<bool>           m_Running = {};
    std::thread                 m_Worker;
    std::mutex                  m_Mutex;
    std::queue<uint32_t>        m_RequestQueue;
    std::atomic<uint32_t>       m_NextHandle;
    std::condition_variable     m_Cond;
    std::unordered_map<uint32_t, std::shared_ptr<Request>> m_Requests;

    //=========================================================================
    // private methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //      スレッド関数です.
    //-------------------------------------------------------------------------
    void OnExecute()
    {
        while(m_Running)
        {
            uint32_t handle = 0;
            std::shared_ptr<Request> req;

            {
                std::unique_lock<std::mutex> locker(m_Mutex);
                m_Cond.wait(locker, [this](){ return !m_RequestQueue.empty() || !m_Running; });
                if (!m_Running && m_RequestQueue.empty())
                    return;

                handle = m_RequestQueue.front();
                m_RequestQueue.pop();

                auto itr = m_Requests.find(handle);
                if (itr != m_Requests.end())
                    req = itr->second;
            }

            if (req)
            {
                FILE* fp = nullptr;

                // 書き込み.
                if (req->Save)
                {
                    auto err = fopen_s(&fp, req->Path.c_str(), "wb");
                    if (err == 0 && fp != nullptr)
                    {
                        fwrite(req->Blob.data(), req->Blob.size(), 1, fp);
                        fclose(fp);
                        req->Success = true;
                    }
                    else
                    {
                        req->Success = false;
                    }
                }
                // 読み込み.
                else
                {
                    auto err = fopen_s(&fp, req->Path.c_str(), "rb");
                    if (err == 0 && fp != nullptr)
                    {
                        fseek(fp, 0, SEEK_END);
                        auto size = ftell(fp);
                        rewind(fp);

                        req->Blob.resize(size);
                        fread(req->Blob.data(), size, 1, fp);
                        fclose(fp);

                        req->Success = true;
                    }
                    else
                    {
                        req->Success = false;
                    }
                }
                req->Done = true;
            }
        }
    }
};
static AsyncFileIO* s_pAsyncFileIO = nullptr;

} // namespace

namespace asdx {

//-----------------------------------------------------------------------------
//      非同期ファイルIOの初期化処理.
//-----------------------------------------------------------------------------
bool InitAsyncFileIO()
{
    if (s_pAsyncFileIO != nullptr)
        return true;

    s_pAsyncFileIO = new AsyncFileIO();
    return true;
}

//-----------------------------------------------------------------------------
//      非同期ファイルIOの終了処理.
//-----------------------------------------------------------------------------
void TermAsyncFileIO()
{
    if (s_pAsyncFileIO != nullptr)
    {
        delete s_pAsyncFileIO;
        s_pAsyncFileIO = nullptr;
    }
}

//-----------------------------------------------------------------------------
//      非同期ファイルIOが初期化済みか確認します.
//-----------------------------------------------------------------------------
bool IsInitAsyncFileIO()
{ return s_pAsyncFileIO != nullptr; }

//-----------------------------------------------------------------------------
//      非同期読み込みリクエストを行います.
//-----------------------------------------------------------------------------
RequestId RequestLoad(const char* path)
{
    assert(s_pAsyncFileIO != nullptr);
    if (s_pAsyncFileIO == nullptr)
        return kInvalidRequestId;

    return s_pAsyncFileIO->RequestLoad(path);
}

//-----------------------------------------------------------------------------
//      非同期書き込みリクエストを行います.
//-----------------------------------------------------------------------------
RequestId RequestSave(const char* path, std::vector<uint8_t>& blob)
{
    assert(s_pAsyncFileIO != nullptr);
    if (s_pAsyncFileIO == nullptr)
        return kInvalidRequestId;

    return s_pAsyncFileIO->RequestSave(path, blob);
}

//-----------------------------------------------------------------------------
//      リクエストが完了したかどうか確認します.
//-----------------------------------------------------------------------------
bool IsFinished(RequestId handle)
{
    assert(s_pAsyncFileIO != nullptr);
    if (s_pAsyncFileIO == nullptr)
        return false;

    return s_pAsyncFileIO->IsFinished(handle);
}

//-----------------------------------------------------------------------------
//      非同期読み込み結果を取得します.
//-----------------------------------------------------------------------------
std::vector<uint8_t> GetLoadResult(RequestId requestId)
{
    assert(s_pAsyncFileIO != nullptr);
    if (s_pAsyncFileIO == nullptr)
        return {};

    return s_pAsyncFileIO->GetLoadResult(requestId);
}

//-----------------------------------------------------------------------------
//      非同期書き込み結果を取得します.
//-----------------------------------------------------------------------------
bool GetSaveResult(RequestId requestId)
{
    assert(s_pAsyncFileIO != nullptr);
    if (s_pAsyncFileIO == nullptr)
        return {};

    return s_pAsyncFileIO->GetSaveResult(requestId);
}

} // namespace asdx
