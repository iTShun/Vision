#include "Serialization/LSFileSerializer.h"
#include "Reflection/LSIReflectable.h"
#include "Serialization/LSBinarySerializer.h"
#include "FileSystem/LSFileSystem.h"
#include "FileSystem/LSDataStream.h"
#include <numeric>

using namespace std::placeholders;

namespace ls
{
	FileEncoder::FileEncoder(const Path& fileLocation)
	{
		mWriteBuffer = (UINT8*)ls_alloc(WRITE_BUFFER_SIZE);

		Path parentDir = fileLocation.getDirectory();
		if (!FileSystem::exists(parentDir))
			FileSystem::createDir(parentDir);
		
		mOutputStream.open(fileLocation.toPlatformString().c_str(), std::ios::out | std::ios::binary);
		if (mOutputStream.fail())
		{
			printf("Failed to save file: \"%s\". Error: %s.", fileLocation.toString().c_str(), strerror(errno));
		}
	}

	FileEncoder::~FileEncoder()
	{
		ls_free(mWriteBuffer);

		mOutputStream.close();
		mOutputStream.clear();
	}

	void FileEncoder::encode(IReflectable* object, SerializationContext* context)
	{
		if (object == nullptr)
			return;

		UINT64 curPos = (UINT64)mOutputStream.tellp();
		mOutputStream.seekp(sizeof(UINT32), std::ios_base::cur);

		BinarySerializer bs;
		UINT32 totalBytesWritten = 0;
		bs.encode(object, mWriteBuffer, WRITE_BUFFER_SIZE, &totalBytesWritten, 
			std::bind(&FileEncoder::flushBuffer, this, _1, _2, _3), false, context);

		mOutputStream.seekp(curPos);
		mOutputStream.write((char*)&totalBytesWritten, sizeof(totalBytesWritten));
		mOutputStream.seekp(totalBytesWritten, std::ios_base::cur);
	}

	UINT8* FileEncoder::flushBuffer(UINT8* bufferStart, UINT32 bytesWritten, UINT32& newBufferSize)
	{
		mOutputStream.write((const char*)bufferStart, bytesWritten);

		return bufferStart;
	}

	FileDecoder::FileDecoder(const Path& fileLocation)
	{
		mInputStream = FileSystem::openFile(fileLocation, true);

		if (mInputStream == nullptr)
			return;

		if (mInputStream->size() > std::numeric_limits<UINT32>::max())
		{
			assert(false && "File size is larger that UINT32 can hold. Ask a programmer to use a bigger data type.");
		}
	}

	SPtr<IReflectable> FileDecoder::decode(SerializationContext* context)
	{
		if (mInputStream->eof())
			return nullptr;

		UINT32 objectSize = 0;
		mInputStream->read(&objectSize, sizeof(objectSize));

		BinarySerializer bs;
		SPtr<IReflectable> object = bs.decode(mInputStream, objectSize, context);

		return object;
	}

	void FileDecoder::skip()
	{
		if (mInputStream->eof())
			return;

		UINT32 objectSize = 0;
		mInputStream->read(&objectSize, sizeof(objectSize));
		mInputStream->skip(objectSize);
	}
}
