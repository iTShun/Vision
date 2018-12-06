#include "Private/UnitTests/LSFileSystemTestSuite.h"

#include "Logger/LSLogger.h"
#include "Error/LSException.h"
#include "FileSystem/LSFileSystem.h"

#include <algorithm>
#include <fstream>

namespace ls
{
	const String testDirectoryName = "FileSystemTestDirectory/";

	void createFile(Path path, String content)
	{
		std::ofstream fs;
		fs.open(path.toPlatformString().c_str());
		fs << content;
		fs.close();
	}

	void createEmptyFile(Path path)
	{
		createFile(path, "");
	}

	String readFile(Path path)
	{
		String content;
		std::ifstream fs;
		fs.open(path.toPlatformString().c_str());
		fs >> content;
		fs.close();
		return content;
	}

	void FileSystemTestSuite::startUp()
	{
		mTestDirectory = FileSystem::getWorkingDirectoryPath() + testDirectoryName;
		if (FileSystem::exists(mTestDirectory))
		{
			LS_EXCEPT(InternalErrorException,
			          String("Directory '") + testDirectoryName
			          + "' should not already exist; you should remove it manually.");
		}
		else
		{
			FileSystem::createDir(mTestDirectory);
			LS_TEST_ASSERT_MSG(FileSystem::exists(mTestDirectory), "FileSystemTestSuite::startUp(): test directory creation failed");
		}
	}

	void FileSystemTestSuite::shutDown()
	{
		FileSystem::remove(mTestDirectory, true);
		if (FileSystem::exists(mTestDirectory))
		{
			LOGERR("FileSystemTestSuite failed to delete '" + mTestDirectory.toString()
				   + "', you should remove it manually.");
		}
	}

	FileSystemTestSuite::FileSystemTestSuite()
	{
		LS_ADD_TEST(FileSystemTestSuite::testExists_yes_file);
		LS_ADD_TEST(FileSystemTestSuite::testExists_yes_dir);
		LS_ADD_TEST(FileSystemTestSuite::testExists_no);
		LS_ADD_TEST(FileSystemTestSuite::testGetFileSize_zero);
		LS_ADD_TEST(FileSystemTestSuite::testGetFileSize_not_zero);
		LS_ADD_TEST(FileSystemTestSuite::testIsFile_yes);
		LS_ADD_TEST(FileSystemTestSuite::testIsFile_no);
		LS_ADD_TEST(FileSystemTestSuite::testIsDirectory_yes);
		LS_ADD_TEST(FileSystemTestSuite::testIsDirectory_no);
		LS_ADD_TEST(FileSystemTestSuite::testRemove_file);
		LS_ADD_TEST(FileSystemTestSuite::testRemove_directory);
		LS_ADD_TEST(FileSystemTestSuite::testMove);
		LS_ADD_TEST(FileSystemTestSuite::testMove_overwrite_existing);
		LS_ADD_TEST(FileSystemTestSuite::testMove_no_overwrite_existing);
		LS_ADD_TEST(FileSystemTestSuite::testCopy);
		LS_ADD_TEST(FileSystemTestSuite::testCopy_overwrite_existing);
		LS_ADD_TEST(FileSystemTestSuite::testCopy_no_overwrite_existing);
		LS_ADD_TEST(FileSystemTestSuite::testGetChildren);
		LS_ADD_TEST(FileSystemTestSuite::testGetLastModifiedTime);
		LS_ADD_TEST(FileSystemTestSuite::testGetTempDirectoryPath);
	}

	void FileSystemTestSuite::testExists_yes_file()
	{
		Path path = mTestDirectory + "plop";
		createEmptyFile(path);
		LS_TEST_ASSERT(FileSystem::exists(path));
		FileSystem::remove(path);
	}

	void FileSystemTestSuite::testExists_yes_dir()
	{
		Path path = mTestDirectory + "plop/";
		FileSystem::createDir(path);
		LS_TEST_ASSERT(FileSystem::exists(path));
		FileSystem::remove(path);
	}

	void FileSystemTestSuite::testExists_no()
	{
		LS_TEST_ASSERT(!FileSystem::exists(Path("this-file-does-not-exist")));
	}

