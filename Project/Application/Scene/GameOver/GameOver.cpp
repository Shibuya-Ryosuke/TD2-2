#include "GameOver.h"
#include "../Title/Title.h"
#include "../../Direction/Transition/TransitionCamera.h"
#include "../../Input/GameInput.h"

using namespace RyoEngine;

namespace App
{
	//演出のカメラを作って場面切り替えを用意する
	void GameOver::Initialize()
	{
		const uint32_t cameraIndex = CameraManager::CreateCamera<TransitionCamera>("Transition");
		transition_.Initialize(CameraManager::GetCamera<TransitionCamera>(cameraIndex));
	}

	//EnterかAでタイトルへ
	void GameOver::Update()
	{
		//演出の間は操作させない
		if (transition_.Update())
		{
			return;
		}
		if (GameInput::IsEnterTriggered())
		{
			transition_.Start<Title>();
		}
	}

	//文字を出す
	void GameOver::Draw()
	{
		//1920x1080が基準の画面の真ん中あたり
		const float width = static_cast<float>(DirectXCommon::GetGameWidth());
		const float height = static_cast<float>(DirectXCommon::GetGameHeight());
		PrintText("GAME OVER", { width * 0.42f, height * 0.45f });
		PrintText("PRESS ENTER / A", { width * 0.43f, height * 0.5f });

		//いちばん手前に場面切り替えの積み木
		transition_.Draw();
	}
}
