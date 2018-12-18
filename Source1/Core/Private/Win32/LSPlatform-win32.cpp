#include "LSPlatform-win32.h"

#if PLATFORM_WINDOWS

#include "Logger/LSLogger.h"

#include <TimeAPI.h>
#include <shellapi.h>
#include <ole2.h>

#pragma comment(lib, "Winmm.lib")

namespace ls
{
	/** Encapsulate native cursor data so we can avoid including windows.h as it pollutes the global namespace. */
	struct LS_CORE_EXPORT NativeCursorData
	{
		HCURSOR cursor;
	};

	struct Platform::Pimpl
	{
		bool mIsCursorHidden = false;
		NativeCursorData mCursor;
		bool mUsingCustomCursor = false;
		//Map<const ct::RenderWindow*, WindowNonClientAreaData> mNonClientAreas;

		bool mIsTrackingMouse = false;
		//NativeDropTargetData mDropTargets;

		bool mRequiresStartUp = false;
		bool mRequiresShutDown = false;

		bool mCursorClipping = false;
		HWND mClipWindow = 0;
		RECT mClipRect;

		bool mIsActive = false;

		Mutex mSync;
	};

	Platform::Pimpl* Platform::mData = ls_new<Platform::Pimpl>();

	Platform::~Platform()
	{
		ls_delete(mData);
		mData = nullptr;
	}

	void Platform::_messagePump()
	{
		MSG  msg;
		while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void Platform::_startUp()
	{
		Lock lock(mData->mSync);

		if (timeBeginPeriod(1) == TIMERR_NOCANDO)
		{
			LOGWRN("Unable to set timer resolution to 1ms. This can cause significant waste " \
				"in performance for waiting threads.");
		}

		mData->mRequiresStartUp = true;
	}

	void Platform::_update()
	{
		/*for (auto& dropTarget : mData->mDropTargets.dropTargetsPerWindow)
		{
			dropTarget.second->update();
		}*/
	}

	void Platform::_coreUpdate()
	{
		{
			Lock lock(mData->mSync);
			if (mData->mRequiresStartUp)
			{
				OleInitialize(nullptr);

				mData->mRequiresStartUp = false;
			}
		}

		/*{
			Lock lock(mData->mSync);
			for (auto& dropTargetToDestroy : mData->mDropTargets.dropTargetsToDestroy)
			{
				dropTargetToDestroy->unregisterWithOS();
				dropTargetToDestroy->Release();
			}

			mData->mDropTargets.dropTargetsToDestroy.clear();
		}*/

		/*{
			Lock lock(mData->mSync);
			for (auto& dropTargetToInit : mData->mDropTargets.dropTargetsToInitialize)
			{
				dropTargetToInit->registerWithOS();
			}

			mData->mDropTargets.dropTargetsToInitialize.clear();
		}*/

		_messagePump();

		{
			Lock lock(mData->mSync);
			if (mData->mRequiresShutDown)
			{
				OleUninitialize();
				mData->mRequiresShutDown = false;
			}
		}
	}

	void Platform::_shutDown()
	{
		Lock lock(mData->mSync);

		timeEndPeriod(1);
		mData->mRequiresShutDown = true;
	}
}

#endif // PLATFORM_WINDOWS