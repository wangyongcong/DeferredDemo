#pragma once
#include "GameFramework.h"

namespace wyc
{
	class GameFramework_API CGameInstance
	{
	public:
		static bool CreateGameInstance();
		static void DestroyGameInstance();

		static std::shared_ptr<CGameInstance> Get();

		CGameInstance();
		virtual ~CGameInstance();

	protected:

		static std::shared_ptr<CGameInstance> sGameInstance;

	};
}