	void FileSystemTestSuite::testGetFileSize_zero()
	{
		Path path = mTestDirectory + "file-size-test-1";
		createEmptyFile(path);
		LS_TEST_ASSERT(FileSystem::getFileSize(path) == 0);
		FileSystem::remove(path);
	}

	void FileSystemTestSuite::testGetFileSize_not_zero()
	{
		Path path = mTestDirectory + "file-size-test-2";
		createFile(path, "0123456789");
		LS_TEST_ASSERT(FileSystem::getFileSize(path) == 10);
		FileSystem::remove(path);
	}

	void FileSystemTestSuite::testIsFile_yes()
	{
		Path path = mTestDirectory + "some-file-1";
		createEmptyFile(path);
		LS_TEST_ASSERT(FileSystem::isFile(path));
	}

	void FileSystemTestSuite::testIsFile_no()
	{
		Path path = mTestDirectory + "some-directory-1/";
		FileSystem::createDir(path);
		LS_TEST_ASSERT(!FileSystem::isFile(path));
	}

	void FileSystemTestSuite::testIsDirectory_yes()
	{
		Path path = mTestDirectory + "some-directory-2/";
		FileSystem::createDir(path);
		LS_TEST_ASSERT(FileSystem::isDirectory(path));
	}

	void FileSystemTestSuite::testIsDirectory_no()
	{
		Path path = mTestDirectory + "some-file-2";
		createEmptyFile(path);
		LS_TEST_ASSERT(!FileSystem::isDirectory(path));
	}

	void FileSystemTestSuite::testRemove_file()
	{
		Path path = mTestDirectory + "file-to-remove";
		createEmptyFile(path);
		LS_TEST_ASSERT(FileSystem::exists(path));
		FileSystem::remove(path);
		LS_TEST_ASSERT(!FileSystem::exists(path));
	}

	void FileSystemTestSuite::testRemove_directory()
	{
		Path path = mTestDirectory + "directory-to-remove/";
		FileSystem::createDir(path);
		LS_TEST_ASSERT(FileSystem::exists(path));
		FileSystem::remove(path, true);
		LS_TEST_ASSERT(!FileSystem::exists(path));
	}

	void FileSystemTestSuite::testMove()
	{
		Path source = mTestDirectory + "move-source-1";
		Path destination = mTestDirectory + "move-destination-1";
		createFile(source, "move-data-source-1");
		LS_TEST_ASSERT(FileSystem::exists(source));
		LS_TEST_ASSERT(!FileSystem::exists(destination));
		FileSystem::move(source, destination);
		LS_TEST_ASSERT(!FileSystem::exists(source));
		LS_TEST_ASSERT(FileSystem::exists(destination));
		LS_TEST_ASSERT(readFile(destination) == "move-data-source-1");
	}

	void FileSystemTestSuite::testMove_overwrite_existing()
	{
		Path source = mTestDirectory + "move-source-2";
		Path destination = mTestDirectory + "move-destination-2";
		createFile(source, "move-data-source-2");
		createFile(destination, "move-data-destination-2");
		LS_TEST_ASSERT(FileSystem::exists(source));
		LS_TEST_ASSERT(FileSystem::exists(destination));
		FileSystem::move(source, destination, true);
		LS_TEST_ASSERT(!FileSystem::exists(source));
		LS_TEST_ASSERT(FileSystem::exists(destination));
		LS_TEST_ASSERT(readFile(destination) == "move-data-source-2");
	}

	void FileSystemTestSuite::testMove_no_overwrite_existing()
	{
		Path source = mTestDirectory + "move-source-3";
		Path destination = mTestDirectory + "move-destination-3";
		createFile(source, "move-data-source-3");
		createFile(destination, "move-data-destination-3");
		LS_TEST_ASSERT(FileSystem::exists(source));
		LS_TEST_ASSERT(FileSystem::exists(destination));
		FileSystem::move(source, destination, false);
		LS_TEST_ASSERT(FileSystem::exists(source));
		LS_TEST_ASSERT(FileSystem::exists(destination));
		LS_TEST_ASSERT(readFile(destination) == "move-data-destination-3");
	}

