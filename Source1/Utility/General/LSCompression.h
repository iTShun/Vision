#pragma once

#include "Prerequisites/LSPrerequisitesUtil.h"

namespace ls
{
	/** @addtogroup General
	 *  @{
	 */

	/** Performs generic compression and decompression on raw data. */
	class LS_UTILITY_EXPORT Compression
	{
	public:
		/** Compresses the data from the provided data stream and outputs the new stream with compressed data. */
		static SPtr<MemoryDataStream> compress(SPtr<DataStream>& input);

		/** Decompresses the data from the provided data stream and outputs the new stream with decompressed data. */
		static SPtr<MemoryDataStream> decompress(SPtr<DataStream>& input);
	};

	/** @} */
}