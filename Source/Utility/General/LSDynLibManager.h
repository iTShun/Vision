#pragma once

#include "Prerequisites/LSPrerequisitesUtil.h"
#include "General/LSModule.h"

namespace ls
{
	/** @addtogroup General
	 *  @{
	 */

	/**
	 * This manager keeps track of all the open dynamic-loading libraries, it manages opening them opens them and can be
	 * used to lookup already already-open libraries.
	 *
	 * @note	Not thread safe.
	 */
	class LS_UTILITY_EXPORT DynLibManager : public Module<DynLibManager>
	{
	public:
		/**
		 * Loads the given file as a dynamic library.
		 *
		 * @param[in]	name	The name of the library. The extension can be omitted.
		 */
		DynLib* load(String name);

		/** Unloads the given library. */
		void unload(DynLib* lib);

	protected:
		Set<UPtr<DynLib>, std::less<>> mLoadedLibraries;
	};

	/** Easy way of accessing DynLibManager. */
	LS_UTILITY_EXPORT DynLibManager& gDynLibManager();

	/** @} */
}
