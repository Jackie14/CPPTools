//////////////////////////////////////////////////////////////////////////
// FilePath.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "FilePath.h"
#include <algorithm>
#include "Log.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#ifndef PATH_MAX
#define PATH_MAX 256
#endif


FilePath::FilePath() :
    mIsAbsolute(false)
{
}

FilePath::FilePath(bool isAbsolute) :
    mIsAbsolute(isAbsolute)
{
}

FilePath::FilePath(const std::string& path)
{
    Assign(path);
}

FilePath::FilePath(const std::string& path, Style style)
{
    Assign(path, style);
}

FilePath::FilePath(const char* path)
{
    Assign(path);
}

FilePath::FilePath(const char* path, Style style)
{
    Assign(std::string(path), style);
}

FilePath::FilePath(const FilePath& path) :
    mNode(path.mNode), mDevice(path.mDevice), mName(path.mName), mDirs(
            path.mDirs), mIsAbsolute(path.mIsAbsolute)
{
}

FilePath::FilePath(const FilePath& parent, const std::string& fileName) :
    mNode(parent.mNode), mDevice(parent.mDevice), mName(parent.mName), mDirs(
            parent.mDirs), mIsAbsolute(parent.mIsAbsolute)
{
    MakeDirectory();
    mName = fileName;
}

FilePath::FilePath(const FilePath& parent, const char* fileName) :
    mNode(parent.mNode), mDevice(parent.mDevice), mName(parent.mName), mDirs(
            parent.mDirs), mIsAbsolute(parent.mIsAbsolute)
{
    MakeDirectory();
    mName = std::string(fileName);
}

FilePath::FilePath(const FilePath& parent, const FilePath& relative) :
    mNode(parent.mNode), mDevice(parent.mDevice), mName(parent.mName), mDirs(
            parent.mDirs), mIsAbsolute(parent.mIsAbsolute)
{
    Resolve(relative);
}

FilePath::~FilePath()
{
}

FilePath& FilePath::operator =(const FilePath& path)
{
    return Assign(path);
}

FilePath& FilePath::operator =(const std::string& path)
{
    return Assign(path);
}

FilePath& FilePath::operator =(const char* path)
{
    return Assign(path);
}

FilePath& FilePath::Assign(const FilePath& path)
{
    if (&path != this)
    {
        mNode = path.mNode;
        mDevice = path.mDevice;
        mName = path.mName;
        mDirs = path.mDirs;
        mIsAbsolute = path.mIsAbsolute;
    }
    return *this;
}

FilePath& FilePath::Assign(const std::string& path, Style style)
{
    switch (style)
    {
    case PATH_UNIX:
        ParseUnix(path);
        break;
    case PATH_WINDOWS:
        ParseWindows(path);
        break;
    case PATH_NATIVE:
        Assign(path);
        break;
    case PATH_GUESS:
        ParseGuess(path);
        break;
    default:
        break;
    }
    return *this;
}

FilePath& FilePath::Assign(const char* path)
{
    return Assign(std::string(path));
}

std::string FilePath::ToString(Style style) const
{
    switch (style)
    {
    case PATH_UNIX:
        return BuildUnix();
    case PATH_WINDOWS:
        return BuildWindows();
    case PATH_NATIVE:
    case PATH_GUESS:
        return ToString();
    default:
        break;
    }
    return std::string("");
}

bool FilePath::Parse(const std::string& path)
{
    try
    {
        FilePath p;
        p.Assign(path);
        Assign(p);
        return true;
    }
    catch (...)
    {
        return false;
    }
    return false;
}

bool FilePath::Parse(const std::string& path, Style style)
{
    try
    {
        FilePath p;
        p.Assign(path, style);
        Assign(p);
        return true;
    }
    catch (...)
    {
        return false;
    }
    return false;
}

FilePath& FilePath::ParseDirectory(const std::string& path)
{
    Assign(path);
    return MakeDirectory();
}

FilePath& FilePath::ParseDirectory(const std::string& path, Style style)
{
    Assign(path, style);
    return MakeDirectory();
}

FilePath& FilePath::MakeDirectory()
{
    PushDirectory(mName);
    mName.clear();
    return *this;
}

