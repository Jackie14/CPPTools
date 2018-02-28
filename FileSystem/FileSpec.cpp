//////////////////////////////////////////////////////////////////////////
// FileSpec.cpp
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#include "FileSpec.h"
#include "FilePath.h"
#include <algorithm>
#include "AutoBuffer.h"
#include "Log.h"
#include "StringUtilities.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <utime.h>
#include <cstring>
#include <dirent.h>

ErrorCode FileSpec::mErrorCode = ErrorOK;

#ifdef _MACOSX
#define O_LARGEFILE 0
#define lseek64 lseek
#endif

FileSpec::FileSpec()
{
    Init(std::string(""));
}

FileSpec::FileSpec(const std::string& path)
{
    Init(path);
}

FileSpec::FileSpec(const char* path)
{
    Init(std::string(path));
}

FileSpec::FileSpec(const FilePath& path)
{
    Init(path.ToString());
}

FileSpec::FileSpec(const FileSpec& file)
{
    Init(file.GetPath());
}

FileSpec::~FileSpec()
{
    Close();
}

void FileSpec::Init(const std::string& path)
{
    mPath = path;
    std::string::size_type n = mPath.size();
    if (n > 1 && (mPath[n - 1] == '\\' || mPath[n - 1] == '/') && !((n == 3
            && mPath[1] == ':')))
    {
        mPath.resize(n - 1);
    }
    mHandle = 0;
}

const std::string& FileSpec::GetPath() const
{
    return mPath;
}

FileSpec& FileSpec::operator =(const FileSpec& file)
{
    SetPath(file.GetPath());
    return *this;
}

FileSpec& FileSpec::operator =(const std::string& path)
{
    SetPath(path);
    return *this;
}

FileSpec& FileSpec::operator =(const char* path)
{
    SetPath(std::string(path));
    return *this;
}

FileSpec& FileSpec::operator =(const FilePath& path)
{
    SetPath(path.ToString());
    return *this;
}

void FileSpec::SetReadOnly(bool flag)
{
    SetWritable(!flag);
}

void FileSpec::CopyTo(const std::string& path) const
{
    FilePath src(GetPath());
    FilePath dest(path);
    FileSpec destFile(path);
    if ((destFile.Exists() && destFile.IsDirectory()) || dest.IsDirectory())
    {
        dest.MakeDirectory();
        dest.SetFileName(src.GetFileName());
    }
    if (IsDirectory())
    {
        CopyDirectory(dest.ToString());
    }
    else
    {
        CopyToImpl(dest.ToString());
    }
}

void FileSpec::CopyDirectory(const std::string& path) const
{
    FileSpec target(path);
    target.CreateDirectories();

    FilePath src(GetPath());
    src.MakeFile();

    std::vector<FileSpec> files;
    List(files, ListingAll);
    for (std::vector<FileSpec>::iterator iter = files.begin(); iter
            != files.end(); ++iter)
    {
        std::string srcPath = (*iter).GetPath();
        std::string destPath = srcPath;
        StringUtilities::Replace(destPath, src.ToString(), path);

        FileSpec fsSrc(srcPath);
        if (fsSrc.IsDirectory())
        {
            FileSpec destFs(destPath);
            // Set the file time here
            destFs.CreateDirectories();
        }
        else
        {
            fsSrc.CopyTo(destPath);
        }
    }
}

void FileSpec::MoveTo(const std::string& path)
{
    CopyTo(path);
    Remove(true);
    SetPath(path);
}

void FileSpec::Remove(bool recursive)
{
    if (recursive && !IsLink() && IsDirectory())
    {
        std::vector<FileSpec> files;
        List(files);
        for (std::vector<FileSpec>::iterator it = files.begin();
                it != files.end(); ++it)
        {
            it->Remove(true);
        }
    }
    RemoveImpl();
}

void FileSpec::CreateDirectories()
{
    if (!Exists())
    {
        FilePath p(GetPath());
        p.MakeDirectory();
        if (p.Depth() > 1)
        {
            p.MakeParent();
            FileSpec f(p);
            f.CreateDirectories();
        }
        CreateDirectory();
    }
}

FileHandle FileSpec::GetHandle() const
{
    return mHandle;
}

//////////////////////////////////////////////////////////////////////////
// Platforms specific
void FileSpec::SetPath(const std::string& path)
{
    mPath = path;
    std::string::size_type n = mPath.size();
    if (n > 1 && mPath[n - 1] == '/')
    {
        mPath.resize(n - 1);
    }
}

