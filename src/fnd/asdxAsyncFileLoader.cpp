//-----------------------------------------------------------------------------
// File : asdxAsyncFileLoader.cpp
// Desc : Async File Loader.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxAsyncFileLoader.h>
#include <thread>
#include <mutex>
#include <string>
#include <queue>
#include <atomic>
#include <unordered_map>
#include <condition_variable>


namespace {

///////////////////////////////////////////////////////////////////////////////
// AsyncFileLoader class
///////////////////////////////////////////////////////////////////////////////
class AsyncFileLoader : public asdx::IAsyncFileLoader
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
    AsyncFileLoader(size_t threadCount) 
    : m_Running     (true)
    , m_NextHandle  (1)
    {
        for(size_t i=0; i<threadCount; ++i)
        {  m_Workers.emplace_back([this]() { this->OnExecute(); }); }
    }

    //-------------------------------------------------------------------------
    //      デストラクタです.
    //-------------------------------------------------------------------------
    ~AsyncFileLoader()
    {
        {
            std::lock_guard<std::mutex> locker(m_Mutex);
            m_Running = false;
        }
        m_Cond.notify_all();
        for(auto& t : m_Workers)
        {
            if (t.joinable())
            { t.join(); }
        }
    }

    //-------------------------------------------------------------------------
    //      ロードリクエストをします.
    //-------------------------------------------------------------------------
    uint64_t RequestLoad(const char* path) override
    {
        auto handle = m_NextHandle++;
        auto req = std::make_shared<Request>();
        req->Path = path;

        {
            std::lock_guard<std::mutex> locker(m_Mutex);
            m_Requests[handle] = req;
            m_RequestQueue.push(handle);
        }

        m_Cond.notify_one();
        return handle;
    }

    //-------------------------------------------------------------------------
    //      ロードが完了したかどうかチェックします.
    //-------------------------------------------------------------------------
    bool IsFinished(uint64_t handle) override
    {
        std::lock_guard<std::mutex> locker(m_Mutex);
        auto itr = m_Requests.find(handle);
        return (itr != m_Requests.end()) && (itr->second->Done.load()); 
    }

    //-------------------------------------------------------------------------
    //      ロードが完了したかどうかチェックします.
    //-------------------------------------------------------------------------
    bool IsAllFinished() override
    {
        std::lock_guard<std::mutex> locker(m_Mutex);
        for(auto& itr : m_Requests)
        {
            if (!itr.second->Done.load())
                return false;
        }

        return true;
    }

    //-------------------------------------------------------------------------
    //      ロード結果を取得します.
    //-------------------------------------------------------------------------
    std::vector<uint8_t> GetResult(uint64_t handle) override
    {
        std::lock_guard<std::mutex> locker(m_Mutex);
        auto itr = m_Requests.find(handle);
        if (itr == m_Requests.end() || !itr->second->Done.load())
            return {};

        std::vector<uint8_t> blob;
        blob.swap(itr->second->Blob);
        m_Requests.erase(itr);
        return blob;
    }

    //-------------------------------------------------------------------------
    //      リクエストキューが空であるかチェックします.
    //-------------------------------------------------------------------------
    bool IsRequestEmpty() override
    {
        std::lock_guard<std::mutex> locker(m_Mutex);
        return m_Requests.empty();
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
        std::string             Path;
        std::vector<uint8_t>    Blob;
        std::atomic<bool>       Done = false;
    };

    //=========================================================================
    // private variables.
    //=========================================================================
    std::atomic<bool>           m_Running = {};
    std::vector<std::thread>    m_Workers;
    std::mutex                  m_Mutex;
    std::queue<uint64_t>        m_RequestQueue;
    std::atomic<uint64_t>       m_NextHandle;
    std::condition_variable     m_Cond;
    std::unordered_map<uint64_t, std::shared_ptr<Request>> m_Requests;

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
            uint64_t handle = 0;
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
                auto err = fopen_s(&fp, req->Path.c_str(), "rb");
                if (err == 0 && fp != nullptr)
                {
                    fseek(fp, 0, SEEK_END);
                    auto size = ftell(fp);
                    rewind(fp);

                    req->Blob.resize(size);
                    fread(req->Blob.data(), size, 1, fp);
                    fclose(fp);
                }
                req->Done = true;
            }
        }
    }
};

} // namespace

namespace asdx {

//-----------------------------------------------------------------------------
//      非同期ファイルローダーを生成します.
//-----------------------------------------------------------------------------
bool CreateAsyncFileLoader(IAsyncFileLoader** ppLoader)
{
    auto instance = new (std::nothrow) AsyncFileLoader(1);
    if (instance == nullptr)
        return false;

    (*ppLoader) = instance;
    return true;
}

} // namespace asdx
