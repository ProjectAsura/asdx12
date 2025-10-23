//-----------------------------------------------------------------------------
// File : asdxPath.cpp
// Desc : File Path.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxPath.h>
#include <Windows.h>
#include <Shlwapi.h>
#include <ShObjIdl.h>   // For Folder Dialog

namespace asdx {

//-----------------------------------------------------------------------------
//      実行ファイルのファイルパスを取得します.
//-----------------------------------------------------------------------------
fs::path GetExePath()
{
    char exePath[512] = {};
    GetModuleFileNameA(nullptr, exePath, 512);
    exePath[511] = '\0';
    PathRemoveFileSpecA(exePath);
    return fs::path(exePath);
}

//-----------------------------------------------------------------------------
//      一時ファイル用のディレクトリパスを取得します.
//-----------------------------------------------------------------------------
fs::path GetTempDir()
{
    char dir[512] = {};
    GetTempPathA(512, dir);
    return fs::path(dir);
}

//-----------------------------------------------------------------------------
//      フルパスに変換します.
//-----------------------------------------------------------------------------
fs::path ToFullPath(const fs::path& value)
{
    char fullPath[512] = {};
    GetFullPathNameA(value.string().c_str(), 512, fullPath, nullptr);
    return fs::path(fullPath);
}

//-----------------------------------------------------------------------------
//      相対パスに変換します.
//-----------------------------------------------------------------------------
fs::path ToRelativePath(const fs::path& base, const fs::path& path, bool directory)
{
    char relativePath[MAX_PATH];
    DWORD attribute = directory ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;

    char srcPath[512];
    GetFullPathNameA(path.string().c_str(), 512, srcPath, nullptr);

    char basePath[512];
    GetFullPathNameA(base.string().c_str(), 512, basePath, nullptr);

    if (PathRelativePathToA(relativePath, basePath, attribute, srcPath, attribute) == TRUE)
    { return fs::path(relativePath); }

    return fs::path();
}

//-----------------------------------------------------------------------------
//      ファイルパスを検索します.
//-----------------------------------------------------------------------------
bool SearchFilePath(const fs::path& filePath, fs::path& result)
{
    if (filePath.empty())
        return false;

    auto checkPath = filePath;
    if (fs::exists(checkPath))
    {
        result = checkPath;
        return true;
    }

    checkPath = fs::path("..\\");
    checkPath += filePath.c_str();
    if (fs::exists(checkPath))
    {
        result = checkPath;
        return true;
    }

    checkPath = fs::path("..\\..\\");
    checkPath += filePath.c_str();
    if (fs::exists(checkPath))
    {
        result = checkPath;
        return true;
    }

    checkPath = fs::path("\\res\\");
    checkPath += filePath.c_str();
    if (fs::exists(checkPath))
    {
        result = checkPath;
        return true;
    }

    auto exePath = GetExePath();
    checkPath = exePath;
    checkPath += "\\";
    checkPath += filePath.c_str();
    if (fs::exists(checkPath))
    {
        result = checkPath;
        return true;
    }

    checkPath = exePath;
    checkPath += "\\..\\";
    checkPath += filePath.c_str();
    if (fs::exists(checkPath))
    {
        result = checkPath;
        return true;
    }

    checkPath = exePath;
    checkPath += "\\..\\..\\";
    checkPath += filePath.c_str();
    if (fs::exists(checkPath))
    {
        result = checkPath;
        return true;
    }

    checkPath = exePath;
    checkPath += "\\res\\";
    checkPath += filePath.c_str();
    if (fs::exists(checkPath))
    {
        result = checkPath;
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
//      特定ディレクトリ下の指定拡張子を持つファイルリストを取得します.
//-----------------------------------------------------------------------------
bool SearchFilesA(const char* directory, const char* ext, std::list<std::string>& result)
{
    WIN32_FIND_DATAA find;

    std::string targetDir = directory;
    std::string dir = directory;

    auto pos = targetDir.find_last_of("\\");
    if (pos != targetDir.size() - 1)
    {
        targetDir += "\\";
        dir += "\\";
    }

    targetDir += "*";

    if (ext != nullptr && strcmp(ext, "") != 0)
    { targetDir += ext; }

    auto handle = FindFirstFileA(targetDir.c_str(), &find);
    if (handle == INVALID_HANDLE_VALUE)
    { return false; }

    std::string file;

    // ディレクトリじゃない場合.
    if ((find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
        file = dir;
        file += find.cFileName;
        result.push_back(file);
    }

    while (FindNextFileA(handle, &find))
    {
        // ディレクトリじゃない場合
        if ((find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            file = dir;
            file += find.cFileName;
            result.push_back(file);
        }
    }

    FindClose(handle);
    return result.empty() == false;
}


//-----------------------------------------------------------------------------
//      オープンファイルダイアログです.
//-----------------------------------------------------------------------------
bool OpenFileDlg(const char* fileFilter, fs::path& result, const fs::path& defaultPath)
{
    OPENFILENAMEA ofn = {};

    CHAR inputFile     [ MAX_PATH ] = { 0 };
    CHAR inputFileTitle[ MAX_PATH ] = { 0 };
    CHAR initDir       [ MAX_PATH ] = { 0 };

    // パスが設定されていれば初期ディレク処理を設定.
    if (!defaultPath.empty() && defaultPath != "")
    {
        auto path = defaultPath.string();
        auto idx = path.find_last_of("\\");
        if (idx != std::string::npos && idx == path.length() - 1)
        { path = path.substr(0, idx); }
        strcpy_s(initDir, path.c_str());
    }

    ofn.lStructSize     = sizeof(OPENFILENAMEA);
    ofn.hwndOwner       = nullptr;
    ofn.lpstrFilter     = fileFilter;
    ofn.nMaxFile        = MAX_PATH;
    ofn.nMaxFileTitle   = MAX_PATH;
    ofn.Flags           = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrTitle      = "Open";
    ofn.lpstrFile       = inputFile;
    ofn.lpstrFileTitle  = inputFileTitle;
    ofn.lpstrInitialDir = initDir;

    if ( GetOpenFileNameA( &ofn ) == TRUE )
    {
        result = fs::path(inputFile);
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
//      セーブファイルダイアログです.
//-----------------------------------------------------------------------------
bool SaveFileDlg(const char* fileFilter, fs::path& result, const fs::path& defaultPath)
{
    OPENFILENAMEA ofn = {};

    CHAR inputFile     [ MAX_PATH ] = { 0 };
    CHAR inputFileTitle[ MAX_PATH ] = { 0 };
    CHAR templateName  [ MAX_PATH ] = { 0 };
    CHAR initDir       [ MAX_PATH ] = { 0 };

    if (!defaultPath.empty() && defaultPath != "")
    {
        auto path = defaultPath.string();
        auto idx = path.find_last_of("\\");
        if (idx != std::string::npos && idx == path.length() - 1)
        { path = path.substr(0, idx); }
        strcpy_s(initDir, path.c_str());
    }

    ofn.lStructSize     = sizeof(OPENFILENAMEA);
    ofn.hwndOwner       = nullptr;
    ofn.lpstrFilter     = fileFilter;
    ofn.nMaxFile        = MAX_PATH;
    ofn.nMaxFileTitle   = MAX_PATH;
    ofn.nFilterIndex    = 1;
    ofn.Flags           = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    ofn.lpstrTitle      = "Save As";
    ofn.lpstrFile       = inputFile;
    ofn.lpstrFileTitle  = inputFileTitle;
    ofn.lpTemplateName  = templateName;
    ofn.lpstrInitialDir = initDir;

    std::string base;
    std::string ext;
    if ( GetSaveFileNameA( &ofn ) == TRUE )
    {
        base = std::string( inputFile ).substr( 0, ofn.nFileExtension - 1 );
        if ( ofn.nFileExtension != 0 )
        {
            ext = std::string( inputFile ).substr( ofn.nFileExtension );
        }
        else
        {
            char* tag = const_cast<char*>(fileFilter);
            for(auto i=1u; i<ofn.nFilterIndex; ++i)
            {
                tag += strlen(tag) + 1;
                tag += strlen(tag) + 1;
            }
            tag += strlen(tag) + 1;
            tag ++;  // *
            ext = std::string(tag);
        }

        result = fs::path((base + ext).c_str());
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
//      フォルダ選択ダイアログを開きます.
//-----------------------------------------------------------------------------
bool OpenFolderDlg(fs::path& result, const fs::path& defaultPath)
{
    IFileDialog* pDlg         = nullptr;
    IShellItem*  pShellItem   = nullptr;
    IShellItem*  pDefaultItem = nullptr;

    DWORD options = 0;

    auto ret = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pDlg));
    if (FAILED(ret))
    { return false; }

    if (!defaultPath.empty() && defaultPath != "")
    {
        auto folder = defaultPath.wstring();
        auto pos = folder.find_last_of(L"\\");
        if (pos != std::wstring::npos && pos == folder.length() - 1)
        { folder = folder.substr(0, pos); }

        ret = SHCreateItemFromParsingName(folder.c_str(), nullptr, IID_PPV_ARGS(&pDefaultItem));
        if (FAILED(ret))
        {
            if (pDefaultItem != nullptr)
            {
                pDefaultItem->Release();
                pDefaultItem = nullptr;
            }
        }
    }

    if (pDefaultItem != nullptr)
    {
        pDlg->SetFolder(pDefaultItem);
    }

    pDlg->GetOptions(&options);
    pDlg->SetOptions(options | FOS_PICKFOLDERS);
    ret = pDlg->Show(nullptr);
    if (FAILED(ret))
    {
        if (pShellItem != nullptr)
        {
            pShellItem->Release();
            pShellItem = nullptr;
        }

        if (pDlg != nullptr)
        {
            pDlg->Release();
            pDlg = nullptr;
        }

        if (pDefaultItem != nullptr)
        {
            pDefaultItem->Release();
            pDefaultItem = nullptr;
        }
        return false;
    }

    ret = pDlg->GetResult(&pShellItem);
    if (FAILED(ret))
    {
        if (pShellItem != nullptr)
        {
            pShellItem->Release();
            pShellItem = nullptr;
        }

        if (pDlg != nullptr)
        {
            pDlg->Release();
            pDlg = nullptr;
        }

        if (pDefaultItem != nullptr)
        {
            pDefaultItem->Release();
            pDefaultItem = nullptr;
        }
        return false;
    }
    
    PWSTR path;
    ret = pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &path);

    if (FAILED(ret))
    {
        if (pShellItem != nullptr)
        {
            pShellItem->Release();
            pShellItem = nullptr;
        }

        if (pDlg != nullptr)
        {
            pDlg->Release();
            pDlg = nullptr;
        }

        if (pDefaultItem != nullptr)
        {
            pDefaultItem->Release();
            pDefaultItem = nullptr;
        }
        return false;
    }

    result = fs::path(path);
    CoTaskMemFree(path);

    if (pShellItem != nullptr)
    {
        pShellItem->Release();
        pShellItem = nullptr;
    }

    if (pDlg != nullptr)
    {
        pDlg->Release();
        pDlg = nullptr;
    }

    if (pDefaultItem != nullptr)
    {
        pDefaultItem->Release();
        pDefaultItem = nullptr;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      ディレクトリを一括削除します.
//-----------------------------------------------------------------------------
bool DeleteDir(const fs::path& path)
{
    WIN32_FIND_DATAA find;

    auto targetDir = path.string();
    targetDir += "*";

    auto handle = FindFirstFileA(targetDir.c_str(), &find);
    if (handle == INVALID_HANDLE_VALUE)
    { return false; }

    std::string dir  = path.string();
    std::string file;

    if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        if (strcmp(find.cFileName, ".") != 0)
        {
            dir += find.cFileName;
            if (!DeleteDir(fs::path(dir.c_str())))
            {
                FindClose(handle);
                return false;
            }
        }
    }
    else
    {
        file = dir;
        file += find.cFileName;
        if (DeleteFileA(file.c_str()) == FALSE)
        {
            FindClose(handle);
            return false;
        }
    }

    while (FindNextFileA(handle, &find))
    {
        if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (strcmp(find.cFileName, "..") != 0)
            {
                dir = path.string();
                dir += find.cFileName;
                if (!DeleteDir(fs::path(dir.c_str())))
                {
                    FindClose(handle);
                    return false;
                }
            }
        }
        else
        {
            file = dir;
            file += find.cFileName;
            if (DeleteFileA(file.c_str()) == FALSE)
            {
                FindClose(handle);
                return false;
            }
        }
    }

    FindClose(handle);
    return (RemoveDirectoryA(path.string().c_str()) != FALSE);
}

} // namespace asdx
