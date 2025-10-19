//-----------------------------------------------------------------------------
// File : asdxBlink.cpp
// Desc : blink Utility.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#if _MSC_VER

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <string>
#include <Windows.h>
#include <processthreadsapi.h>
#include <fnd/asdxLogger.h>


namespace {
//-----------------------------------------------------------------------------
// Global Variables.
//-----------------------------------------------------------------------------
DWORD   g_ProcessId = 0;
HANDLE  g_hProcess  = nullptr;
HANDLE  g_hThread   = nullptr;
} // namespace


namespace asdx {

//-----------------------------------------------------------------------------
//      blink.exe を起動します.
//-----------------------------------------------------------------------------
bool InitBlink(const char* path)
{
    if (g_hProcess != nullptr && g_hThread != nullptr)
        return true;

    g_ProcessId = GetCurrentProcessId();

    // blinkを立ち上げる.
    {
        STARTUPINFOA startupInfo = {};
        PROCESS_INFORMATION processInfo = {};

        DWORD flag = NORMAL_PRIORITY_CLASS;
        startupInfo.cb = sizeof(STARTUPINFOA);

        // "blink.exe PID"
        std::string cmd = path;
        cmd += " " + std::to_string(g_ProcessId);

        auto ret = CreateProcessA(
            nullptr,
            const_cast<char*>(cmd.c_str()),
            nullptr,
            nullptr,
            FALSE,
            flag,
            nullptr,
            nullptr,
            &startupInfo,
            &processInfo);

        if (ret == FALSE)
        {
            CloseHandle(processInfo.hThread);
            CloseHandle(processInfo.hProcess);
            return false;
        }

        g_hProcess = processInfo.hProcess;
        g_hThread  = processInfo.hThread;
        ILOG("blink launch sucesss. cmd = %s", cmd.c_str());
    }

    return true;
}

//-----------------------------------------------------------------------------
//      blink.exe を終了させます.
//-----------------------------------------------------------------------------
void TermBlink()
{
    if (g_hThread != nullptr)
    {
        CloseHandle(g_hThread);
        g_hThread = nullptr;
    }

    if (g_hProcess != nullptr)
    {
        CloseHandle(g_hProcess);
        g_hProcess = nullptr;
    }

    g_ProcessId = 0;
}

} // namespace asdx

#endif// _MSC_VER