FilePath& FilePath::MakeFile()
{
    if (!mDirs.empty() && mName.empty())
    {
        mName = mDirs.back();
        mDirs.pop_back();
    }
    return *this;
}

FilePath& FilePath::MakeAbsolute()
{
    return MakeAbsolute(Current());
}

FilePath& FilePath::MakeAbsolute(const FilePath& base)
{
    if (!mIsAbsolute)
    {
        FilePath tmp = base;
        tmp.MakeDirectory();
        for (std::vector<std::string>::const_iterator it = mDirs.begin();
                it != mDirs.end(); ++it)
        {
            tmp.PushDirectory(*it);
        }
        mNode = tmp.mNode;
        mDevice = tmp.mDevice;
        mDirs = tmp.mDirs;
        mIsAbsolute = base.mIsAbsolute;
    }
    return *this;
}

FilePath FilePath::Absolute() const
{
    FilePath result(*this);
    if (!result.mIsAbsolute)
    {
        result.MakeAbsolute();
    }
    return result;
}

FilePath FilePath::Absolute(const FilePath& base) const
{
    FilePath result(*this);
    if (!result.mIsAbsolute)
    {
        result.MakeAbsolute(base);
    }
    return result;
}

FilePath FilePath::Parent() const
{
    FilePath p(*this);
    return p.MakeParent();
}

FilePath& FilePath::MakeParent()
{
    if (mName.empty())
    {
        if (mDirs.empty())
        {
            if (!mIsAbsolute)
            {
                mDirs.push_back("..");
            }
        }
        else
        {
            if (mDirs.back() == "..")
            {
                mDirs.push_back("..");
            }
            else
            {
                mDirs.pop_back();
            }
        }
    }
    else
    {
        mName.clear();
    }
    return *this;
}

FilePath& FilePath::Append(const FilePath& path)
{
    MakeDirectory();
    mDirs.insert(mDirs.end(), path.mDirs.begin(), path.mDirs.end());
    mName = path.mName;
    return *this;
}

FilePath& FilePath::Resolve(const FilePath& path)
{
    if (path.IsAbsolute())
    {
        Assign(path);
    }
    else
    {
        for (int i = 0; i < path.Depth(); ++i)
        {
            PushDirectory(path[i]);
        }
        mName = path.mName;
    }
    return *this;
}

void FilePath::SetNode(const std::string& node)
{
    mNode = node;
    mIsAbsolute = mIsAbsolute || !node.empty();
}

void FilePath::SetDevice(const std::string& device)
{
    mDevice = device;
    mIsAbsolute = mIsAbsolute || !device.empty();
}

const std::string& FilePath::Directory(int n) const
{
    if (n < (int) (mDirs.size()))
    {
        return mDirs[n];
    }
    else
    {
        return mName;
    }
}

const std::string& FilePath::operator [](int n) const
{
    if (n < (int) (mDirs.size()))
    {
        return mDirs[n];
    }
    else
    {
        return mName;
    }
}

void FilePath::PushDirectory(const std::string& dir)
{
    if (!dir.empty() && dir != ".")
    {
        if (dir == "..")
        {
            if (!mDirs.empty() && mDirs.back() != "..")
            {
                mDirs.pop_back();
            }
            else if (!mIsAbsolute)
            {
                mDirs.push_back(dir);
            }
        }
        else
        {
            mDirs.push_back(dir);
        }
    }
}

void FilePath::PopDirectory()
{
    mDirs.pop_back();
}

void FilePath::SetFileName(const std::string& name)
{
    mName = name;
}

void FilePath::SetBaseName(const std::string& name)
{
    std::string ext = GetExtension();
    mName = name;
    if (!ext.empty())
    {
        mName.append(".");
        mName.append(ext);
    }
}

std::string FilePath::GetBaseName() const
{
    char dot = '.';
    std::string::size_type pos = mName.rfind(dot);
    if (pos != std::string::npos)
    {
        return mName.substr(0, pos);
    }
    else
    {
        return mName;
    }
}

void FilePath::SetExtension(const std::string& extension)
{
    mName = GetBaseName();
    if (!extension.empty())
    {
        mName.append(".");
        mName.append(extension);
    }
}