bool FileSpec::Exists() const
{
    struct stat st;
    return stat(mPath.c_str(), &st) == 0;
}

bool FileSpec::CanRead() const
{
    struct stat st;
    if (stat(mPath.c_str(), &st) == 0)
    {
        if (geteuid() == 0)
        {
            return true;
        }
        else if (st.st_uid == geteuid())
        {
            return (st.st_mode & S_IRUSR) != 0;
        }
        else if (st.st_gid == getegid())
        {
            return (st.st_mode & S_IRGRP) != 0;
        }
        else
        {
            return (st.st_mode & S_IROTH) != 0;
        }
    }
    else
    {
        HandleLastError(mPath);
    }
    return false;
}

bool FileSpec::CanWrite() const
{
    struct stat st;
    if (stat(mPath.c_str(), &st) == 0)
    {
        if (geteuid() == 0)
        {
            return true;
        }
        else if (st.st_uid == geteuid())
        {
            return (st.st_mode & S_IWUSR) != 0;
        }
        else if (st.st_gid == getegid())
        {
            return (st.st_mode & S_IWGRP) != 0;
        }
        else
        {
            return (st.st_mode & S_IWOTH) != 0;
        }
    }
    else
    {
        HandleLastError(mPath);
    }
    return false;
}

bool FileSpec::CanExecute() const
{
    struct stat st;
    if (stat(mPath.c_str(), &st) == 0)
    {
        if (st.st_uid == geteuid() || geteuid() == 0)
        {
            return (st.st_mode & S_IXUSR) != 0;
        }
        else if (st.st_gid == getegid())
        {
            return (st.st_mode & S_IXGRP) != 0;
        }
        else
        {
            return (st.st_mode & S_IXOTH) != 0;
        }
    }
    else
    {
        HandleLastError(mPath);
    }
    return false;
}

bool FileSpec::IsFile() const
{
    struct stat st;
    if (stat(mPath.c_str(), &st) == 0)
    {
        return S_ISREG(st.st_mode);
    }
    else
    {
        HandleLastError(mPath);
    }

    return false;
}

bool FileSpec::IsDirectory() const
{
    struct stat st;
    if (stat(mPath.c_str(), &st) == 0)
    {
        return S_ISDIR(st.st_mode);
    }
    else
    {
        HandleLastError(mPath);
    }

    return false;
}

bool FileSpec::IsLink() const
{
    struct stat st;
    if (lstat(mPath.c_str(), &st) == 0)
    {
        return S_ISLNK(st.st_mode);
    }
    else
    {
        HandleLastError(mPath);
    }

    return false;
}

bool FileSpec::IsDevice() const
{
    struct stat st;
    if (stat(mPath.c_str(), &st) == 0)
    {
        return S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode);
    }
    else
    {
        HandleLastError(mPath);
    }

    return false;
}

bool FileSpec::IsHidden() const
{
    FilePath p(mPath);
    p.MakeFile();

    return p.GetFileName()[0] == '.';
}

Timestamp FileSpec::GetCreated() const
{
    struct stat st;
    if (stat(mPath.c_str(), &st) == 0)
    {
        return Timestamp::FromEpochTime(st.st_ctime);
    }
    else
    {
        return Timestamp();
    }
}

Timestamp FileSpec::GetLastModified() const
{
    struct stat st;
    if (stat(mPath.c_str(), &st) == 0)
    {
        return Timestamp::FromEpochTime(st.st_mtime);
    }
    else
    {
        HandleLastError(mPath);
    }

    return 0;
}

void FileSpec::SetLastModified(const Timestamp& ts)
{
    struct utimbuf tb;
    tb.actime = ts.GetEpochTime();
    tb.modtime = ts.GetEpochTime();
    if (utime(mPath.c_str(), &tb) != 0)
    {
        HandleLastError(mPath);
    }
}

UInt64 FileSpec::GetSize() const
{
    struct stat st;
    if (stat(mPath.c_str(), &st) == 0)
    {
        return st.st_size;
    }
    else
    {
        HandleLastError(mPath);
    }

    return 0;
}

void FileSpec::SetSize(UInt64 size)
{
    if (truncate(mPath.c_str(), size) != 0)
    {
        HandleLastError(mPath);
    }
}

