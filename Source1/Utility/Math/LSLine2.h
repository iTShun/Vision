#pragma once

#include "Prerequisites/LSPrerequisitesUtil.h"
#include "Math/LSVector2.h"

namespace ls 
{
	/** @addtogroup Math
	 *  @{
	 */

	/** A line in 2D space represented with an origin and direction. */
	class LS_UTILITY_EXPORT Line2
	{
	public:
		Line2() = default;

		Line2(const Vector2& origin, const Vector2& direction)
			:mOrigin(origin), mDirection(direction) 
		{ }

		void setOrigin(const Vector2& origin) { mOrigin = origin; } 
		const Vector2& getOrigin(void) const { return mOrigin; } 

		void setDirection(const Vector2& dir) { mDirection = dir; } 
		const Vector2& getDirection(void) const {return mDirection;} 

		/** Gets the position of a point t units along the line. */
		Vector2 getPoint(float t) const 
		{ 
			return Vector2(mOrigin + (mDirection * t));
		}
		
		/** Gets the position of a point t units along the line. */
		Vector2 operator*(float t) const 
		{ 
			return getPoint(t);
		}

		/** Line/Line intersection, returns boolean result and distance to intersection point. */
		std::pair<bool, float> intersects(const Line2& line) const;

	protected:
		Vector2 mOrigin = Vector2::ZERO;
		Vector2 mDirection = Vector2::UNIT_X;
	};

	/** @} */
}
