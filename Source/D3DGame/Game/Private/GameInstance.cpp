#include "D3DGamePCH.h"
#include "GameInstance.h"

namespace wyc
{
	std::shared_ptr<wyc::CGameInstance> CGameInstance::sGameInstance;

	bool CGameInstance::CreateGameInstance()
	{
		sGameInstance = std::make_shared<CGameInstance>();
		return true;
	}

	void CGameInstance::DestroyGameInstance()
	{

	}

	std::shared_ptr<wyc::CGameInstance> CGameInstance::Get()
	{
		return sGameInstance;
	}

	CGameInstance::CGameInstance()
	{

	}

	CGameInstance::~CGameInstance()
	{

	}

} // namespace wyc