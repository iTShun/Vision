#include "Reflection/LSRTTIField.h"
#include "Error/LSException.h"

namespace ls
{
	void RTTIField::checkIsPlain(bool array)
	{
		if(!isPlainType())
		{
            LS_EXCEPT(InternalErrorException,
                      "Invalid field type. Needed: Plain type. Got: " + toString(mIsVectorType) + ", " +
                      toString(isPlainType()) + ", " + toString(isReflectableType()) + ", " + toString(isDataBlockType()) + ", " + toString(isReflectablePtrType()));
		}

		checkIsArray(array);
	}

	void RTTIField::checkIsDataBlock()
	{
		if(!isDataBlockType())
		{
            LS_EXCEPT(InternalErrorException,
                      "Invalid field type. Needed: Data block. Got: " + toString(mIsVectorType) + ", " +
                      toString(isPlainType()) + ", " + toString(isReflectableType()) + ", " + toString(isDataBlockType()) + ", " + toString(isReflectablePtrType()));
		}
	}

	void RTTIField::checkIsComplex(bool array)
	{
		if(!isReflectableType())
		{
            LS_EXCEPT(InternalErrorException,
                      "Invalid field type. Needed: Complex type. Got: " + toString(mIsVectorType) + ", " +
                      toString(isPlainType()) + ", " + toString(isReflectableType()) + ", " + toString(isDataBlockType()) + ", " + toString(isReflectablePtrType()));
		}

		checkIsArray(array);
	}

	void RTTIField::checkIsComplexPtr(bool array)
	{
		if(!isReflectablePtrType())
		{
            LS_EXCEPT(InternalErrorException,
                      "Invalid field type. Needed: Complex ptr type. Got: " + toString(mIsVectorType) + ", " +
                      toString(isPlainType()) + ", " + toString(isReflectableType()) + ", " + toString(isDataBlockType()) + ", " + toString(isReflectablePtrType()));
		}

		checkIsArray(array);
	}

	void RTTIField::checkIsArray(bool array)
	{
		if(array && !mIsVectorType)
		{
            LS_EXCEPT(InternalErrorException,
                      "Invalid field type. Needed an array type but got a single type.");
		}

		if(!array && mIsVectorType)
		{
            LS_EXCEPT(InternalErrorException,
                      "Invalid field type. Needed a single type but got an array type.");
		}
	}
}
