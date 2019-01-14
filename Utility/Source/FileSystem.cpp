#include "FileSystem.h"
#include "Exception.h"
#include "Debug.h"

#if PLATFORM_WINDOWS
#   include <windows.h>
#   include "Unicode.h"
#else
#   include <dirent.h>
#   include <errno.h>
#   include <sys/stat.h>
#   include <sys/types.h>
#   include <unistd.h>

#   include <climits>
#   include <cstring>
#   include <cstdio>
#   include <cstdlib>
#   include <fstream>

#define HANDLE_PATH_ERROR(path__, errno__) \
    LOGERR(String(__FUNCTION__) + ": " + (path__) + ": " + (strerror(errno__)));
#endif

namespace Utility
{
#   if PLATFORM_WINDOWS
    void win32_handleError(DWORD error, const WString& path);
    WString win32_getCurrentDirectory();
    WString win32_getTempDirectory();
    bool win32_pathExists(const WString& path);
    bool win32_isDirectory(const WString& path);
    bool win32_isDevice(const WString& path);
    bool win32_isFile(const WString& path);
    bool win32_createFile(const WString& path);
    bool win32_createDirectory(const WString& path);
    UINT64 win32_getFileSize(const WString& path);
    std::time_t win32_getLastModifiedTime(const WString& path);
#   else
    bool unix_pathExists(const String& path);
    bool unix_stat(const String& path, struct stat *st_buf);
    bool unix_isFile(const String& path);
    bool unix_isDirectory(const String& path);
    bool unix_createDirectory(const String& path);
#   endif
    
    SPtr<DataStream> FileSystem::openFile(const Path& path, bool readOnly)
    {
#   if PLATFORM_WINDOWS
        WString pathWString = UTF8::toWide(path.toString());
        const wchar_t* pathString = pathWString.c_str();
        
        if (!win32_pathExists(pathString) || !win32_isFile(pathString))
        {
            LOGWRN("Attempting to open a file that doesn't exist: " + path.toString());
            return nullptr;
        }
#   else
        String pathString = path.toString();
#   endif
        
        DataStream::AccessMode accessMode = DataStream::READ;
        if (!readOnly)
            accessMode = (DataStream::AccessMode)((UINT32)accessMode | (UINT32)DataStream::WRITE);
        
        return SharedPtrNew<FileDataStream>(path, accessMode, true);
    }
    
    SPtr<DataStream> FileSystem::createAndOpenFile(const Path& path)
    {
        return SharedPtrNew<FileDataStream>(path, DataStream::AccessMode::WRITE, true);
    }
    
    UINT64 FileSystem::getFileSize(const Path& path)
    {
#   if PLATFORM_WINDOWS
        return win32_getFileSize(UTF8::toWide(path.toString()));
#   else
        struct stat st_buf;
        
        if (stat(path.toString().c_str(), &st_buf) == 0)
        {
            return (UINT64)st_buf.st_size;
        }
        else
        {
            HANDLE_PATH_ERROR(path.toString(), errno);
            return (UINT64)-1;
        }
#   endif
    }

	void FileSystem::remove(const Path& path, bool recursively)
	{
		if (!FileSystem::exists(path))
			return;

		if (recursively)
		{
			Vector<Path> files;
			Vector<Path> directories;

			getChildren(path, files, directories);

			for (auto& file : files)
				remove(file, false);

			for (auto& dir : directories)
				remove(dir, true);
		}

		FileSystem::removeFile(path);
	}

	void FileSystem::move(const Path& oldPath, const Path& newPath, bool overwriteExisting)
	{
		if (FileSystem::exists(newPath))
		{
			if (overwriteExisting)
				FileSystem::remove(newPath);
			else
			{
                LOGWRN("Move operation failed because another file already exists at the new path: \"" + newPath.toString() + "\"");
				return;
			}
		}

		FileSystem::moveFile(oldPath, newPath);
	}
    
