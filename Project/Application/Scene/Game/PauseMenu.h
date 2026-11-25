#pragma once
#include <cstdint>

namespace App
{
	//デバッグ表示で出すゲームに戻る、タイトルに戻る、リトライのポーズメニュー
	class PauseMenu
	{
	public:
		//このフレームで選ばれたもの
		enum class Choice
		{
			None, //まだ選んでいない
			Resume, //ゲームにもどる
			Title, //タイトルにもどる
			Retry, //インゲームの最初からやり直す
		};

		//いちばん上を選んだ状態で開く
		void Open();
		//閉じる
		void Close() { isOpen_ = false; }
		bool IsOpen() const { return isOpen_; }

		//上下で選んで決定したものを返す
		Choice Update();
		//開いている間だけメニューを描く
		void Draw() const;

	private:
		//選んでいる項目をstepだけ上下にずらしてはしで回り込む
		void Move(int32_t step);

		bool isOpen_ = false;
		int32_t selected_ = 0; //上から数えた選んでいる項目
		bool isStickHeld_ = false; //右スティックを倒したままか
	};
}
