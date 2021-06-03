#include "GameFrameworkPCH.h"
#include "GameInstance.h"

namespace wyc
{
	GameInstance* GameInstance::spGameInstance;

	bool GameInstance::CreateGameInstance()
	{
		spGameInstance = new GameInstance();
		return true;
	}

	void GameInstance::DestroyGameInstance()
	{
		if(spGameInstance)
		{
			delete spGameInstance;
			spGameInstance = nullptr;
		}
	}

	GameInstance* GameInstance::Get()
	{
		return spGameInstance;
	}

	GameInstance::GameInstance()
	{

	}

	GameInstance::~GameInstance()
	{

	}

} // namespace wyc