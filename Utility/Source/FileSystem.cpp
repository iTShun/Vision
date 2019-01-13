#include "FileSystem.h"
#include "Debug.h"

namespace Utility
{
    SPtr<DataStream> FileSystem::openFile(const Path& path, bool readOnly)
    {
        return nullptr;
    }
    
    SPtr<DataStream> FileSystem::createAndOpenFile(const Path& path)
    {
        return nullptr;
    }
    
    UINT64 FileSystem::getFileSize(const Path& path)
    {
        return 0;
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
        
    }
    
    bool FileSystem::exists(const Path& path)
    {
        return false;
    }
    
    bool FileSystem::isFile(const Path& path)
    {
        return false;
    }
    
    bool FileSystem::isDirectory(const Path& path)
    {
        return false;
    }
    
    void FileSystem::getChildren(const Path& dirPath, Vector<Path>& files, Vector<Path>& directories)
    {
        
    }
    
    bool FileSystem::iterate(const Path& dirPath, std::function<bool(const Path&)> fileCallback,
                             std::function<bool(const Path&)> dirCallback, bool recursive)
    {
        return false;
    }
    
    std::time_t FileSystem::getLastModifiedTime(const Path& path)
    {
        std::time_t time = 0;
        
        return time;
    }
    
    Path FileSystem::getWorkingDirectoryPath()
    {
        return Path("");
    }
    
    Path FileSystem::getTempDirectoryPath()
    {
        return Path("");
    }
    
    void FileSystem::copyFile(const Path& source, const Path& destination)
    {
        
    }
    
    void FileSystem::removeFile(const Path& path)
    {
        
    }
    
    void FileSystem::moveFile(const Path& oldPath, const Path& newPath)
    {
        
    }

	Mutex FileScheduler::mMutex;
}

