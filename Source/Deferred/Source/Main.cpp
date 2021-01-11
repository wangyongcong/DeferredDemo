#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>

#include <memory>

#include "GameApplication.h"

using namespace wyc;

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	if(!CGameApplication::CreateApplication(hInstance))
	{
		return 1;
	}

	auto Application = CGameApplication::Get();

	if(!Application->ShowWindow(L"D3D12 Game", 1290, 720))
	{
		return 2;
	}

	Application->Run();

	Application->DestroyApplication();

	return 0;
}