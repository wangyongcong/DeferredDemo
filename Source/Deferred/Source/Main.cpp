#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>
#include <memory>
#include "GameApplication.h"

using namespace wyc;

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	if(!CGameApplication::CreateApplication(hInstance, L"GameDeferred", 1290, 720))
	{
		return 1;
	}

	auto Application = CGameApplication::Get();
	// @todo: exception handling
	Application->Run();

	Application->DestroyApplication();

	return 0;
}