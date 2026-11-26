#pragma once
#include <RyoEngine.h>
#include "GameOverBury.h"
#include "GameOverMenu.h"
#include "../Showcase/ShowcaseStage.h"
#include "../../Direction/Transition/BlockTransition.h"

namespace App
{
	//プレイヤーがきょろきょろしたあと降ってくる積み木に埋もれて、ボス戦からやり直すかタイトルに戻るかを選ぶ仮のゲームオーバーシーン
	class GameOver : public RyoEngine::BaseScene
	{
	public:
		GameOver() = default;
		~GameOver() override = default;

		GameOver(const GameOver&) = delete;
		GameOver& operator=(const GameOver&) = delete;

		void Initialize() override; //カメラと空と床とプレイヤーと積み木を用意する
		void Update() override; //埋もれる演出を進めて、埋もれたら選んだ方へ積み木で切り替える
		void Draw() override; //積み木とプレイヤーと文字を描く
		void Finalize() override; //ACESを元に戻す

	private:
		ShowcaseStage stage_; //カメラと空と床
		GameOverBury bury_; //積み木に埋もれるプレイヤー
		BlockTransition transition_; //場面切り替えの積み木
		GameOverMenu menu_; //やり直すかタイトルに戻るか
		float menuTimer_ = 0.0f; //埋もれきってからの秒数
	};
}
