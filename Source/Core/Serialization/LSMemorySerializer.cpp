#include "Serialization/LSMemorySerializer.h"
#include "Reflection/LSIReflectable.h"
#include "Serialization/LSBinarySerializer.h"
#include "FileSystem/LSDataStream.h"

using namespace std::placeholders;

namespace ls
{
	UINT8* MemorySerializer::encode(IReflectable* object, UINT32& bytesWritten, 
		std::function<void*(UINT32)> allocator, bool shallow, SerializationContext* context)
	{
		using namespace std::placeholders;

		BinarySerializer bs;

		BufferPiece piece;
		piece.buffer = (UINT8*)ls_alloc(WRITE_BUFFER_SIZE);
		piece.size = 0;

		mBufferPieces.push_back(piece);

		bs.encode(object, piece.buffer, WRITE_BUFFER_SIZE, &bytesWritten, 
			std::bind(&MemorySerializer::flushBuffer, this, _1, _2, _3), shallow, context);

		UINT8* resultBuffer;
		if(allocator != nullptr)
			resultBuffer = (UINT8*)allocator(bytesWritten);
		else
			resultBuffer = (UINT8*)ls_alloc(bytesWritten);

		UINT32 offset = 0;
		for(auto iter = mBufferPieces.begin(); iter != mBufferPieces.end(); ++iter)
		{
			if(iter->size > 0)
			{
				memcpy(resultBuffer + offset, iter->buffer, iter->size);
				offset += iter->size;
			}
		}

		for(auto iter = mBufferPieces.rbegin(); iter != mBufferPieces.rend(); ++iter)
		{
			ls_free(iter->buffer);
		}

		mBufferPieces.clear();

		return resultBuffer;
	}

	SPtr<IReflectable> MemorySerializer::decode(UINT8* buffer, UINT32 bufferSize, SerializationContext* context)
	{
		SPtr<MemoryDataStream> stream = ls_shared_ptr_new<MemoryDataStream>(buffer, bufferSize, false);

		BinarySerializer bs;
		SPtr<IReflectable> object = bs.decode(stream, (UINT32)bufferSize, context);

		return object;
	}

	UINT8* MemorySerializer::flushBuffer(UINT8* bufferStart, UINT32 bytesWritten, UINT32& newBufferSize)
	{
		mBufferPieces.back().size = bytesWritten;

		BufferPiece piece;
		piece.buffer = (UINT8*)ls_alloc(WRITE_BUFFER_SIZE);
		piece.size = 0;

		mBufferPieces.push_back(piece);

		return piece.buffer;
	}
}