    void FileSystem::copy(const Path& oldPath, const Path& newPath, bool overwriteExisting)
    {
        Stack<std::tuple<Path, Path>> todo;
        todo.push(std::make_tuple(oldPath, newPath));
        
        while (!todo.empty())
        {
            auto current = todo.top();
            todo.pop();
            
            Path sourcePath = std::get<0>(current);
            if (!FileSystem::exists(sourcePath))
                continue;
            
            bool srcIsFile = FileSystem::isFile(sourcePath);
            Path destinationPath = std::get<1>(current);
            bool destExists = FileSystem::exists(destinationPath);
            
            if (destExists)
            {
                if (FileSystem::isFile(destinationPath))
                {
                    if (overwriteExisting)
                        FileSystem::remove(destinationPath);
                    else
                    {
                        LOGWRN("Copy operation failed because another file already exists at the new path: \"" + destinationPath.toString() + "\"");
                        return;
                    }
                }
            }
            
            if (srcIsFile)
            {
                FileSystem::copyFile(sourcePath, destinationPath);
            }
            else
            {
                if (!destExists)
                    FileSystem::createDir(destinationPath);
                
                Vector<Path> files;
                Vector<Path> directories;
                getChildren(destinationPath, files, directories);
                
                for (auto& file : files)
                {
                    Path fileDestPath = destinationPath;
                    fileDestPath.append(file.getTail());
                    
                    todo.push(std::make_tuple(file, fileDestPath));
                }
                
                for (auto& dir : directories)
                {
                    Path dirDestPath = destinationPath;
                    dirDestPath.append(dir.getTail());
                    
                    todo.push(std::make_tuple(dir, dirDestPath));
                }
            }
        }
    }
    
    void FileSystem::createDir(const Path& path)
    {
        Path parentPath = path;
        while (!exists(parentPath) && parentPath.getNumDirectories() > 0)
        {
            parentPath = parentPath.getParent();
        }
        
#   if PLATFORM_WINDOWS
        for (UINT32 i = parentPath.getNumDirectories(); i < path.getNumDirectories(); ++i)
        {
            parentPath.append(path[i]);
            win32_createDirectory(UTF8::toWide(parentPath.toString()));
        }
        
        if (path.isFile())
            win32_createDirectory(UTF8::toWide(path.toString()));
#   else
        for (UINT32 i = parentPath.getNumDirectories(); i < path.getNumDirectories(); ++i)
        {
            parentPath.append(path[i]);
            unix_createDirectory(parentPath.toString());
        }
        
        // Last "file" entry is also considered a directory
        if(!parentPath.equals(path))
            unix_createDirectory(path.toString());
#   endif
    }
    
    bool FileSystem::exists(const Path& path)
    {
#   if PLATFORM_WINDOWS
        return win32_pathExists(UTF8::toWide(path.toString()));
#   else
        return unix_pathExists(path.toString());
#   endif
    }
    
    bool FileSystem::isFile(const Path& path)
    {
#   if PLATFORM_WINDOWS
        WString pathStr = UTF8::toWide(path.toString());
        
        return win32_pathExists(pathStr) && win32_isFile(pathStr);
#   else
        String pathStr = path.toString();
        return unix_pathExists(pathStr) && unix_isFile(pathStr);
#   endif
    }
    
    bool FileSystem::isDirectory(const Path& path)
    {
#   if PLATFORM_WINDOWS
        WString pathStr = UTF8::toWide(path.toString());
        
        return win32_pathExists(pathStr) && win32_isDirectory(pathStr);
#   else
        String pathStr = path.toString();
        return unix_pathExists(pathStr) && unix_isDirectory(pathStr);
#   endif
    }
    
