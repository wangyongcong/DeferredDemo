#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>
#include <memory>
#include "IGameInstance.h"
#include "IRenderDevice.h"
#include "GameApplication.h"

using namespace wyc;

class GameDeferred : public IGameInstance
{
public:
	
	void Init() override
	{
	}


	void Exit() override
	{
	}


	void Tick(float deltaTime) override
	{
	}


	void Draw() override
	{
		GameApplication* pApplication = GameApplication::Get();
		IRenderDevice* pRenderer = pApplication->GetDevice();
		
	}

};

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	if(!GameApplication::CreateApplication(hInstance, L"GameDeferred", 1290, 720))
	{
		return 1;
	}

	auto Application = GameApplication::Get();
	GameDeferred TheGame;
	Application->StartGame(&TheGame);

	Application->DestroyApplication();

	return 0;
}