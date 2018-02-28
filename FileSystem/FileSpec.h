//////////////////////////////////////////////////////////////////////////
// FileSpec.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef FileSpec_INCLUDED
#define FileSpec_INCLUDED

#include "Types.h"
#include "Timestamp.h"
#include "ErrorCodes.h"
#include <vector>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <utime.h>
#include <dirent.h>

typedef int FileHandle;

class FilePath;

// The FileSpec class provides methods for working with a file.
class FileSpec
{
public:
    enum ListingMethod
    {
        ListingAll, ListingFiles, ListingSubfolders,
    };

    // Creates the FileSpec object.
    FileSpec();

    // Creates the FileSpec object.
    FileSpec(const std::string& path);

    // Creates the FileSpec object.
    FileSpec(const char* path);

    // Creates the FileSpec object.
    FileSpec(const FilePath& path);

    // Copy constructor.
    FileSpec(const FileSpec& file);

    // Destroys the FileSpec object.
    virtual ~FileSpec();

    // Assignment operator.
    FileSpec& operator =(const FileSpec& file);

    // Assignment operator.
    FileSpec& operator =(const std::string& path);

    // Assignment operator.
    FileSpec& operator =(const char* path);

    // Assignment operator.
    FileSpec& operator =(const FilePath& path);

    // Returns the path.
    const std::string& GetPath() const;

    // Sets path
    void SetPath(const std::string& path);

    // Returns true if the file exists.
    bool Exists() const;

    // Returns true if the file is readable.
    bool CanRead() const;

    // Returns true if the file is writable.
    bool CanWrite() const;

    // Returns true if the file is executable.
    // On Windows, the file must have the extension ".exe" to be executable.
    // On Unix platforms, the executable permission bit must be set.
    bool CanExecute() const;

    // Returns true if the file is a regular file.
    bool IsFile() const;

    // Returns true if the file is a symbolic link.
    bool IsLink() const;

    // Returns true if the file is a directory.
    bool IsDirectory() const;

    // Returns true if the file is a device.
    bool IsDevice() const;

    // Returns true if the file is hidden.
    // On Windows platforms, the file's hidden attribute is set for this to be true.
    // On Unix platforms, the file name must begin with a period for this to be true.
    bool IsHidden() const;

    // Returns the creation date of the file.
    // Not all platforms or filesystems (e.g. Linux and most Unix
    // platforms with the exception of FreeBSD and Mac OS X)
    // maintain the creation date of a file.
    // On such platforms, created() returns the time of the last inode modification.
    Timestamp GetCreated() const;

    // Returns the modification date of the file.
    Timestamp GetLastModified() const;

    // Sets the modification date of the file.
    void SetLastModified(const Timestamp& ts);

    // Returns the size of the file in bytes.
    UInt64 GetSize() const;

    // Sets the size of the file in bytes. Can be used to truncate a file.
    void SetSize(UInt64 size);

    // Makes the file writable (if flag is true), or non-writable (if flag is false)
    // by setting the file's flags in the filesystem accordingly.
    void SetWritable(bool flag = true);

    // Makes the file non-writable (if flag is true), or writable (if flag is false)
    // by setting the file's flags in the filesystem accordingly.
    void SetReadOnly(bool flag = true);

    // Makes the file executable (if flag is true), or non-executable (if flag is false)
    // by setting the file's permission bits accordingly.
    // Does nothing on Windows.	
    void SetExecutable(bool flag = true);

    // Copies the file (or directory) to the given path. 
    // The target path can be a directory. A directory is copied recursively.
    void CopyTo(const std::string& path) const;

    // Copies the file (or directory) to the given path and 
    // removes the original file. The target path can be a directory.
    void MoveTo(const std::string& path);

    // Renames the file to the new name.
    void RenameTo(const std::string& path);

    // Deletes the file. If recursive is true and the
    // file is a directory, recursively deletes all files in the directory.
    void Remove(bool recursive = false);

    // Creates a new, empty file in an atomic operation.
    // Returns true if the file has been created and false if the file already Exists. 
    bool Create();

    // Creates a new, empty file in an atomic operation.
    bool Recreate();

    // Creates a directory.
    // Returns true if the directory has been created and false if it already Exists.
    bool CreateDirectory();

    // Creates a directory (and all parent directories if necessary).
    void CreateDirectories();

    // Fills the vector with the names of all files in the directory.
    void List(std::vector<FileSpec>& files, ListingMethod method = ListingAll) const;

    bool operator ==(const FileSpec& file) const
    {
        return GetPath() == file.GetPath();
    }
    bool operator !=(const FileSpec& file) const
    {
        return GetPath() != file.GetPath();
    }
    bool operator <(const FileSpec& file) const
    {
        return GetPath() < file.GetPath();
    }
    bool operator <=(const FileSpec& file) const
    {
        return GetPath() <= file.GetPath();
    }
    bool operator >(const FileSpec& file) const
    {
        return GetPath() > file.GetPath();
    }
    bool operator >=(const FileSpec& file) const
    {
        return GetPath() >= file.GetPath();
    }

    // For internal use only. 
    static ErrorCode HandleLastError(const std::string& path);

    // File content operations
    // Open exiting file
    // If want to open in read-only mode, passing false
    bool Open(bool isReadWrite = true);
    // Close opened file
    void Close();
    // Read file content to buffer
    int Read(void* buf, UInt64 length);
    // Write buffer to file
    int Write(const void* buf, UInt64 length);
    //#define FILE_BEGIN           0
    //#define FILE_CURRENT         1
    //#define FILE_END             2
    // Or
    // SEEK_SET: The offset is set to offset bytes. 
    // SEEK_CUR: The offset is set to its current location plus offset bytes. 
    // SEEK_END: The offset is set to the size of the file plus offset bytes.
    Int64 Seek(Int64 offset, int from = 0);
    UInt64 GetPosition();
    bool IsHandleValid() const;
    FileHandle GetHandle() const;

    static ErrorCode GetErrorCode()
    {
        return mErrorCode;
    }

    static void SetErrorCode(ErrorCode errCode)
    {
        mErrorCode = errCode;
    }

protected:
    void Init(const std::string& path);
    // Copies a directory. Used internally by CopyTo().
    void CopyDirectory(const std::string& path) const;
    void CopyToImpl(const std::string& path) const;
    void RemoveImpl();

private:
    std::string mPath;
    FileHandle mHandle;
    static ErrorCode mErrorCode;
};

#endif // FileSpec_INCLUDED