    void FileSystem::getChildren(const Path& dirPath, Vector<Path>& files, Vector<Path>& directories)
    {
#   if PLATFORM_WINDOWS
        WString findPath = UTF8::toWide(dirPath.toString());
        
        if (win32_isFile(findPath))
            return;
        
        if(dirPath.isFile()) // Assuming the file is a folder, just improperly formatted in Path
            findPath.append(L"\\*");
        else
            findPath.append(L"*");
        
        WIN32_FIND_DATAW findData;
        HANDLE fileHandle = FindFirstFileW(findPath.c_str(), &findData);
        if(fileHandle == INVALID_HANDLE_VALUE)
        {
            win32_handleError(GetLastError(), findPath);
            return;
        }
        
        WString tempName;
        do
        {
            tempName = findData.cFileName;
            
            if (tempName != L"." && tempName != L"..")
            {
                Path fullPath = dirPath;
                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
                    directories.push_back(fullPath.append(UTF8::fromWide(tempName) + u8"/"));
                else
                    files.push_back(fullPath.append(UTF8::fromWide(tempName)));
            }
            
            if(FindNextFileW(fileHandle, &findData) == FALSE)
            {
                if (GetLastError() != ERROR_NO_MORE_FILES)
                    win32_handleError(GetLastError(), findPath);
                
                break;
            }
        } while (true);
        
        FindClose(fileHandle);
#   else
        const String pathStr = dirPath.toString();
        
        if (unix_isFile(pathStr))
            return;
        
        DIR *dp = opendir(pathStr.c_str());
        if (dp == NULL)
        {
            HANDLE_PATH_ERROR(pathStr, errno);
            return;
        }
        
        struct dirent *ep;
        while ( (ep = readdir(dp)) )
        {
            const String filename(ep->d_name);
            if (filename != "." && filename != "..")
            {
                if (unix_isDirectory(pathStr + "/" + filename))
                    directories.push_back(dirPath + (filename + "/"));
                else
                    files.push_back(dirPath + filename);
            }
        }
        closedir(dp);
#   endif
    }
    
    bool FileSystem::iterate(const Path& dirPath, std::function<bool(const Path&)> fileCallback,
                             std::function<bool(const Path&)> dirCallback, bool recursive)
    {
#   if PLATFORM_WINDOWS
        WString findPath = UTF8::toWide(dirPath.toString());
        
        if (win32_isFile(findPath))
            return false;
        
        if (dirPath.isFile()) // Assuming the file is a folder, just improperly formatted in Path
            findPath.append(L"\\*");
        else
            findPath.append(L"*");
        
        WIN32_FIND_DATAW findData;
        HANDLE fileHandle = FindFirstFileW(findPath.c_str(), &findData);
        if (fileHandle == INVALID_HANDLE_VALUE)
        {
            win32_handleError(GetLastError(), findPath);
            return false;
        }
        
        WString tempName;
        do
        {
            tempName = findData.cFileName;
            
            if (tempName != L"." && tempName != L"..")
            {
                Path fullPath = dirPath;
                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
                {
                    Path childDir = fullPath.append(UTF8::fromWide(tempName) + u8"/");
                    if (dirCallback != nullptr)
                    {
                        if (!dirCallback(childDir))
                        {
                            FindClose(fileHandle);
                            return false;
                        }
                    }
                    
                    if (recursive)
                    {
                        if (!iterate(childDir, fileCallback, dirCallback, recursive))
                        {
                            FindClose(fileHandle);
                            return false;
                        }
                    }
                }
                else
                {
                    Path filePath = fullPath.append(UTF8::fromWide(tempName));
                    if (fileCallback != nullptr)
                    {
                        if (!fileCallback(filePath))
                        {
                            FindClose(fileHandle);
                            return false;
                        }
                    }
                }
            }
            
            if(FindNextFileW(fileHandle, &findData) == FALSE)
            {
                if (GetLastError() != ERROR_NO_MORE_FILES)
                    win32_handleError(GetLastError(), findPath);
                
                break;
            }
        } while (true);
        
        FindClose(fileHandle);
        return true;
#   else
        String pathStr = dirPath.toString();
        
        if (unix_isFile(pathStr))
            return false;
        
        DIR* dirHandle = opendir(pathStr.c_str());
        if (dirHandle == nullptr)
        {
            HANDLE_PATH_ERROR(pathStr, errno);
            return false;
        }
        
        dirent* entry;
        while((entry = readdir(dirHandle)))
        {
            String filename(entry->d_name);
            if (filename == "." || filename == "..")
                continue;
            
            Path fullPath = dirPath;
            if (unix_isDirectory(pathStr + "/" + filename))
            {
                Path childDir = fullPath.append(filename + "/");
                if (dirCallback != nullptr)
                {
                    if (!dirCallback(childDir))
                    {
                        closedir(dirHandle);
                        return false;
                    }
                }
                
                if (recursive)
                {
                    if (!iterate(childDir, fileCallback, dirCallback, recursive))
                    {
                        closedir(dirHandle);
                        return false;
                    }
                }
            }
            else
            {
                Path filePath = fullPath.append(filename);
                if (fileCallback != nullptr)
                {
                    if (!fileCallback(filePath))
                    {
                        closedir(dirHandle);
                        return false;
                    }
                }
            }
        }
        closedir(dirHandle);
        
        return true;
#   endif
    }
    
