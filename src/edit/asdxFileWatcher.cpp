//-----------------------------------------------------------------------------
// File : asdxFileWatcher.cpp
// Desc : File Wacher.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <vector>
#include <string>
#include <Windows.h>
#include <edit/asdxFileWatcher.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMisc.h>


namespace {

//-----------------------------------------------------------------------------
//      フルパスに変換します.
//-----------------------------------------------------------------------------
std::string ToFullPathA(const char* path)
{
    char fullPath[512] = {};

    GetFullPathNameA(path, 512, fullPath, nullptr);
    return std::string(fullPath);
}
} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Worker structure
///////////////////////////////////////////////////////////////////////////////
struct FileWatcher::Worker
{
    HANDLE                  hEvent          = nullptr;
    HANDLE                  hDir            = nullptr;
    std::vector<uint8_t>    Buffer          = {};
    std::string             DirectoryPath   = {};
    OVERLAPPED*             pOverlapped     = nullptr;

    Worker()
    { /* DO_NOTHING */ }

    ~Worker()
    { /* DO_NOTHING */ }

    bool Prepare(const std::string& dirPath, size_t bufferSize)
    {
        hDir = CreateFileA(
            dirPath.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            nullptr
        );
        if (hDir == INVALID_HANDLE_VALUE)
        {
            ELOGA("Error : CreateFileA() Failed. path = %s, errcode = 0x%x", dirPath.c_str(), GetLastError());
            hDir = nullptr;
            return false;
        }

        hEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);
        if (hEvent == nullptr)
        {
            ELOGA("Error : CreateEventA() Failed.");
            CloseHandle(hDir);
            hDir = nullptr;
            return false;
        }

        DirectoryPath   = dirPath;
        Buffer.resize(bufferSize);

        return true;
    }

    void BeginWatch()
    {
        ResetEvent(hEvent);

        DWORD filter =
            FILE_NOTIFY_CHANGE_FILE_NAME  |   // ファイル名の変更.
            FILE_NOTIFY_CHANGE_DIR_NAME   |   // ディレクトリ名の変更.
            FILE_NOTIFY_CHANGE_ATTRIBUTES |   // 属性の変更.
            FILE_NOTIFY_CHANGE_SIZE       |   // サイズの変更.
            FILE_NOTIFY_CHANGE_LAST_WRITE;    // 最終書き込み日時の変更.

        auto pOverlapped = new OVERLAPPED();
        ZeroMemory(pOverlapped, sizeof(OVERLAPPED));
        pOverlapped->hEvent = hEvent;

        ReadDirectoryChangesW(
            hDir,
            Buffer.data(),
            DWORD(Buffer.size()),
            TRUE,
            filter,
            nullptr,
            pOverlapped,
            nullptr
        );
    }

    void Process(std::vector<asdx::FileEventArgs>& outEvents)
    {
        DWORD bytes = 0;

        if (!GetOverlappedResult(hDir, nullptr, &bytes, FALSE))
            return;

        if (bytes == 0)
            return;

        auto pInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(Buffer.data());

        for(;;)
        {
            std::wstring ws(pInfo->FileName, pInfo->FileNameLength / sizeof(wchar_t));

            auto fileName = asdx::ToStringA(ws.c_str());
            auto fullPath = DirectoryPath + "/" + fileName;

            asdx::FileEventArgs args;
            args.Type     = asdx::FileEventArgs::TYPE(pInfo->Action);
            args.FullPath = ToFullPathA(fullPath.c_str());

            outEvents.push_back(args);

            if (pInfo->NextEntryOffset == 0)
                break;

            pInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                reinterpret_cast<uint8_t*>(pInfo) + pInfo->NextEntryOffset);
        }

        BeginWatch();
    }

    void Release()
    {
        if (hDir)
        {
            CloseHandle(hDir);
            hDir = nullptr;
        }

        if (hEvent)
        {
            CloseHandle(hEvent);
            hEvent = nullptr;
        }

        Buffer.clear();
        Buffer.shrink_to_fit();

        if (pOverlapped != nullptr)
        {
            delete pOverlapped;
            pOverlapped = nullptr;
        }
    }
};


///////////////////////////////////////////////////////////////////////////////
// FileWatcher class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
FileWatcher::FileWatcher()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
FileWatcher::~FileWatcher()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool FileWatcher::Init(Desc& desc)
{
    if (desc.pListeners.empty() || desc.Dirs.empty())
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    // 念のために終了させる.
    Term();

    // 終了フラグを下す.
    m_Finish = false;

    m_DirCount      = desc.Dirs.size();
    m_WaitTimeMsec  = desc.WaitTimeMsec; 

    m_Listeners.assign(desc.pListeners.begin(), desc.pListeners.end());

    m_pWorkers = new(std::nothrow) Worker[m_DirCount];
    if (m_pWorkers == nullptr)
    {
        ELOG("Error : Out of Memory.");
        return false;
    }

    for(size_t i=0; i<m_DirCount; ++i)
    {
        if (!m_pWorkers[i].Prepare(desc.Dirs[i], desc.BufferSize))
        {
            ELOG("Error : Worker::Prepare() Failed.");
            return false;
        }

        m_Events.push_back(m_pWorkers[i].hEvent);
    }

    // 監視スレッド起動.
    m_pThread = new std::thread([this]()
    {
        std::vector<asdx::FileEventArgs> events;

        while(!m_Finish)
        {
            DWORD ret = WaitForMultipleObjects(
                DWORD(m_Events.size()),
                m_Events.data(),
                FALSE,
                m_WaitTimeMsec);

            if (ret >= WAIT_OBJECT_0 && ret < WAIT_OBJECT_0 + m_DirCount)
            {
                size_t index = ret - WAIT_OBJECT_0;
                events.clear();

                m_pWorkers[index].Process(events);
                for(auto& listener : m_Listeners)
                {
                    listener->OnChanged(events);
                }
            }
        }
    });
    if (m_pThread == nullptr)
    { return false; }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void FileWatcher::Term()
{
    if (m_pThread == nullptr)
    { return; }

    // 終了フラグを立てる.
    m_Finish = true;

    // join可能になるまで待つ.
    while (!m_pThread->joinable())
    { std::this_thread::sleep_for(std::chrono::milliseconds(1)); }

    // joinする
    if (m_pThread->joinable())
    { m_pThread->join(); }

    // スレッド破棄.
    delete m_pThread;
    m_pThread = nullptr;

    for(size_t i=0; i<m_DirCount; ++i)
    {
        m_pWorkers[i].Release();
    }

    delete [] m_pWorkers;
    m_pWorkers = nullptr;

    m_Events.clear();

    m_DirCount     = 0;
    m_WaitTimeMsec = 0;
}

} // namespace asdx
