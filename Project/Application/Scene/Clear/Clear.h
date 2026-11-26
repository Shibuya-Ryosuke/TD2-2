#pragma once
#include <RyoEngine.h>
#include "ClearShow.h"
#include "../Showcase/ShowcaseStage.h"
#include "../../Direction/Transition/BlockTransition.h"

namespace App
{
	//プレイヤーが倒れたボスをハンマーでたたいてバラバラにし、踊り始めたらタイトルに戻れる仮のクリアシーン
	class Clear : public RyoEngine::BaseScene
	{
	public:
		Clear() = default;
		~Clear() override = default;

		Clear(const Clear&) = delete;
		Clear& operator=(const Clear&) = delete;

		void Initialize() override; //カメラと空と床とプレイヤーとボスを用意する
		void Update() override; //流れを進めて、踊り始めたらEnterかAでタイトルへ
		void Draw() override; //床とキャラと文字を描く
		void Finalize() override; //ACESとNearFadeを元に戻す

	private:
		ShowcaseStage stage_; //カメラと空と床
		ClearShow show_; //たたいて踊る流れ
		BlockTransition transition_; //場面切り替えの積み木
	};
}