    std::time_t FileSystem::getLastModifiedTime(const Path& path)
    {
#   if PLATFORM_WINDOWS
        return win32_getLastModifiedTime(UTF8::toWide(path.toString()));
#   else
        struct stat st_buf;
        stat(path.toString().c_str(), &st_buf);
        std::time_t time = st_buf.st_mtime;
        
        return time;
#   endif
    }
    
    Path FileSystem::getWorkingDirectoryPath()
    {
#   if PLATFORM_WINDOWS
        const String utf8dir = UTF8::fromWide(win32_getCurrentDirectory());
        return Path(utf8dir);
#   else
        char *buffer = NewN<char>(PATH_MAX);
        
        String wd;
        if (getcwd(buffer, PATH_MAX) != nullptr)
            wd = buffer;
        else
            LOGERR(String("Error when calling getcwd(): ") + strerror(errno));
        
        Free(buffer);
        return Path(wd);
#   endif
    }
    
    Path FileSystem::getTempDirectoryPath()
    {
#   if PLATFORM_WINDOWS
        const String utf8dir = UTF8::fromWide(win32_getTempDirectory());
        return Path(utf8dir);
#   else
        String tmpdir;
        
        // Try different things:
        // 1) If defined, honor the TMPDIR environnement variable
        char* TMPDIR = getenv("TMPDIR");
        if (TMPDIR != nullptr)
            tmpdir = TMPDIR;
        else
        {
            // 2) If defined, honor the P_tmpdir macro
#ifdef P_tmpdir
            tmpdir = String(P_tmpdir);
#else
            // 3) If everything else fails, simply default to /tmp
            tmpdir = String("/tmp");
#endif
        }
        
        tmpdir.append("/bsf-XXXXXX");
        
        // null terminated, modifiable tmpdir name template
        Vector<char> nameTemplate(tmpdir.c_str(), tmpdir.c_str() + tmpdir.size() + 1);
        char *directoryName = mkdtemp(nameTemplate.data());
        
        if (directoryName == nullptr)
        {
            LOGERR(String(__FUNCTION__) + ": " + strerror(errno));
            return Path(StringUtil::BLANK);
        }
        
        return Path(String(directoryName) + "/");
#   endif
    }
    
    void FileSystem::copyFile(const Path& source, const Path& destination)
    {
#   if PLATFORM_WINDOWS
        WString fromStr = UTF8::toWide(source.toString());
        WString toStr = UTF8::toWide(destination.toString());
        
        if (CopyFileW(fromStr.c_str(), toStr.c_str(), FALSE) == FALSE)
            win32_handleError(GetLastError(), fromStr);
#   else
        std::ifstream sourceStream(source.toString().c_str(), std::ios::binary);
        std::ofstream destinationStream(destination.toString().c_str(), std::ios::binary);
        
        destinationStream << sourceStream.rdbuf();
        sourceStream.close();
        destinationStream.close();
#   endif
    }
    
