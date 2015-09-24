#include "PlatformManager.hpp"

//TODO separate platform implementations
#include <Windows.h>

MessagePumpResult PlatformManager::PumpMessage()
{
	MSG msg;
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) 
		{
			return Quit;
		}
		else 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			return MoreMessages;
		}
	}

	return NoMessages;
}
