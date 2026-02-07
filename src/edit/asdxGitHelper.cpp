//-----------------------------------------------------------------------------
// File : asdxGitHelper.cpp
// Desc : Git Helper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <edit/asdxGitHelper.h>
#include <fnd/asdxMisc.h>
#include <Windows.h>


namespace {

//-----------------------------------------------------------------------------
//      CP932 から UTF-8 に変換します.
//-----------------------------------------------------------------------------
std::string CP932ToUTF8(const std::string& src)
{
    if (src.empty()) return {};

    int wlen = MultiByteToWideChar(932, 0, src.data(), (int)src.size(), nullptr, 0);
    std::wstring wide(wlen, 0);
    MultiByteToWideChar(932, 0, src.data(), (int)src.size(), wide.data(), wlen);

    int ulen = WideCharToMultiByte(CP_UTF8, 0, wide.data(), wlen, nullptr, 0, nullptr, nullptr);
    std::string utf8(ulen, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), wlen, utf8.data(), ulen, nullptr, nullptr);

    return utf8;
}

//-----------------------------------------------------------------------------
//      コマンドを実行し，標準出力の結果を取得します.
//-----------------------------------------------------------------------------
std::string RunCommand(const char* cmd)
{
    std::string result;
    char buffer[4096];

    FILE* pipe = _popen(cmd, "r");
    if (!pipe) return {};

    while (fgets(buffer, sizeof(buffer), pipe))
        result += buffer;

    _pclose(pipe);
    return result;
}

//-----------------------------------------------------------------------------
//      指定文字列を取り除きます.
//-----------------------------------------------------------------------------
std::string Trim(std::string s)
{
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };

    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());

    return s;
}


} // namespace

