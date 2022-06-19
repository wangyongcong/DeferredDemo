#pragma once
#include "GameApplication.h"

namespace wyc
{
	class WindowsApplication: public GameApplication
	{
	public:
		explicit WindowsApplication(const wchar_t* appName);

	};
}
