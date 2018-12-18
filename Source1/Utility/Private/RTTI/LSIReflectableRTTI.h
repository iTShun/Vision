#pragma once

#include "Prerequisites/LSPrerequisitesUtil.h"
#include "Reflection/LSRTTIType.h"

namespace ls
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	class LS_UTILITY_EXPORT IReflectableRTTI : public RTTIType <IReflectable, IReflectable, IReflectableRTTI>
	{
	public:
		const String& getRTTIName() override
		{
			static String name = "IReflectable";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_IReflectable;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
}
