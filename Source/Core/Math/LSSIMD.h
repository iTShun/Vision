#pragma once

#include "Platform/LSPlatform.h"
#include "Math/LSVector4.h"
#include "Math/LSAABox.h"
#include "Math/LSSphere.h"

#define SIMDPP_ARCH_X86_SSE4_1

#if COMPILER_MSVC
#	pragma warning(disable: 4244)
#endif

// ThirdParty
#include "simdpp/simd.h"

#if COMPILER_MSVC
#	pragma warning(default: 4244)
#endif

namespace ls
{
	namespace simd
	{
		using namespace simdpp;

		/** @addtogroup Math
		 *  @{
		 */

		/** 
		 * Version of ls::AABox suitable for SIMD use. Takes up a bit more memory than standard AABox and is always 16-byte
		 * aligned.
		 */
		struct AABox
		{
			/** Center of the bounds, W component unused. */
			SIMDPP_ALIGN(16) Vector4 center;

			/** Extents (half-size) of the bounds, W component unused. */
			SIMDPP_ALIGN(16) Vector4 extents;

			AABox() = default;

			/** Initializes bounds from an AABox. */
			AABox(const ls::AABox& box)
			{
				center = Vector4(box.getCenter());
				extents = Vector4(box.getHalfSize());
			}

			/** Initializes bounds from a Sphere. */
			AABox(const Sphere& sphere)
			{
				center = Vector4(sphere.getCenter());

				float radius = sphere.getRadius();
				extents = Vector4(radius, radius, radius, 0.0f);
			}

			/** Initializes bounds from a vector representing the center and equal extents in all directions. */
			AABox(const Vector3& center, float extent)
			{
				this->center = Vector4(center);
				extents = Vector4(extent, extent, extent, 0.0f);
			}

			/** Returns true if the current bounds object intersects the provided object. */
			bool intersects(const AABox& other) const
			{
				auto myCenter = load<float32x4>(&center);
				auto otherCenter = load<float32x4>(&other.center);

				float32x4 diff = abs(sub(myCenter, otherCenter));

				auto myExtents = simd::load<float32x4>(&extents);
				auto otherExtents = simd::load<float32x4>(&other.extents);

				float32x4 extents = add(myExtents, otherExtents);

				return test_bits_any(bit_cast<uint32x4>(cmp_gt(diff, extents))) == false;
			}
		};

		/** @} */
	}
}
