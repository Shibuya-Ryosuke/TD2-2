#include "PauseMenu.h"
#include "../../Input/GameInput.h"
#include <RyoEngine.h>
#include <array>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		constexpr int32_t kItemCount = 3; //項目の数
		constexpr float kStickThreshold = 0.5f; //右スティックをこれ以上倒したら1つ動く
		constexpr float kStickRelease = 0.3f; //右スティックをこれより戻したら次に倒せる
		constexpr float kLeft = 860.0f; //メニューの左のはし
		constexpr float kTitleY = 400.0f; //見出しの高さ
		constexpr float kFirstItemY = 470.0f; //いちばん上の項目の高さ
		constexpr float kItemSpacing = 40.0f; //項目と項目の間

		//上から順の項目の文字
		constexpr std::array<const char*, kItemCount> kItemTexts = {
			"BACK TO GAME",
			"BACK TO TITLE",
			"RETRY FROM THE START",
		};
	}

	//いちばん上を選んだ状態で開く
	void PauseMenu::Open()
	{
		isOpen_ = true;
		selected_ = 0;
		//開いたときに倒していたスティックでは動かさない
		isStickHeld_ = true;
	}

	//上下で選んで決定したものを返す
	PauseMenu::Choice PauseMenu::Update()
	{
		if (!isOpen_)
		{
			return Choice::None;
		}

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
			return Choice::Resume;
		}
		if (selected_ == 1)
		{
			return Choice::Title;
		}
		return Choice::Retry;
	}

	//選んでいる項目をstepだけ上下にずらしてはしで回り込む
	void PauseMenu::Move(int32_t step)
	{
		selected_ = (selected_ + step + kItemCount) % kItemCount;
	}

	//開いている間だけメニューを描く
	void PauseMenu::Draw() const
	{
		if (!isOpen_)
		{
			return;
		}
		PrintText("PAUSE", { kLeft, kTitleY });
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
