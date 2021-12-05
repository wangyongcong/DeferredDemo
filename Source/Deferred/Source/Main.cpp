#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>
#include <memory>
#include "IGameInstance.h"
#include "IRenderer.h"
#include "GameApplication.h"

using namespace wyc;
using namespace rtm;

struct Vertex
{
	float3f position;
	float4f color;
};

void GenerateTriangleVertex(float r)
{
	const float sin30 = 0.5f, cos30 = 0.866f;
	Vertex vertices[] = {
		{
			{ 0, r, 0 },
			{ 1.0f, 0, 0, 1.0f },
		},
		{
			{ -r * cos30, -r * sin30, 0 },
			{ 0, 1.0f, 0, 1.0f },
		},
		{
			{ r * cos30, -r * sin30, 0 },
			{ 0, 0, 1.0f, 1.0f },
		},
	};
}

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


	void Draw(IRenderer* pRenderer) override
	{
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