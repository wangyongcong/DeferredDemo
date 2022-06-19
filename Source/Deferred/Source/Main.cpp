#include "GameApplication.h"
#include "IGameInstance.h"
#include "rtm/types.h"

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

APPLICATION_MAIN(GameDeferred)
