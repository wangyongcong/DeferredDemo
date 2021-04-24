#pragma once

#ifdef GameFramework_EXPORTS
	#define GameFramework_API __declspec(dllexport)
#else
	#define GameFramework_API __declspec(dllimport)
#endif
