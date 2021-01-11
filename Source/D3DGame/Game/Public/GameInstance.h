#pragma once
#include "D3DGame.h"

namespace wyc
{
	class D3DGAME_API CGameInstance
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