    void FileSystem::removeFile(const Path& path)
    {
#   if PLATFORM_WINDOWS
        WString pathStr = UTF8::toWide(path.toString());
        if (win32_isDirectory(pathStr))
        {
            if (RemoveDirectoryW(pathStr.c_str()) == 0)
                win32_handleError(GetLastError(), pathStr);
        }
        else
        {
            if (DeleteFileW(pathStr.c_str()) == 0)
                win32_handleError(GetLastError(), pathStr);
        }
#   else
        String pathStr = path.toString();
        if (unix_isDirectory(pathStr))
        {
            if (rmdir(pathStr.c_str()))
                HANDLE_PATH_ERROR(pathStr, errno);
        }
        else
        {
            if (unlink(pathStr.c_str()))
                HANDLE_PATH_ERROR(pathStr, errno);
        }
#   endif
    }
    
    void FileSystem::moveFile(const Path& oldPath, const Path& newPath)
    {
#   if PLATFORM_WINDOWS
        WString oldPathStr = UTF8::toWide(oldPath.toString());
        WString newPathStr = UTF8::toWide(newPath.toString());
        
        if (MoveFileW(oldPathStr.c_str(), newPathStr.c_str()) == 0)
            win32_handleError(GetLastError(), oldPathStr);
#   else
        String oldPathStr = oldPath.toString();
        String newPathStr = newPath.toString();
        if (std::rename(oldPathStr.c_str(), newPathStr.c_str()) == -1)
        {
            // Cross-filesystem copy is likely needed (for example, /tmp to Banshee install dir while copying assets)
            std::ifstream src(oldPathStr.c_str(), std::ios::binary);
            std::ofstream dst(newPathStr.c_str(), std::ios::binary);
            dst << src.rdbuf(); // First, copy
            
            // Error handling
            src.close();
            if (!src)
            {
                LOGERR(String(__FUNCTION__) + ": renaming " + oldPathStr + " to " + newPathStr +
                       ": " + strerror(errno));
                return; // Do not remove source if we failed!
            }
            
            // Then, remove source file (hopefully succeeds)
            if (std::remove(oldPathStr.c_str()) == -1)
            {
                LOGERR(String(__FUNCTION__) + ": renaming " + oldPathStr + " to " + newPathStr +
                       ": " + strerror(errno));
            }
        }
#   endif
    }

