#pragma once

#include "Prerequisites/LSPrerequisitesUtil.h"
#include "Error/LSException.h"

namespace ls
{
	/** @addtogroup General
	 *  @{
	 */

	/** Serializable blob of raw memory. */
	struct DataBlob
	{
		UINT8* data = nullptr;
		UINT32 size = 0;
	};

	/** @} */

	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	template<> struct RTTIPlainType<DataBlob>
	{	
		enum { id = TID_DataBlob }; enum { hasDynamicSize = 1 };

		static void toMemory(const DataBlob& data, char* memory)
		{ 
			UINT32 size = getDynamicSize(data);

			memcpy(memory, &size, sizeof(UINT32));
			memory += sizeof(UINT32);

			memcpy(memory, data.data, data.size);
		}

		static UINT32 fromMemory(DataBlob& data, char* memory)
		{ 
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32)); 
			memory += sizeof(UINT32);

			if(data.data != nullptr)
				ls_free(data.data);

			data.size = size - sizeof(UINT32);
			data.data = (UINT8*)ls_alloc(data.size);
			memcpy(data.data, memory, data.size);

			return size;
		}

		static UINT32 getDynamicSize(const DataBlob& data)
		{ 
			UINT64 dataSize = data.size + sizeof(UINT32);
				
#if DEBUG_MODE
			if(dataSize > std::numeric_limits<UINT32>::max())
			{
				LS_EXCEPT(InternalErrorException, "Data overflow! Size doesn't fit into 32 bits.");
			}
#endif

			return (UINT32)dataSize;
		}	
	}; 

	/** @} */
	/** @endcond */
}