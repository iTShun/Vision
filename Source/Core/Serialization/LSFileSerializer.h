#pragma once

#include "Platform/LSPlatform.h"

namespace ls
{
	/** @addtogroup Serialization
	 *  @{
	 */

	/** Encodes the provided object to the specified file using the RTTI system. */
	class FileEncoder
	{
		public:
		FileEncoder(const Path& fileLocation);
		~FileEncoder();

		/**
		 * Parses the provided object, serializes all of its data as specified by its RTTIType and saves the serialized 
		 * data to the provided file location.
		 *
		 * @param[in]	object		Object to encode.
		 * @param[in]	context		Optional object that will be passed along to all serialized objects through
		 *							their serialization callbacks. Can be used for controlling serialization, 
		 *							maintaining state or sharing information between objects during 
		 *							serialization.
		 */
		void encode(IReflectable* object, SerializationContext* context = nullptr);

	private:
		/** Called by the binary serializer whenever the buffer gets full. */
		UINT8* flushBuffer(UINT8* bufferStart, UINT32 bytesWritten, UINT32& newBufferSize);

		std::ofstream mOutputStream;
		UINT8* mWriteBuffer = nullptr;

		static const UINT32 WRITE_BUFFER_SIZE = 2048;
	};

	/** Decodes objects from the specified file using the RTTI system. */
	class FileDecoder
	{
	public:
		FileDecoder(const Path& fileLocation);

		/**	
		 * Deserializes an IReflectable object by reading the binary data at the provided file location. 
		 *
		 * @param[in]	context		Optional object that will be passed along to all deserialized objects through
		 *							their deserialization callbacks. Can be used for controlling deserialization, 
		 *							maintaining state or sharing information between objects during 
		 *							deserialization.
		 */
		SPtr<IReflectable> decode(SerializationContext* context = nullptr);

		/** Skips over than object in the file. Calling decode() will decode the next object. */
		void skip();

	private:
		SPtr<DataStream> mInputStream;
	};

	/** @} */
}