std::string FilePath::GetExtension() const
{
    char dot = '.';
    std::string::size_type pos = mName.rfind(dot);
    if (pos != std::string::npos)
    {
        return mName.substr(pos + 1);
    }
    else
    {
        return std::string();
    }
}

void FilePath::Clear()
{
    mNode.clear();
    mDevice.clear();
    mName.clear();
    mDirs.clear();
    mIsAbsolute = false;
}

void FilePath::ParseUnix(const std::string& path)
{
    Clear();

    std::string::const_iterator it = path.begin();
    std::string::const_iterator end = path.end();

    if (it != end)
    {
        if (*it == '/')
        {
            mIsAbsolute = true;
            ++it;
        }
        else if (*it == '~')
        {
            ++it;
            if (it == end || *it == '/')
            {
                FilePath cwd(Home());
                mDirs = cwd.mDirs;
                mIsAbsolute = true;
            }
            else
            {
                --it;
            }
        }

        while (it != end)
        {
            std::string name;
            while (it != end && *it != '/')
            {
                name += *it++;
            }
            if (it != end)
            {
                if (mDirs.empty())
                {
                    if (!name.empty() && *(name.rbegin()) == ':')
                    {
                        mDevice.assign(name, 0, name.length() - 1);
                    }
                    else
                    {
                        PushDirectory(name);
                    }
                }
                else
                {
                    PushDirectory(name);
                }
            }
            else
            {
                mName = name;
            }

            if (it != end)
            {
                ++it;
            }
        }
    }
}

// Use wchar_t for Windows
void FilePath::ParseWindows(const std::string& path)
{
    Clear();

    std::string::const_iterator it = path.begin();
    std::string::const_iterator end = path.end();

    if (it != end)
    {
        if (*it == '\\' || *it == '/')
        {
            mIsAbsolute = true;
            ++it;
        }

        if (mIsAbsolute && it != end && (*it == '\\' || *it == '/')) // UNC
        {
            ++it;
            while (it != end && *it != '\\' && *it != '/')
            {
                mNode += *it++;
            }
            if (it != end)
            {
                ++it;
            }
        }
        else if (it != end)
        {
            wchar_t d = (wchar_t) *it++;
            if (it != end && *it == ':') // drive letter
            {
                if (mIsAbsolute || !((d >= 'a' && d <= 'z') || (d >= 'A' && d
                        <= 'Z')))
                {
                    LOG(LogError, "Parse failed");
                    return;
                }

                mIsAbsolute = true;
                mDevice += d;
                ++it;
                if (it == end || (*it != '\\' && *it != '/'))
                {
                    LOG(LogError, "Parse failed");
                    return;
                }

                ++it;
            }
            else
            {
                --it;
            }
        }

        while (it != end)
        {
            std::string name;
            while (it != end && *it != '\\' && *it != '/')
            {
                name += *it++;
            }

            if (it != end)
            {
                PushDirectory(name);
            }
            else
            {
                mName = name;
            }

            if (it != end)
            {
                ++it;
            }
        }
    }
    if (!mNode.empty() && mDirs.empty() && !mName.empty())
    {
        MakeDirectory();
    }
}

void FilePath::ParseGuess(const std::string& path)
{
    bool hasBackslash = false;
    bool hasSlash = false;
    bool hasOpenBracket = false;
    bool hasClosBracket = false;
    bool isWindows = path.length() > 2 && path[1] == ':' && (path[2] == '/'
            || path[2] == '\\');
    std::string::const_iterator end = path.end();
    std::string::const_iterator semiIt = end;
    if (!isWindows)
    {
        for (std::string::const_iterator it = path.begin(); it != end; ++it)
        {
            switch (*it)
            {
            case '\\':
                hasBackslash = true;
                break;
            case '/':
                hasSlash = true;
                break;
            case '[':
                hasOpenBracket = true;
            case ']':
                hasClosBracket = hasOpenBracket;
            case ';':
                semiIt = it;
                break;
            }
        }
    }
    if (hasBackslash || isWindows)
    {
        ParseWindows(path);
    }
    else if (hasSlash)
    {
        ParseUnix(path);
    }
    else
    {
        bool isVMS = hasClosBracket;
        if (!isVMS && semiIt != end)
        {
            isVMS = true;
            ++semiIt;
            while (semiIt != end)
            {
                if (*semiIt < '0' || *semiIt > '9')
                {
                    isVMS = false;
                    break;
                }
                ++semiIt;
            }
        }
        ParseUnix(path);
    }
}

