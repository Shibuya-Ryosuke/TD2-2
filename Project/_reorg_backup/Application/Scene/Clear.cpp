#include "Clear.h"
#include "Title.h"
#include "../Input/GameInput.h"

using namespace RyoEngine;

namespace App
{
	// 今は何もしない
	void Clear::Initialize()
	{
	}

	// Enter / A でタイトルへ
	void Clear::Update()
	{
		if (GameInput::IsEnterTriggered())
		{
			SceneManager::ChangeScene<Title>();
		}
	}

	// 文字を出す
	void Clear::Draw()
	{
		// 画面の真ん中あたり (文字の位置はゲーム画面の解像度が基準。今は 1920 x 1080)
		const float width = static_cast<float>(DirectXCommon::GetGameWidth());
		const float height = static_cast<float>(DirectXCommon::GetGameHeight());
		PrintText("GAME CLEAR!", { width * 0.42f, height * 0.45f });
		PrintText("PRESS ENTER / A", { width * 0.43f, height * 0.5f });
	}
}
