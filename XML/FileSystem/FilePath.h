//////////////////////////////////////////////////////////////////////////
// FilePath.h
// Yuchuan Wang
//////////////////////////////////////////////////////////////////////////

#ifndef FilePath_INCLUDED
#define FilePath_INCLUDED

#include <vector>
#include <string>

class FilePath
{
public:
    enum Style
    {
        PATH_UNIX, // Unix-style path
        PATH_WINDOWS, // Windows-style path
        PATH_NATIVE, // The current platform's native style
        PATH_GUESS   // Guess the style by examining the path
    };

    // Creates an empty relative path.
    FilePath();

    // Creates an empty absolute or relative path.
    FilePath(bool isAbsolute);

    // Creates a path from a string.
    FilePath(const char* path);

    // Creates a path from a string.
    FilePath(const char* path, Style style);

    // Creates a path from a string.
    FilePath(const std::string& path);

    // Creates a path from a string.
    FilePath(const std::string& path, Style style);

    // Copy constructor
    FilePath(const FilePath& path);

    // Creates a path from a parent path and a filename.
    // The parent path is expected to reference a directory.
    FilePath(const FilePath& parent, const std::string& fileName);

    // Creates a path from a parent path and a filename.
    // The parent path is expected to reference a directory.
    FilePath(const FilePath& parent, const char* fileName);

    // Creates a path from a parent path and a relative path.
    // The parent path is expected to reference a directory.
    // The relative path is appended to the parent path.
    FilePath(const FilePath& parent, const FilePath& relative);

    // Destroys the FilePath.
    ~FilePath();

    // Assignment operator.
    FilePath& operator =(const FilePath& path);

    // Assigns a string containing a path in native format.
    FilePath& operator =(const std::string& path);

    // Assigns a string containing a path in native format.
    FilePath& operator =(const char* path);

    // Assigns a string containing a path in native format.
    FilePath& Assign(const std::string& path);

    // Assigns a string containing a path.
    FilePath& Assign(const std::string& path, Style style);

    // Assigns the given path.
    FilePath& Assign(const FilePath& path);

    // Assigns a string containing a path.
    FilePath& Assign(const char* path);

    // Returns a string containing the path in native format.
    std::string ToString() const;

    // Returns a string containing the path in the given format.
    std::string ToString(Style style) const;

    // Tries to interpret the given string as a path in native format.
    // If the path is syntactically valid, assigns the path and returns true.
    // Otherwise leaves the object unchanged and returns false.
    bool Parse(const std::string& path);

    // Tries to interpret the given string as a path, according to the given style.
    // If the path is syntactically valid, assigns the path and returns true.
    // Otherwise leaves the object unchanged and returns false.
    bool Parse(const std::string& path, Style style);

    // The resulting path always refers to a directory and the filename part is empty.
    FilePath& ParseDirectory(const std::string& path);

    // The resulting path always refers to a directory and the filename part is empty.
    FilePath& ParseDirectory(const std::string& path, Style style);

    // If the path contains a filename, the filename is appended to the directory list and cleared.
    // Thus the resulting path always refers to a directory.
    FilePath& MakeDirectory();

    // If the path contains no filename, the last directory becomes the filename.
    FilePath& MakeFile();

    // Makes the path refer to its parent.
    FilePath& MakeParent();

    // Makes the path absolute if it is relative.
    // The current working directory is taken as base directory.
    FilePath& MakeAbsolute();

    // Makes the path absolute if it is relative.
    // The given path is taken as base. 
    FilePath& MakeAbsolute(const FilePath& base);

    // Appends the given path.
    FilePath& Append(const FilePath& path);

    // Resolves the given path against the current one.
    // If the given path is absolute, it replaces the current one.
    // Otherwise, the relative path is appended to the current path.
    FilePath& Resolve(const FilePath& path);

    // Returns true if the path is absolute.
    bool IsAbsolute() const
    {
        return mIsAbsolute;
    }

