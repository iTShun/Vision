#pragma once

#include "Prerequisites/LSPrerequisitesUtil.h"
#include "Math/LSVector3.h"
#include "Math/LSAABox.h"
#include "Math/LSSphere.h"
#include "Math/LSMatrix4.h"

namespace ls
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Bounds represented by an axis aligned box and a sphere. */
	class LS_UTILITY_EXPORT Bounds
	{
	public:
		Bounds() = default;
		Bounds(const AABox& box, const Sphere& sphere);
		~Bounds() = default;

		/** Returns the axis aligned box representing the bounds. */
		const AABox& getBox() const { return mBox; }

		/** Returns the sphere representing the bounds. */
		const Sphere& getSphere() const { return mSphere; }

		/** Updates the bounds by setting the new bounding box and sphere. */
		void setBounds(const AABox& box, const Sphere& sphere);

		/** Merges the two bounds, creating a new bounds that encapsulates them both. */
		void merge(const Bounds& rhs);

		/** Expands the bounds so it includes the provided point. */
		void merge(const Vector3& point);

		/** 
		 * Transforms the bounds by the given matrix.
		 *
		 * @note	
		 * As the resulting box will no longer be axis aligned, an axis align box
		 * is instead created by encompassing the transformed oriented bounding box.
		 * Retrieving the value as an actual OBB would provide a tighter fit.
		 */
		void transform(const Matrix4& matrix);

		/**
		 * Transforms the bounds by the given matrix.
		 *
		 * @note	
		 * As the resulting box will no longer be axis aligned, an axis align box
		 * is instead created by encompassing the transformed oriented bounding box.
		 * Retrieving the value as an actual OBB would provide a tighter fit.
		 *
		 * @note
		 * Provided matrix must be affine.
		 */
		void transformAffine(const Matrix4& matrix);

	protected:
		AABox mBox;
		Sphere mSphere;
	};

	/** @} */

	LS_ALLOW_MEMCPY_SERIALIZATION(Bounds)
}
