#pragma once
#include "GameFramework.h"

namespace wyc
{

	class IRenderer;

	class GAME_FRAMEWORK_API IGameInstance
	{
	public:
		virtual void Init() = 0;
		virtual void Exit() = 0;
		virtual void Tick(float deltaTime) = 0;
		virtual void Draw(IRenderer* pRenderer) = 0;
	};
}