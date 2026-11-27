#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <memory>
#include "BossBattle.h"
#include "BossFallFlow.h"
#include "../../Object/Boss/BossTuning.h"
#include "GamePhase.h"
#include "ClearPose.h"
#include "DefeatSequence.h"
#include "PauseMenu.h"
//[アイテム] アイテムは一旦使わないのでコメントアウト
// #include "../../Object/Item/ItemManager.h"
#include "PlayerAction.h"
#include "PlayerFallFlow.h"
#include "BossWake.h"
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

	//ボスと同時に動いて正面の床を崩し、ボスを下の層へ落としていくゲームシーン
	class Game : public RyoEngine::BaseScene
	{
	public:
		//シーンをどこから始めるか
		enum class StartMode
		{
			Battle, //ボス戦の最初から
			AfterTutorial, //チュートリアルで寝ているボスに足場を当てた続きから
		};
		//次にゲームシーンを始める所を決める
		static void SetStartMode(StartMode mode) { startMode_ = mode; }
		//チュートリアルから寝ているボスとプレイヤーの位置とダメージを受け取る
		static void SetWakeStart(const WakeStart& start) { wakeStart_ = start; }

		Game(); //PlayerとBossを作る
		~Game() override;

		Game(const Game&) = delete;
		Game& operator=(const Game&) = delete;

		void Initialize() override; //カメラと床とキャラを用意してゲームを始める
		void Update() override; //ポーズとゲームを進めてクリアかゲームオーバーへ
		void Draw() override; //床とキャラとデバッグ表示を描く
		void Finalize() override; //シーンを出るときにエンジンの設定を戻す

	private:
		//1フレームの流れ
		void UpdatePause(); //ポーズメニューで選んだものに合わせて戻るかシーンを変える
		void UpdateGame(float deltaTime);
		void UpdateBossFall(float deltaTime); //ボスを落とす流れを進める
		void HandleBattleEvent(BossBattle::Event event); //戦闘中に起きたことに合わせて場面を変える
		void UpdateVisuals(); //見た目を確定する

		//床と配置
		void ResetFloor(bool showBelow); //床を元に戻してやり直す
		void PlaceActors(bool keepBossLanding); //プレイヤーとボスを最初の位置に置く
		Cell GetPlayerCell() const; //プレイヤーのいるマス
		BossTuning GetTuning() const; //今の層のボスの強さに、今の形態の速さなどを入れたもの

		//チュートリアルの続きと穴に落ちたとき
		void StartAfterTutorial(); //チュートリアルと同じ所に寝ているボスとプレイヤーを置く
		void UpdateBossWake(float deltaTime); //ボスが起きたらボス戦を始める
		void UpdatePlayerFall(float deltaTime); //穴に落ちて下の層へ行く流れを進める

		//ダメージ
		void FailFall(); //穴に落ちた
		void DamageBoss(int32_t amount); //ボスのダメージ
		void Defeat(); //HPが0になってゲームオーバー
		void UpdateDefeat(); //やられ演出を進めて、終わったらゲームオーバーへ

		//場面の切り替え
		void StartBattle(); //すぐに戦闘を始める
		void BeginEscape(); //ボスの島が切り離された
		void BeginBossDrop(); //ボスが落ち始める
		void OnArrivedBelow(); //下の層に着いた
		void FinishBossFall(); //足場を飛ばし終わった

		//持ち物
		LayerFloors floors_; //今いる層と下の層と上の層
		std::unique_ptr<Player> player_{};
		std::unique_ptr<Boss> boss_{};
		StageCamera* camera_ = nullptr; //持ち主はCameraManager
		SkyDome sky_; //背景の天球
		PlayerAction action_; //プレイヤーの向きと正面の1マスを崩すと移動
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// ItemManager items_;                //アイテム
		PlayerHealth health_; //プレイヤーのHPと無敵
		BossBattle battle_; //戦闘中のボスとのやりとり
		BossFallFlow bossFall_; //ボスを落とす流れ
		PlayerFallFlow playerFall_; //プレイヤーが穴に落ちて下の層へ行く流れ
		BossWake bossWake_; //チュートリアルのあとボスが起きる

		//状態
		GamePhase phase_ = GamePhase::Battle;
		PauseMenu pauseMenu_; //TABかメニューボタンで開くポーズメニュー
		bool isGameOver_ = false;
		bool isCleared_ = false;
		DefeatSequence defeatSequence_; //やられ演出でカメラとプレイヤーだけ進める
		ClearPose clearPose_; //クリアのときにプレイヤーへ寄せてポーズを見せる

		NearFadeSwitch nearFade_; //ゲーム中だけNearFadeを切る
		ToneMapSwitch toneMap_; //ゲーム中だけACESを切り替える
		BlockTransition transition_; //場面切り替えの積み木
		GameDirection direction_; //画面の演出と揺れと着地のぷるるん

		static inline StartMode startMode_ = StartMode::Battle; //チュートリアルから来たときだけその続きから
		static inline WakeStart wakeStart_{}; //チュートリアルから受け取った位置とダメージ
	};
}
