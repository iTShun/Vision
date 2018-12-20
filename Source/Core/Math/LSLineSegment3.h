#pragma once

#include "Platform/LSPlatform.h"
#include "Math/LSVector3.h"

namespace ls
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Represents a line segment in three dimensional space defined by a start and an end point. */
	class LineSegment3
	{
	public:
		LineSegment3() = default;
		LineSegment3(const Vector3& start, const Vector3& end);

		/**
		 * Find the nearest point on the line segment and the provided ray.
		 *
		 * @return	Set of nearest points and distance from the points. First nearest point is a point along the ray, 
		 *			while the second is along the line segment.
		 *
		 * @note	If segment and ray are parallel the set of points at the segment origin are returned.
		 */
		std::pair<std::array<Vector3, 2>, float> getNearestPoint(const Ray& ray) const;

		/** Returns the length of the line segment. */
		float getLength() const { return start.distance(end); }

		/** Returns the center point along the line segment. */
		Vector3 getCenter() const { return start + (end - start) * 0.5f; }

		Vector3 start = LSZero;
		Vector3 end = LSZero;
	};

	/** @} */
}
