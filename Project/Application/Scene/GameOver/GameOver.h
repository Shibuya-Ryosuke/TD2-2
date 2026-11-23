#pragma once
#include <RyoEngine.h>
#include "../../Direction/Transition/BlockTransition.h"

namespace App
{
	//Debugのときだけ文字が出る仮のゲームオーバーシーン
	class GameOver : public RyoEngine::BaseScene
	{
	public:
		GameOver() = default;
		~GameOver() override = default;

		GameOver(const GameOver&) = delete;
		GameOver& operator=(const GameOver&) = delete;

		void Initialize() override; //演出のカメラを作る
		void Update() override; //EnterかAでタイトルへ
		void Draw() override; //文字を出す

	private:
		BlockTransition transition_; //場面切り替えの積み木
	};
}