void FileSpec::SetWritable(bool flag)
{
    struct stat st;
    if (stat(mPath.c_str(), &st) != 0)
    {
        HandleLastError(mPath);
        return;
    }

    mode_t mode;
    if (flag)
    {
        mode = st.st_mode | S_IWUSR;
    }
    else
    {
        mode_t wmask = S_IWUSR | S_IWGRP | S_IWOTH;
        mode = st.st_mode & ~wmask;
    }

    if (chmod(mPath.c_str(), mode) != 0)
    {
        HandleLastError(mPath);
    }
}

void FileSpec::SetExecutable(bool flag)
{
    struct stat st;
    if (stat(mPath.c_str(), &st) != 0)
    {
        HandleLastError(mPath);
        return;
    }

    mode_t mode;
    if (flag)
    {
        mode = st.st_mode | S_IXUSR;
    }
    else
    {
        mode_t wmask = S_IXUSR | S_IXGRP | S_IXOTH;
        mode = st.st_mode & ~wmask;
    }

    if (chmod(mPath.c_str(), mode) != 0)
    {
        HandleLastError(mPath);
    }
}

void FileSpec::CopyToImpl(const std::string& path) const
{
    int sd = open(mPath.c_str(), O_RDONLY);
    if (sd == -1)
    {
        HandleLastError(mPath);
        return;
    }

    struct stat st;
    if (fstat(sd, &st) != 0)
    {
        close(sd);
        HandleLastError(mPath);
        return;
    }
    const long blockSize = st.st_blksize;

    int dd = open(path.c_str(), O_CREAT | O_TRUNC | O_WRONLY, st.st_mode
            & S_IRWXU);
    if (dd == -1)
    {
        close(sd);
        HandleLastError(path);
        return;
    }

    AutoBuffer<char> buffer(blockSize);
    try
    {
        int n;
        while ((n = read(sd, buffer.begin(), blockSize)) > 0)
        {
            if (write(dd, buffer.begin(), n) != n)
            {
                HandleLastError(path);
            }
        }
        if (n < 0)
        {
            HandleLastError(mPath);
            close(sd);
            close(dd);
            return;
        }
    }
    catch (...)
    {
        close(sd);
        close(dd);
        throw;
    }
    close(sd);
    if (fsync(dd) != 0)
    {
        close(dd);
        HandleLastError(path);
    }
    if (close(dd) != 0)
    {
        HandleLastError(path);
    }
}

void FileSpec::RenameTo(const std::string& path)
{
    if (rename(mPath.c_str(), path.c_str()) != 0)
    {
        HandleLastError(mPath);
        return;
    }

    SetPath(path);
}

void FileSpec::RemoveImpl()
{
    int rc;
    if (!IsLink() && IsDirectory())
    {
        rc = rmdir(mPath.c_str());
    }
    else
    {
        rc = unlink(mPath.c_str());
    }

    if (rc)
    {
        HandleLastError(mPath);
    }
}

