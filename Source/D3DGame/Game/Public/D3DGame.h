#pragma once

#ifdef D3DGame_EXPORTS
	#define D3DGAME_API __declspec(dllexport)
#else
	#define D3DGAME_API __declspec(dllimport)
#endif