    // Returns true if the path is relative.
    bool IsRelative() const
    {
        return !mIsAbsolute;
    }

    // Returns true if the path references a directory (the filename part is empty).
    bool IsDirectory() const
    {
        return mName.empty();
    }

    // Returns true if the path references a file (the filename part is not empty).
    bool IsFile() const
    {
        return !mName.empty();
    }

    // Sets the node name.
    // Setting a non-empty node automatically makes the path an absolute one.
    void SetNode(const std::string& node);

    // Returns the node name.
    const std::string& GetNode() const
    {
        return mNode;
    }

    // Sets the device name.
    // Setting a non-empty device automatically makes the path an absolute one.
    void SetDevice(const std::string& device);

    // Returns the device name.
    const std::string& GetDevice() const
    {
        return mDevice;
    }

    // Returns the number of directories in the directory list.
    int Depth() const
    {
        return int(mDirs.size());
    }

    // Returns the n'th directory in the directory list.
    // If n == depth(), returns the filename.
    const std::string& Directory(int n) const;

    // Returns the n'th directory in the directory list.
    // If n == depth(), returns the filename.
    const std::string& operator [](int n) const;

    // Adds a directory to the directory list.
    void PushDirectory(const std::string& dir);

    // Removes the last directory from the directory list.
    void PopDirectory();

    // Sets the filename.
    void SetFileName(const std::string& name);

    // Returns the filename.
    const std::string& GetFileName() const
    {
        return mName;
    }

    // Sets the basename part of the filename and
    // does not change the extension.
    void SetBaseName(const std::string& name);

    // Returns the basename (the filename sans extension) of the path. 
    // Filename without extension
    std::string GetBaseName() const;

    // Sets the filename extension.
    void SetExtension(const std::string& extension);

    // Returns the filename extension.
    std::string GetExtension() const;

    // Clears all components.
    void Clear();

    // Returns a path referring to the path's directory.
    FilePath Parent() const;

    // Returns an absolute variant of the path, taking the current working directory as base.
    FilePath Absolute() const;

    // Returns an absolute variant of the path, taking the given path as base.
    FilePath Absolute(const FilePath& base) const;

    // Creates a path referring to a directory.
    static FilePath ForDirectory(const std::string& path)
    {
        FilePath p;
        return p.ParseDirectory(path);
    }

    // Creates a path referring to a directory.
    static FilePath ForDirectory(const std::string& path, Style style)
    {
        FilePath p;
        return p.ParseDirectory(path, style);
    }

    // Returns the platform's path name separator, which separates the components (names) in a path.
    // On Unix systems, this is the slash '/'. On Windows systems, this is the backslash '\'.
    static char Separator()
    {
        return '/';
    }

    // Returns the platform's path separator, which separates single paths in a list of paths.
    // On Unix systems, this is the colon ':'. On Windows systems, this is the semicolon ';'.
    static char PathSeparator()
    {
        return ':';
    }

    // Returns the current working directory.
    static std::string Current();

    // Returns the user's home directory.
    static std::string Home();

    // Returns the temporary directory.
    static std::string Temp();

    // Returns the name of the NULL device.
    static std::string NullDevice();

    // Returns the OS root folder.
    static std::string OSRoot();

    // Expands all environment variables contained in the path.
    // On Unix, a tilde as first character in the path is replaced with the path to user's home directory.
    static std::string Expand(const std::string& path);

    // Fills the vector with all filesystem roots available on the system.
    // On Unix, there is exactly one root, "/". On Windows, the roots are the drive letters.
    static void ListRoots(std::vector<std::string>& roots);

protected:
    void ParseUnix(const std::string& path);
    void ParseWindows(const std::string& path);
    void ParseGuess(const std::string& path);
    std::string BuildUnix() const;
    std::string BuildWindows() const;

private:
    std::string mNode;
    std::string mDevice;
    std::string mName;
    std::vector<std::string> mDirs;
    bool mIsAbsolute;
};

#endif // FilePath_INCLUDED