	Mutex FileScheduler::mMutex;
    
#   if PLATFORM_WINDOWS
    void win32_handleError(DWORD error, const WString& path)
    {
        switch (error)
        {
            case ERROR_FILE_NOT_FOUND:
                LOGERR("File at path: \"" + toString(path) + "\" not found.");
                break;
            case ERROR_PATH_NOT_FOUND:
            case ERROR_BAD_NETPATH:
            case ERROR_CANT_RESOLVE_FILENAME:
            case ERROR_INVALID_DRIVE:
                LOGERR("Path \"" + toString(path) + "\" not found.");
                break;
            case ERROR_ACCESS_DENIED:
                LOGERR("Access to path \"" + toString(path) + "\" denied.");
                break;
            case ERROR_ALREADY_EXISTS:
            case ERROR_FILE_EXISTS:
                LOGERR("File/folder at path \"" + toString(path) + "\" already exists.");
                break;
            case ERROR_INVALID_NAME:
            case ERROR_DIRECTORY:
            case ERROR_FILENAME_EXCED_RANGE:
            case ERROR_BAD_PATHNAME:
                LOGERR("Invalid path string: \"" + toString(path) + "\".");
                break;
            case ERROR_FILE_READ_ONLY:
                LOGERR("File at path \"" + toString(path) + "\" is read only.");
                break;
            case ERROR_CANNOT_MAKE:
                LOGERR("Cannot create file/folder at path: \"" + toString(path) + "\".");
                break;
            case ERROR_DIR_NOT_EMPTY:
                LOGERR("Directory at path \"" + toString(path) + "\" not empty.");
                break;
            case ERROR_WRITE_FAULT:
                LOGERR("Error while writing a file at path \"" + toString(path) + "\".");
                break;
            case ERROR_READ_FAULT:
                LOGERR("Error while reading a file at path \"" + toString(path) + "\".");
                break;
            case ERROR_SHARING_VIOLATION:
                LOGERR("Sharing violation at path \"" + toString(path) + "\".");
                break;
            case ERROR_LOCK_VIOLATION:
                LOGERR("Lock violation at path \"" + toString(path) + "\".");
                break;
            case ERROR_HANDLE_EOF:
                LOGERR("End of file reached for file at path \"" + toString(path) + "\".");
                break;
            case ERROR_HANDLE_DISK_FULL:
            case ERROR_DISK_FULL:
                LOGERR("Disk full.");
                break;
            case ERROR_NEGATIVE_SEEK:
                LOGERR("Negative seek.");
                break;
            default:
                LOGERR("Undefined file system exception: " + toString((UINT32)error));
                break;
        }
    }
    
    WString win32_getCurrentDirectory()
    {
        DWORD len = GetCurrentDirectoryW(0, NULL);
        if (len > 0)
        {
            wchar_t* buffer = (wchar_t*)Alloc(len * sizeof(wchar_t));
            
            DWORD n = GetCurrentDirectoryW(len, buffer);
            if (n > 0 && n <= len)
            {
                WString result(buffer);
                if (result[result.size() - 1] != L'\\')
                    result.append(L"\\");
                
                Free(buffer);
                return result;
            }
            
            Free(buffer);
        }
        
        return StringUtil::WBLANK;
    }
    
    WString win32_getTempDirectory()
    {
        DWORD len = GetTempPathW(0, NULL);
        if (len > 0)
        {
            wchar_t* buffer = (wchar_t*)Alloc(len * sizeof(wchar_t));
            
            DWORD n = GetTempPathW(len, buffer);
            if (n > 0 && n <= len)
            {
                WString result(buffer);
                if (result[result.size() - 1] != L'\\')
                    result.append(L"\\");
                
                Free(buffer);
                return result;
            }
            
            Free(buffer);
        }
        
        return StringUtil::WBLANK;
    }
    
    bool win32_pathExists(const WString& path)
    {
        DWORD attr = GetFileAttributesW(path.c_str());
        if (attr == 0xFFFFFFFF)
        {
            switch (GetLastError())
            {
                case ERROR_FILE_NOT_FOUND:
                case ERROR_PATH_NOT_FOUND:
                case ERROR_NOT_READY:
                case ERROR_INVALID_DRIVE:
                    return false;
                default:
                    win32_handleError(GetLastError(), path);
            }
        }
        return true;
    }
    
    bool win32_isDirectory(const WString& path)
    {
        DWORD attr = GetFileAttributesW(path.c_str());
        if (attr == 0xFFFFFFFF)
            win32_handleError(GetLastError(), path);
        
        return (attr & FILE_ATTRIBUTE_DIRECTORY) != FALSE;
    }
    
