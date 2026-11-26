#pragma once
#include <cstdint>

namespace App
{
	//デバッグ表示で出すボス戦の最初からやり直すか、タイトルに戻るかのゲームオーバーのメニュー
	class GameOverMenu
	{
	public:
		//このフレームで選ばれたもの
		enum class Choice
		{
			None, //まだ選んでいない
			Retry, //ボス戦の最初からやり直す
			Title, //タイトルにもどる
		};

		//上下で選んで決定したものを返す
		Choice Update();
		//メニューを描く
		void Draw() const;

	private:
		//選んでいる項目をstepだけ上下にずらしてはしで回り込む
		void Move(int32_t step);

		int32_t selected_ = 0; //上から数えた選んでいる項目
		bool isStickHeld_ = true; //右スティックを倒したままか
	};
}