std::string FilePath::BuildUnix() const
{
    std::string result;
    if (!mDevice.empty())
    {
        result.append("/");
        result.append(mDevice);
        result.append(":/");
    }
    else if (mIsAbsolute)
    {
        result.append("/");
    }
    for (std::vector<std::string>::const_iterator it = mDirs.begin();
            it != mDirs.end(); ++it)
    {
        result.append(*it);
        result.append("/");
    }
    result.append(mName);
    return result;
}

std::string FilePath::BuildWindows() const
{
    std::string result;
    if (!mNode.empty())
    {
        result.append("\\\\");
        result.append(mNode);
        result.append("\\");
    }
    else if (!mDevice.empty())
    {
        result.append(mDevice);
        result.append(":\\");
    }
    else if (mIsAbsolute)
    {
        result.append("\\");
    }
    for (std::vector<std::string>::const_iterator it = mDirs.begin();
            it != mDirs.end(); ++it)
    {
        result.append(*it);
        result.append("\\");
    }
    result.append(mName);
    return result;
}

//////////////////////////////////////////////////////////////////////////
// Platforms specific
FilePath& FilePath::Assign(const std::string& path)
{
    ParseUnix(path);
    return *this;
}

std::string FilePath::ToString() const
{
    return BuildUnix();
}

std::string FilePath::Current()
{
    std::string path;
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)))
    {
        path = cwd;
    }
    else
    {
        LOG(LogError, "Cannot get current directory");
        return "";
    }
    std::string::size_type n = path.size();
    if (n > 0 && path[n - 1] != '/')
    {
        path.append("/");
    }
    return path;
}

std::string FilePath::Home()
{
    std::string path;
    struct passwd* pwd = getpwuid(getuid());
    if (pwd)
    {
        path = pwd->pw_dir;
    }
    else
    {
        pwd = getpwuid(geteuid());
        if (pwd)
        {
            path = pwd->pw_dir;
        }
        else
        {
            path = getenv("HOME");
        }
    }
    std::string::size_type n = path.size();
    if (n > 0 && path[n - 1] != '/')
    {
        path.append("/");
    }
    return path;
}

std::string FilePath::Temp()
{
    std::string path;
    char* tmp = getenv("TMPDIR");
    if (tmp)
    {
        path = tmp;
        std::string::size_type n = path.size();
        if (n > 0 && path[n - 1] != '/')
            path.append("/");
    }
    else
    {
        path = "/tmp/";
    }
    return path;
}

std::string FilePath::NullDevice()
{
    return "/dev/null";
}

std::string FilePath::OSRoot()
{
    std::string root("/");
    return root;
}

std::string FilePath::Expand(const std::string& path)
{
    std::string result;
    std::string::const_iterator it = path.begin();
    std::string::const_iterator end = path.end();
    if (it != end && *it == '~')
    {
        ++it;
        if (it != end && *it == '/')
        {
            result += Home();
            ++it;
        }
        else
        {
            result += '~';
        }
    }
    while (it != end)
    {
        if (*it == '$')
        {
            std::string var;
            ++it;
            if (it != end && *it == '{')
            {
                ++it;
                while (it != end && *it != '}')
                {
                    var += *it++;
                }
                if (it != end)
                {
                    ++it;
                }
            }
            else
            {
                while (it != end && (std::isalnum(*it) || *it == '_'))
                {
                    var += *it++;
                }
            }
            char* val = getenv(var.c_str());
            if (val)
            {
                result += val;
            }
        }
        else
        {
            result += *it++;
        }
    }
    return result;
}

void FilePath::ListRoots(std::vector<std::string>& roots)
{
    roots.clear();
    roots.push_back("/");
}
