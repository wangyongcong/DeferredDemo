#pragma once
#include "GameFramework.h"

namespace wyc
{
	class GAME_FRAMEWORK_API GameInstance
	{
	public:
		static bool CreateGameInstance();
		static void DestroyGameInstance();
		static GameInstance* Get();

		GameInstance();
		virtual ~GameInstance();

	protected:
		static GameInstance* spGameInstance;

	};
}