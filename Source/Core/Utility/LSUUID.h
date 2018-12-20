#pragma once

#include "Platform/LSTypes.h"
#include "Allocators/LSMemoryAllocator.h"
#include "Utility/LSUtil.h"

namespace ls
{
	/** @addtogroup Utility-Core
	*  @{
	*/

	/** Represents a universally unique identifier. */
	struct UUID
	{
		/** Initializes an empty UUID. */
		constexpr UUID() = default;

		/** Initializes an UUID using LSEngine's UUID representation. */
		constexpr UUID(UINT32 data1, UINT32 data2, UINT32 data3, UINT32 data4)
			: mData{ data1, data2, data3, data4 }
		{ }

		/** Initializes an UUID using its string representation. */
		explicit UUID(const String& uuid);

		constexpr bool operator==(const UUID& rhs) const
		{
			return mData[0] == rhs.mData[0] && mData[1] == rhs.mData[1] && mData[2] == rhs.mData[2] && mData[3] == rhs.mData[3];
		}

		constexpr bool operator!=(const UUID& rhs) const
		{
			return !(*this == rhs);
		}

		constexpr bool operator<(const UUID& rhs) const
		{
			for (UINT32 i = 0; i < 4; i++)
			{
				if (mData[i] < rhs.mData[i])
					return true;
				else if (mData[i] > rhs.mData[i])
					return false;

				// Move to next element if equal
			}

			// They're equal
			return false;
		}

		/** Checks has the UUID been initialized to a valid value. */
		constexpr bool empty() const
		{
			return mData[0] == 0 && mData[1] == 0 && mData[2] == 0 && mData[3] == 0;
		}

		/** Converts the UUID into its string representation. */
		String toString() const;

		static UUID EMPTY;
	private:
		friend struct std::hash<UUID>;

		UINT32 mData[4] = { 0, 0, 0, 0 };
	};

	//BS_ALLOW_MEMCPY_SERIALIZATION(UUID) Todo

	/**
	* Utility class for generating universally unique identifiers.
	*
	* @note	Thread safe.
	*/
	class UUIDGenerator
	{
	public:
		/**	Generate a new random universally unique identifier. */
		static UUID generateRandom();
	};

	/** @} */
}

/** @cond STDLIB */
/** @addtogroup Utility
*  @{
*/

namespace std
{
	/**	Hash value generator for UUID. */
	template<>
	struct hash<ls::UUID>
	{
		size_t operator()(const ls::UUID& value) const
		{
			size_t hash = 0;
			ls::hash_combine(hash, value.mData[0]);
			ls::hash_combine(hash, value.mData[1]);
			ls::hash_combine(hash, value.mData[2]);
			ls::hash_combine(hash, value.mData[3]);

			return hash;
		}
	};
}

/** @} */
/** @endcond */
