#pragma once
#include <RyoEngine.h>
#include "../../Direction/Transition/BlockTransition.h"

namespace App
{
	//Debugのときだけ文字が出る仮のクリアシーン
	class Clear : public RyoEngine::BaseScene
	{
	public:
		Clear() = default;
		~Clear() override = default;

		Clear(const Clear&) = delete;
		Clear& operator=(const Clear&) = delete;

		void Initialize() override; //演出のカメラを作る
		void Update() override; //EnterかAでタイトルへ
		void Draw() override; //文字を出す

	private:
		BlockTransition transition_; //場面切り替えの積み木
	};
}
