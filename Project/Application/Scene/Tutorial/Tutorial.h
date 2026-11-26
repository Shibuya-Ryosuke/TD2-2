#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <memory>
#include "TutorialFlow.h"
#include "../Game/PauseMenu.h"
#include "../Game/PlayerAction.h"
#include "../../Stage/Floor/LayerFloors.h"
#include "../../Stage/Sky/SkyDome.h"
#include "../../Object/Player/PlayerHealth.h"
#include "../../Camera/NearFadeSwitch.h"
#include "../../Direction/Screen/ToneMapSwitch.h"
#include "../../Direction/Transition/BlockTransition.h"
#include "../../Direction/GameDirection.h"

namespace App
{
	class Player;
	class Boss;
	class StageCamera;

	//タイトルのあと、動く・壊す・積み木を落とす・切り離す・下へ落ちるを練習して、寝ているボスに足場を当てるシーン
	class Tutorial : public RyoEngine::BaseScene
	{
	public:
		Tutorial(); //PlayerとBossを作る
		~Tutorial() override;

		Tutorial(const Tutorial&) = delete;
		Tutorial& operator=(const Tutorial&) = delete;

		void Initialize() override; //カメラとチュートリアルの床と寝ているボスを用意する
		void Update() override; //ポーズとチュートリアルを進めて、終わったらゲームシーンへ
		void Draw() override; //床とキャラと積み木とやることの文字を描く
		void Finalize() override; //シーンを出るときにエンジンの設定を戻す

	private:
		void UpdatePause(); //ポーズメニューで選んだものに合わせて戻るかシーンを変える
		void UpdateVisuals(); //見た目を確定する
		void HandOver(); //プレイヤーとボスの位置とダメージをゲームシーンへ渡して切り替える
		TutorialFlow::Context MakeContext(); //チュートリアルの流れが動かすもの

		LayerFloors floors_; //チュートリアルの床と、寝ているボスがいる1層目
		std::unique_ptr<Player> player_{};
		std::unique_ptr<Boss> boss_{};
		StageCamera* camera_ = nullptr; //持ち主はCameraManager
		SkyDome sky_; //背景の天球
		PlayerAction action_; //プレイヤーの向きと正面の1マスを崩すと移動
		PlayerHealth health_; //画面の演出に渡すだけでチュートリアルでは減らない
		TutorialFlow flow_; //チュートリアルの流れ
		int32_t bossDamage_ = 0; //残った足場で寝ているボスに当てたダメージ
		bool isHandedOver_ = false; //ゲームシーンへ切り替えたか

		PauseMenu pauseMenu_; //TABかメニューボタンで開くポーズメニュー
		NearFadeSwitch nearFade_; //チュートリアル中だけNearFadeを切る
		ToneMapSwitch toneMap_; //チュートリアル中だけACESを切り替える
		BlockTransition transition_; //場面切り替えの積み木
		GameDirection direction_; //画面の演出と揺れと着地のぷるるん
	};
}
