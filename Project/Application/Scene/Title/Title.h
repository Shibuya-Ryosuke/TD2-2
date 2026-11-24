#pragma once
#include <RyoEngine.h>
#include "../../Direction/Transition/BlockTransition.h"

namespace App
{
	//Debugのときだけ文字が出る仮のタイトルシーン
	class Title : public RyoEngine::BaseScene
	{
	public:
		Title() = default;
		~Title() override = default;

		Title(const Title&) = delete;
		Title& operator=(const Title&) = delete;

		void Initialize() override; //演出のカメラを作る
		void Update() override; //EnterかAでゲームへ
		void Draw() override; //文字を出す

	private:
		BlockTransition transition_; //場面切り替えの積み木
	};
}