namespace asdx {

//-----------------------------------------------------------------------------
//      ファイルを追加します.
//-----------------------------------------------------------------------------
bool GitAdd(const char* filename)
{
    char cmd[2048] = {};
    sprintf_s(cmd, "git add %s", filename);
    return RunProcess(cmd);
}

//-----------------------------------------------------------------------------
//      ファイルを削除します.
//-----------------------------------------------------------------------------
bool GitRemove(const char* filename)
{
    char cmd[2048] = {};
    sprintf_s(cmd, "git rm -f %s", filename);
    return RunProcess(cmd);
}

//-----------------------------------------------------------------------------
//      ファイル名を変更します.
//-----------------------------------------------------------------------------
bool GitRename(const char* oldName, const char* newName)
{
    char cmd[2048] = {};
    sprintf_s(cmd, "git mv \"%s\" \"%s\"", oldName, newName);
    return RunProcess(cmd);
}

//-----------------------------------------------------------------------------
//      プルを行います.
//-----------------------------------------------------------------------------
bool GitPull()
{ return RunProcess("git pull"); }

//-----------------------------------------------------------------------------
//      フェッチを行います.
//-----------------------------------------------------------------------------
bool GitFetch()
{ return RunProcess("git fetch"); }

//-----------------------------------------------------------------------------
//      プッシュを行います.
//-----------------------------------------------------------------------------
bool GitPush(const char* branchName)
{
    char cmd[2048] = {};
    sprintf_s(cmd, "git push origin %s", branchName);
    return RunProcess(cmd);
}

//-----------------------------------------------------------------------------
//      ログを取得します.
//-----------------------------------------------------------------------------
std::vector<GitCommitRecord> GitLog(int maxCount)
{
    std::vector<GitCommitRecord> commits;
    if (maxCount <= 0)
        return commits;

    char cmd[512];
    snprintf(cmd, sizeof(cmd),
        "git log -n %d --pretty=format:\"%%H%%x1f%%an%%x1f%%ad%%x1f%%s%%x1e\" --date=short",
        maxCount);

    std::string raw  = RunCommand(cmd);
    std::string utf8 = CP932ToUTF8(raw);

    size_t pos = 0;
    while (true)
    {
        size_t end = utf8.find('\x1e', pos);
        if (end == std::string::npos)
            break;

        std::string line = utf8.substr(pos, end - pos);
        pos = end + 1;

        GitCommitRecord c;
        size_t p = 0;

        auto next = [&](std::string& out)
        {
            size_t n = line.find('\x1f', p);
            out = line.substr(p, n - p);
            p = n + 1;
        };

        next(c.Hash);
        next(c.Author);
        next(c.Date);
        c.Message = line.substr(p);

        commits.push_back(std::move(c));
    }
    return commits;
}

//-----------------------------------------------------------------------------
//      コミットします.
//-----------------------------------------------------------------------------
bool GitCommit(const char* message)
{
    char cmd[2048] = {};
    sprintf_s(cmd, "git commit --amend %s", message);
    return RunProcess(cmd);
}

//-----------------------------------------------------------------------------
//      指定されたコミットを打ち消します.
//-----------------------------------------------------------------------------
bool GitRevert(const char* commitHash)
{
    char cmd[2048] = {};
    sprintf_s(cmd, "git revert %s", commitHash);
    return RunProcess(cmd);
}

//-----------------------------------------------------------------------------
//      指定されたコミットにリセットします.
//-----------------------------------------------------------------------------
bool GitReset(const char* commitHash, bool hard)
{
    char cmd[2048] = {};
    if (hard)
        sprintf_s(cmd, "git reset --hard %s", commitHash);
    else
        sprintf_s(cmd, "git reset %s", commitHash);

    return RunProcess(cmd);
}

//-----------------------------------------------------------------------------
//      ブランチを作成します.
//-----------------------------------------------------------------------------
bool GitCreateBranch(const char* branchName)
{
    char cmd[2048] = {};
    sprintf_s(cmd, "git branch %s", branchName);
    return RunProcess(cmd);
}

//-----------------------------------------------------------------------------
//      ブランチを削除します.
//-----------------------------------------------------------------------------
bool GitDeleteBranch(const char* branchName)
{
    char cmd[2048] = {};
    sprintf_s(cmd, "git branch -d %s", branchName);
    return RunProcess(cmd);
}

//-----------------------------------------------------------------------------
//      ブランチを切り替えます.
//-----------------------------------------------------------------------------
bool GitCheckout(const char* branchName)
{
    char cmd[2048] = {};
    sprintf_s(cmd, "git checkout %s", branchName);
    return RunProcess(cmd);
}

//-----------------------------------------------------------------------------
//      ブランチをリベースします.
//-----------------------------------------------------------------------------
bool GitRebase(const char* branchName)
{
    char cmd[2048] = {};
    sprintf_s(cmd, "git rebase %s", branchName);
    return RunProcess(cmd);
}

//-----------------------------------------------------------------------------
//      ブランチをマージします.
//-----------------------------------------------------------------------------
bool GitMerge(const char* branchName)
{
    char cmd[2048] = {};
    sprintf_s(cmd, "git merge %s", branchName);
    return RunProcess(cmd);
}

//-----------------------------------------------------------------------------
//      ブランチ一覧を取得します.
//-----------------------------------------------------------------------------
std::vector<std::string> GitBranchList()
{
    std::vector<std::string> branches;

    FILE* pipe = _popen("git branch -a", "r");
    if (!pipe)
        return branches;

    char buf[512];
    while (fgets(buf, sizeof(buf), pipe))
    {
        std::string line = Trim(buf);

        // 現在のブランチ先頭の '*' を除去
        if (!line.empty() && line[0] == '*')
            line = Trim(line.substr(1));

        // 空行除外
        if (!line.empty())
            branches.push_back(line);
    }

    _pclose(pipe);

    return branches;
}

//-----------------------------------------------------------------------------
//      スタッシュに退避します.
//-----------------------------------------------------------------------------
bool GitStashSave(const char* message)
{
    char cmd[2048] = {};
    sprintf_s(cmd, "git stash save %s", message);
    return RunProcess(cmd);
}

//-----------------------------------------------------------------------------
//      スタッシュ一覧を取得します.
//-----------------------------------------------------------------------------
std::vector<GitStashRecord> GetStashList()
{
    std::vector<GitStashRecord> result;

    FILE* pipe = _popen("git stash list", "r");
    if (!pipe)
        return result;

    char buf[1024];

    while (fgets(buf, sizeof(buf), pipe))
    {
        std::string line = Trim(buf);
        if (line.empty())
            continue;

        // 最初の ": " を境界に分割
        size_t pos = line.find(": ");
        if (pos == std::string::npos)
            continue;

        GitStashRecord e;
        e.Name    = line.substr(0, pos);
        e.Message = line.substr(pos + 2);

        result.push_back(std::move(e));
    }

    _pclose(pipe);

    return result;
}

//-----------------------------------------------------------------------------
//      スタッシュに退避した変更を適用します.
//-----------------------------------------------------------------------------
bool GitStashApply(const char* stashName)
{
    char cmd[2048] = {};
    sprintf_s(cmd, "git stash apply %s --index", stashName);
    return RunProcess(cmd);
}

//-----------------------------------------------------------------------------
//      スタッシュに退避した作業を削除します.
//-----------------------------------------------------------------------------
bool GitStashDrop(const char* stashName)
{
    char cmd[2048] = {};
    sprintf_s(cmd, "git stash drop %s", stashName);
    return RunProcess(cmd);
}

} // namespace asdx