bool FileSpec::Create()
{
    int n = open(mPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (n != -1)
    {
        close(n);
        return true;
    }

    if (n == -1 && errno == EEXIST)
    {
        return false;
    }
    else
    {
        HandleLastError(mPath);
    }

    return false;
}

bool FileSpec::Recreate()
{
    int n = open(mPath.c_str(),
            O_WRONLY | O_EXCL | O_TRUNC, S_IRUSR | S_IWUSR);
    if (n != -1)
    {
        close(n);
        return true;
    }

    if (n == -1 && errno == EEXIST)
    {
        return false;
    }
    else
    {
        HandleLastError(mPath);
    }

    return false;
}

bool FileSpec::CreateDirectory()
{
    if (Exists() && IsDirectory())
    {
        return false;
    }

    if (mkdir(mPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
    {
        HandleLastError(mPath);
        return false;
    }

    return true;
}

void FileSpec::List(std::vector<FileSpec>& files, ListingMethod method) const
{
    FilePath currentPath(mPath);
    currentPath.MakeDirectory();
    std::string findPath = currentPath.ToString();

    DIR* dp = NULL;
    struct dirent* entry = NULL;
    struct stat statBuf;

    // Check whether the directory is existing or not
    if ((dp = opendir(findPath.c_str())) == NULL)
    {
        HandleLastError(mPath);
        return;
    }

    // Change to the root directory
    chdir(findPath.c_str());
    while ((entry = readdir(dp)) != NULL)
    {
        // Compose full path
        std::string currentName = entry->d_name;
        FilePath temp = currentPath;
        temp.Append(currentName);
        FileSpec currentFile(temp);

        lstat(entry->d_name, &statBuf);
        if (S_ISDIR(statBuf.st_mode))
        {
            // Find a directory, but ignore . and ..
            if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name)
                    == 0)
            {
                continue;
            }

            // Recursively listing dirs
            if ((method == ListingAll) || (method == ListingSubfolders))
            {
                files.push_back(currentFile);
            }

            currentFile.List(files, method);
        }
        else
        {
            // Find a file
            LOG(LogInfo, currentFile.GetPath().c_str());

            if ((method == ListingAll) || (method == ListingFiles))
            {
                files.push_back(currentFile);
            }
        }
    }

    // Back to parent directory
    chdir("..");

    // Close the opening directory
    closedir(dp);
}

ErrorCode FileSpec::HandleLastError(const std::string& path)
{
    ErrorCode ret;

    switch (errno)
    {
    case EIO:
        LOG(LogError, "IO Error");
        ret = ErrorFileIO;
        break;
    case EPERM:
        LOG(LogError, "Insufficient permissions: %s", path.c_str());
        ret = ErrorFileAccessDenied;
        break;
    case EACCES:
        LOG(LogError, "File access denied: %s", path.c_str());
        ret = ErrorFileAccessDenied;
        break;
    case ENOENT:
        LOG(LogError, "File not found: %s", path.c_str());
        ret = ErrorFileNotFound;
        break;
    case ENOTDIR:
        LOG(LogError, "Not a directory: %s", path.c_str());
        ret = ErrorFileNotDirectory;
        break;
    case EISDIR:
        LOG(LogError, "Not a file: %s", path.c_str());
        ret = ErrorFileNotFile;
        break;
    case EROFS:
        LOG(LogError, "File readonly: %s", path.c_str());
        ret = ErrorFileReadOnly;
        break;
    case EEXIST:
        LOG(LogError, "File exists: %s", path.c_str());
        ret = ErrorFileExists;
        break;
    case ENOSPC:
        LOG(LogError, "No space left on device: %s", path.c_str());
        ret = ErrorFileNoSpaceLeft;
        break;
    case EDQUOT:
        LOG(LogError, "Disk quota exceeded: %s", path.c_str());
        ret = ErrorFileQuotaExceeded;
        break;
    case ENOTEMPTY:
        LOG(LogError, "Directory not empty: %s", path.c_str());
        ret = ErrorFileDirectoryNotEmpty;
        break;
    case ENAMETOOLONG:
        LOG(LogError, "Path syntax error: %s", path.c_str());
        ret = ErrorFilePathSyntax;
        break;
    case ENFILE:
        LOG(LogError, "File table overflow: %s", path.c_str());
        ret = ErrorFileTableOverflow;
        break;
    case EMFILE:
        LOG(LogError, "Too many open files: %s", path.c_str());
        ret = ErrorFileTooManyOpening;
        break;
    default:
        LOG(LogError, "File error: %s", path.c_str());
        ret = ErrorFile;
        break;
    }

    SetErrorCode(ret);
    return ret;
}

bool FileSpec::Open(bool isReadWrite)
{
    Close();

    if (mPath.empty())
    {
        return false;
    }

    int flags = O_LARGEFILE | O_CREAT;
    if(isReadWrite)
    {
        flags |= O_RDWR;
    }
    else
    {
        flags |= O_RDONLY;
    }
    mHandle = open(mPath.c_str(), flags, 0755);
    if (mHandle == -1)
    {
        return false;
    }

    return true;
}

void FileSpec::Close()
{
    if (mHandle > 0)
    {
        close(mHandle);
        mHandle = 0;
    }
}

int FileSpec::Read(void* buf, UInt64 length)
{
    if (!IsHandleValid())
    {
        return 0;
    }

    return read(mHandle, buf, length);
}

int FileSpec::Write(const void* buf, UInt64 length)
{
    if (!IsHandleValid())
    {
        return 0;
    }

    return write(mHandle, buf, length);
}

Int64 FileSpec::Seek(Int64 offset, int from)
{
    if (!IsHandleValid())
    {
        return -1;
    }

    return lseek64(mHandle, offset, from);
}

UInt64 FileSpec::GetPosition()
{
    if (!IsHandleValid())
    {
        return -1;
    }

    return lseek64(mHandle, 0, SEEK_CUR);
}

bool FileSpec::IsHandleValid() const
{
    if (mHandle <= 0)
    {
        return false;
    }

    return true;
}

