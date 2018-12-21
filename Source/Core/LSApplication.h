#pragma once

#include "Platform/LSPlatform.h"
#include "Utility/LSModule.h"

namespace ls
{
	class Application : public Module<Application>
	{
	public:
		Application();
		virtual ~Application();

		INT32 run(int argc = 0, const char **argv = nullptr);

	protected:
		/** @copydoc Module::onStartUp */
		void onStartUp() override;

	protected:
		bool mRunning;
	};
}