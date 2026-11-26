#include "GameOverMenu.h"
#include "../../Input/GameInput.h"
#include <RyoEngine.h>
#include <array>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		constexpr int32_t kItemCount = 2; //項目の数
		constexpr float kStickThreshold = 0.5f; //右スティックをこれ以上倒したら1つ動く
		constexpr float kStickRelease = 0.3f; //右スティックをこれより戻したら次に倒せる
		constexpr float kLeft = 860.0f; //メニューの左のはし
		constexpr float kTitleY = 420.0f; //見出しの高さ
		constexpr float kFirstItemY = 490.0f; //いちばん上の項目の高さ
		constexpr float kItemSpacing = 40.0f; //項目と項目の間

		//上から順の項目の文字
		constexpr std::array<const char*, kItemCount> kItemTexts = {
			"RETRY FROM THE BOSS BATTLE",
			"BACK TO TITLE",
		};
	}

	//上下で選んで決定したものを返す
	GameOverMenu::Choice GameOverMenu::Update()
	{
		//WSと十字の上下
		if (GameInput::IsMenuUpTriggered())
		{
			Move(-1);
		}
		if (GameInput::IsMenuDownTriggered())
		{
			Move(1);
		}

		//右スティックは倒した瞬間だけ1つ動く
		const float stickY = GameInput::GetMenuStickY();
		if (isStickHeld_)
		{
			if (std::abs(stickY) < kStickRelease)
			{
				isStickHeld_ = false;
			}
		}
		else if (stickY > kStickThreshold)
		{
			Move(-1);
			isStickHeld_ = true;
		}
		else if (stickY < -kStickThreshold)
		{
			Move(1);
			isStickHeld_ = true;
		}

		//EnterかAで決定
		if (!GameInput::IsEnterTriggered())
		{
			return Choice::None;
		}
		if (selected_ == 0)
		{
			return Choice::Retry;
		}
		return Choice::Title;
	}

	//選んでいる項目をstepだけ上下にずらしてはしで回り込む
	void GameOverMenu::Move(int32_t step)
	{
		selected_ = (selected_ + step + kItemCount) % kItemCount;
	}

	//メニューを描く
	void GameOverMenu::Draw() const
	{
		PrintText("GAME OVER", { kLeft, kTitleY });
		for (int32_t index = 0; index < kItemCount; ++index)
		{
			const float y = kFirstItemY + kItemSpacing * static_cast<float>(index);
			const char* text = kItemTexts[static_cast<size_t>(index)];
			//選んでいる項目に印をつける
			if (index == selected_)
			{
				PrintText("> {}", { kLeft, y }, text);
			}
			else
			{
				PrintText("  {}", { kLeft, y }, text);
			}
		}
	}
}
