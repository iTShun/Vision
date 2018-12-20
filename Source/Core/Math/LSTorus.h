#pragma once

#include "Platform/LSPlatform.h"
#include "Math/LSVector3.h"

namespace ls
{
	/** @addtogroup Math
	 *  @{
	 */

	/**
	 * Represents a torus at the world center. Outer radius represents the distance from the center, and inner radius 
	 * represents the radius of the tube. Inner radius must be less or equal than the outer radius.
	 */
	class Torus
	{
	public:
		Torus() = default;

		Torus(const Vector3& normal, float outerRadius, float innerRadius)
			:normal(normal), outerRadius(outerRadius), innerRadius(innerRadius)
		{ }

		/** Ray/torus intersection, returns boolean result and distance to nearest intersection point. */
		std::pair<bool, float> intersects(const Ray& ray) const;

		Vector3 normal{LSZero};
		float outerRadius = 0.0f;
		float innerRadius = 0.0f;
	};

	/** @} */
}
