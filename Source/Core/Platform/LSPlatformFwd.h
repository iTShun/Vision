#pragma once

namespace ls
{
	/** @addtogroup Math
	*  @{
	*/

	/** Values that represent in which order are euler angles applied when used in transformations. */
	enum class EulerAngleOrder
	{
		XYZ,
		XZY,
		YXZ,
		YZX,
		ZXY,
		ZYX
	};

	/** Enum used for object construction specifying the object should be zero initialized. */
	enum LS_ZERO { LSZero };

	/** Enum used for matrix/quaternion constructor specifying it should be initialized with an identity value. */
	enum LS_IDENTITY { LSIdentity };

	/** @} */
}

namespace ls
{
	// Rtti
	class IReflectable;
	class RTTITypeBase;
	struct RTTIField;
	// Serialization
	struct SerializationContext;
	struct SerializedInstance;
	struct SerializedObject;
	// FileSystem
	class DataStream;
	class Path;

	// Math
	class AABox;
	class Bounds;
	class Capsule;
	class Color;
	class Degree;
	class Math;
	class Matrix3;
	class Matrix4;
	class Plane;
	class Quaternion;
	class Radian;
	class Ray;
	class Rect2;
	class Rect2I;
	class Rect3;
	class Sphere;
	class Vector2;
	struct Vector2I;
	class Vector3;
	class Vector4;

	enum TypeID_Utility
	{
		TID_Abstract = 50, // Special type ID used for Abstract classes. Only type ID that may be used by more than one class.
		TID_WString = 51,
		TID_Path = 52,
		TID_Vector = 53,
		TID_Map = 54,
		TID_UnorderedMap = 55,
		TID_Pair = 56,
		TID_Set = 57,
		TID_StringID = 58,
		TID_SerializedInstance = 59,
		TID_SerializedField = 60,
		TID_SerializedObject = 61,
		TID_SerializedArray = 62,
		TID_SerializedEntry = 63,
		TID_SerializedArrayEntry = 64,
		TID_SerializedSubObject = 65,
		TID_UnorderedSet = 66,
		TID_SerializedDataBlock = 67,
		TID_Flags = 68,
		TID_IReflectable = 69,
		TID_DataBlob = 70,
		TID_ColorGradient = 71,
		TID_SerializationContext = 72
	};
}