	void FileSystemTestSuite::testCopy()
	{
		Path source = mTestDirectory + "copy-source-1";
		Path destination = mTestDirectory + "copy-destination-1";
		createFile(source, "copy-data-source-1");
		LS_TEST_ASSERT(FileSystem::exists(source));
		LS_TEST_ASSERT(!FileSystem::exists(destination));
		FileSystem::copy(source, destination);
		LS_TEST_ASSERT(FileSystem::exists(source));
		LS_TEST_ASSERT(FileSystem::exists(destination));
		LS_TEST_ASSERT(readFile(source) == "copy-data-source-1");
		LS_TEST_ASSERT(readFile(destination) == "copy-data-source-1");
	}

	void FileSystemTestSuite::testCopy_overwrite_existing()
	{
		Path source = mTestDirectory + "copy-source-2";
		Path destination = mTestDirectory + "copy-destination-2";
		createFile(source, "copy-data-source-2");
		createFile(destination, "copy-data-destination-2");
		LS_TEST_ASSERT(FileSystem::exists(source));
		LS_TEST_ASSERT(FileSystem::exists(destination));
		FileSystem::copy(source, destination, true);
		LS_TEST_ASSERT(FileSystem::exists(source));
		LS_TEST_ASSERT(FileSystem::exists(destination));
		LS_TEST_ASSERT(readFile(source) == "copy-data-source-2");
		LS_TEST_ASSERT(readFile(destination) == "copy-data-source-2");
	}

	void FileSystemTestSuite::testCopy_no_overwrite_existing()
	{
		Path source = mTestDirectory + "copy-source-3";
		Path destination = mTestDirectory + "copy-destination-3";
		createFile(source, "copy-data-source-3");
		createFile(destination, "copy-data-destination-3");
		LS_TEST_ASSERT(FileSystem::exists(source));
		LS_TEST_ASSERT(FileSystem::exists(destination));
		FileSystem::copy(source, destination, false);
		LS_TEST_ASSERT(FileSystem::exists(source));
		LS_TEST_ASSERT(FileSystem::exists(destination));
		LS_TEST_ASSERT(readFile(source) == "copy-data-source-3");
		LS_TEST_ASSERT(readFile(destination) == "copy-data-destination-3");
	}


#define CONTAINS(v, e) (std::find(v.begin(), v.end(), e) != v.end())

	void FileSystemTestSuite::testGetChildren()
	{
		Path path = mTestDirectory + "get-children-test/";
		FileSystem::createDir(path);
		FileSystem::createDir(path + "foo/");
		FileSystem::createDir(path + "bar/");
		FileSystem::createDir(path + "baz/");
		createEmptyFile(path + "ga");
		createEmptyFile(path + "bu");
		createEmptyFile(path + "zo");
		createEmptyFile(path + "meu");
		Vector<Path> files, directories;
		FileSystem::getChildren(path, files, directories);
		LS_TEST_ASSERT(files.size() == 4);
		LS_TEST_ASSERT(CONTAINS(files, path + "ga"));
		LS_TEST_ASSERT(CONTAINS(files, path + "bu"));
		LS_TEST_ASSERT(CONTAINS(files, path + "zo"));
		LS_TEST_ASSERT(CONTAINS(files, path + "meu"));
		LS_TEST_ASSERT(directories.size() == 3);
		LS_TEST_ASSERT(CONTAINS(directories, path + "foo"));
		LS_TEST_ASSERT(CONTAINS(directories, path + "bar"));
		LS_TEST_ASSERT(CONTAINS(directories, path + "baz"));
	}

	void FileSystemTestSuite::testGetLastModifiedTime()
	{
		std::time_t beforeTime;
		time(&beforeTime);

		Path path = mTestDirectory + "blah1234";
		createFile(path, "blah");
		std::time_t mtime = FileSystem::getLastModifiedTime(path);
		LS_TEST_ASSERT(mtime >= beforeTime);
		LS_TEST_ASSERT(mtime <= beforeTime + 10);
	}

	void FileSystemTestSuite::testGetTempDirectoryPath()
	{
		Path path = FileSystem::getTempDirectoryPath();
		/* No judging. */
		LS_TEST_ASSERT(!path.toString().empty());
	}
}