    bool win32_isDevice(const WString& path)
    {
        WString ucPath = path;
        StringUtil::toUpperCase(ucPath);
        
        return
        ucPath.compare(0, 4, L"\\\\.\\") == 0 ||
        ucPath.compare(L"CON") == 0 ||
        ucPath.compare(L"PRN") == 0 ||
        ucPath.compare(L"AUX") == 0 ||
        ucPath.compare(L"NUL") == 0 ||
        ucPath.compare(L"LPT1") == 0 ||
        ucPath.compare(L"LPT2") == 0 ||
        ucPath.compare(L"LPT3") == 0 ||
        ucPath.compare(L"LPT4") == 0 ||
        ucPath.compare(L"LPT5") == 0 ||
        ucPath.compare(L"LPT6") == 0 ||
        ucPath.compare(L"LPT7") == 0 ||
        ucPath.compare(L"LPT8") == 0 ||
        ucPath.compare(L"LPT9") == 0 ||
        ucPath.compare(L"COM1") == 0 ||
        ucPath.compare(L"COM2") == 0 ||
        ucPath.compare(L"COM3") == 0 ||
        ucPath.compare(L"COM4") == 0 ||
        ucPath.compare(L"COM5") == 0 ||
        ucPath.compare(L"COM6") == 0 ||
        ucPath.compare(L"COM7") == 0 ||
        ucPath.compare(L"COM8") == 0 ||
        ucPath.compare(L"COM9") == 0;
    }
    
    bool win32_isFile(const WString& path)
    {
        return !win32_isDirectory(path) && !win32_isDevice(path);
    }
    
    bool win32_createFile(const WString& path)
    {
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, 0, CREATE_NEW, 0, 0);
        
        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
            return true;
        }
        else if (GetLastError() == ERROR_FILE_EXISTS)
            return false;
        else
            win32_handleError(GetLastError(), path);
        
        return false;
    }
    
    bool win32_createDirectory(const WString& path)
    {
        if (win32_pathExists(path) && win32_isDirectory(path))
            return false;
        
        if (CreateDirectoryW(path.c_str(), 0) == FALSE)
            win32_handleError(GetLastError(), path);
        
        return true;
    }
    
    UINT64 win32_getFileSize(const WString& path)
    {
        WIN32_FILE_ATTRIBUTE_DATA attrData;
        if (GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &attrData) == FALSE)
            win32_handleError(GetLastError(), path);
        
        LARGE_INTEGER li;
        li.LowPart = attrData.nFileSizeLow;
        li.HighPart = attrData.nFileSizeHigh;
        return (UINT64)li.QuadPart;
    }
    
    std::time_t win32_getLastModifiedTime(const WString& path)
    {
        WIN32_FILE_ATTRIBUTE_DATA fad;
        if (GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &fad) == 0)
            win32_handleError(GetLastError(), path);
        
        ULARGE_INTEGER ull;
        ull.LowPart = fad.ftLastWriteTime.dwLowDateTime;
        ull.HighPart = fad.ftLastWriteTime.dwHighDateTime;
        
        return (std::time_t) ((ull.QuadPart / 10000000ULL) - 11644473600ULL);
    }
#   else
    bool unix_pathExists(const String& path)
    {
        struct stat st_buf;
        if (stat(path.c_str(), &st_buf) == 0)
            return true;
        else
            if (errno == ENOENT)    // No such file or directory
                return false;
            else
            {
                HANDLE_PATH_ERROR(path, errno);
                return false;
            }
    }
    
    bool unix_stat(const String& path, struct stat *st_buf)
    {
        if (stat(path.c_str(), st_buf) != 0)
        {
            HANDLE_PATH_ERROR(path, errno);
            return false;
        }
        return true;
    }
    
    bool unix_isFile(const String& path)
    {
        struct stat st_buf;
        if (unix_stat(path, &st_buf))
            return S_ISREG(st_buf.st_mode);
        
        return false;
    }
    
    bool unix_isDirectory(const String& path)
    {
        struct stat st_buf;
        if (unix_stat(path, &st_buf))
            return S_ISDIR(st_buf.st_mode);
        
        return false;
    }
    
    bool unix_createDirectory(const String& path)
    {
        if (unix_pathExists(path) && unix_isDirectory(path))
            return false;
        
        if (mkdir(path.c_str(), 0755))
        {
            HANDLE_PATH_ERROR(path, errno);
            return false;
        }
        
        return true;
    }
#   endif
}

