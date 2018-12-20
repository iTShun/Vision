#include "Reflection/LSRTTIField.h"

namespace ls
{
	void RTTIField::checkIsPlain(bool array)
	{
		if(!isPlainType())
		{
			assert(false && "Invalid field type.");
		}

		checkIsArray(array);
	}

	void RTTIField::checkIsDataBlock()
	{
		if(!isDataBlockType())
		{
			assert(false && "Invalid field type.");
		}
	}

	void RTTIField::checkIsComplex(bool array)
	{
		if(!isReflectableType())
		{
			assert(false && "Invalid field type.");
		}

		checkIsArray(array);
	}

	void RTTIField::checkIsComplexPtr(bool array)
	{
		if(!isReflectablePtrType())
		{
			assert(false && "Invalid field type.");
		}

		checkIsArray(array);
	}

	void RTTIField::checkIsArray(bool array)
	{
		if(array && !mIsVectorType)
		{
			assert(false && "Invalid field type. Needed an array type but got a single type.");
		}

		if(!array && mIsVectorType)
		{
			assert(false && "Invalid field type. Needed a single type but got an array type.");
		}
	}
}