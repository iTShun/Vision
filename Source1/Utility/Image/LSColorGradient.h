#pragma once

#include "Prerequisites/LSPrerequisitesUtil.h"
#include "Math/LSColor.h"
#include "Allocators/LSPoolAlloc.h"

namespace ls 
{
	/** @addtogroup Image
	 *  @{
	 */

	/** Single key in a ColorGradient. */
	struct LS_SCRIPT_EXPORT(m:Image,pl:true) ColorGradientKey
	{
		ColorGradientKey() = default;
		ColorGradientKey(const Color& color, float time)
			:color(color), time(time)
		{ }

		Color color;
		float time = 0.0f;
	};

	/** 
	 * Represents a range of color values over some parameters, similar to a curve. Internally represented as a set of
	 * keys that get interpolated between.
	 */
	class LS_UTILITY_EXPORT LS_SCRIPT_EXPORT(m:Image) ColorGradient
	{
		static constexpr UINT32 MAX_KEYS = 8;
	public:
		LS_SCRIPT_EXPORT()
		ColorGradient() = default;

		LS_SCRIPT_EXPORT()
		ColorGradient(const Color& color);

		LS_SCRIPT_EXPORT()
		ColorGradient(const Vector<ColorGradientKey>& keys);

		/** Evaluates a color at the specified @p t. */
		RGBA evaluate(float t) const;

		/** Keys that control the gradient, sorted by time from first to last. Key times should be in range [0, 1]. */
		LS_SCRIPT_EXPORT()
		void setKeys(const Vector<ColorGradientKey>& keys, float duration = 1.0f);

		/** @copydoc setKeys */
		LS_SCRIPT_EXPORT()
		Vector<ColorGradientKey> getKeys() const;

		/** Specify a "gradient" that represents a single color value. */
		LS_SCRIPT_EXPORT()
		void setConstant(const Color& color);

		/** 
		 * Returns the duration over which the gradient values are interpolated over. Corresponds to the time value of the
		 * final keyframe.
		 */
		float getDuration() const { return mDuration; }

		/** Returns the time of the first and last keyframe in the gradient. */
		std::pair<float, float> getTimeRange() const;

	private:
		friend struct RTTIPlainType<ColorGradient>;

		RGBA mColors[MAX_KEYS];
		uint16_t mTimes[MAX_KEYS];
		uint32_t mNumKeys = 0;
		float mDuration = 0.0f;
	};

	/* @} */

	IMPLEMENT_GLOBAL_POOL(ColorGradient, 32)
}
