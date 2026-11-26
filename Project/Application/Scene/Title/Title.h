#pragma once
#include <RyoEngine.h>
#include "TitleDrop.h"
#include "../Showcase/ShowcaseStage.h"
#include "../../Direction/Transition/BlockTransition.h"

namespace App
{
	//プレイヤーが積み木を持ってゆらゆらしていて、スタートで落としてあせり、床ごと落ちて場面切り替えにつながる仮のタイトルシーン
	class Title : public RyoEngine::BaseScene
	{
	public:
		Title() = default;
		~Title() override = default;

		Title(const Title&) = delete;
		Title& operator=(const Title&) = delete;

		void Initialize() override; //カメラと空と床とプレイヤーを用意する
		void Update() override; //EnterかAで積み木を落としてチュートリアルへ
		void Draw() override; //床とプレイヤーと文字を描く
		void Finalize() override; //ACESとNearFadeを元に戻す

	private:
		void UpdateCamera(); //ゆらゆらは引き、落としたら寄り、がたがたで引いて、落ちたら見下ろして追う

		ShowcaseStage stage_; //カメラと空と床
		TitleDrop drop_; //積み木を持つプレイヤー
		BlockTransition transition_; //場面切り替えの積み木
		bool isTransitionStarted_ = false; //場面切り替えを始めたか
	};
}
