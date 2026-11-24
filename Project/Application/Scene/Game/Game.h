#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <memory>
#include "BossBattle.h"
#include "BossFallFlow.h"
#include "../../Object/Boss/BossTuning.h"
#include "GamePhase.h"
//[アイテム] アイテムは一旦使わないのでコメントアウト
// #include "../../Object/Item/ItemManager.h"
#include "../../Stage/LayerFloors.h"
#include "../../Object/Player/PlayerHealth.h"
#include "../../Object/Player/Shape/ShapeController.h"
#include "../../Camera/NearFadeSwitch.h"
#include "../../Direction/Transition/BlockTransition.h"
#include "../../Direction/GameDirection.h"

namespace App
{
	class Player;
	class Boss;
	class StageCamera;

	//交互に動いて床を崩し、ボスを下の層へ落としていくゲームシーン
	class Game : public RyoEngine::BaseScene
	{
	public:
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
		void UpdateGame(float deltaTime);
		void UpdateBossFall(float deltaTime); //ボスを落とす流れを進める
		void HandleBattleEvent(BossBattle::Event event); //戦闘中に起きたことに合わせて場面を変える
		void UpdateVisuals(); //見た目を確定する

		//床と配置
		void ResetFloor(bool showBelow); //床を元に戻してやり直す
		void PlaceActors(bool keepBossLanding); //プレイヤーとボスを最初の位置に置く
		Cell GetPlayerCell() const; //プレイヤーのいるマス
		BossTuning GetTuning() const { return GetBossTuning(floors_.GetLayer()); } //今の層のボスの強さ

		//ダメージ
		void FailFall(); //穴に落ちた
		void DamageBoss(int32_t amount); //ボスのダメージ
		void Defeat(); //HPが0になってゲームオーバー

		//場面の切り替え
		void WaitForMovement(); //方向の入力待ちにする
		void BeginEscape(); //ボスの島が切り離された
		void BeginBossDrop(); //ボスが落ち始める
		void OnArrivedBelow(); //下の層に着いた
		void FinishBossFall(); //足場を飛ばし終わった

		//持ち物
		LayerFloors floors_; //今いる層と下の層と上の層
		std::unique_ptr<Player> player_{};
		std::unique_ptr<Boss> boss_{};
		StageCamera* camera_ = nullptr; //持ち主はCameraManager
		ShapeController shapes_; //床を崩す形
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// ItemManager items_;                //アイテム
		PlayerHealth health_; //プレイヤーのHPと無敵
		BossBattle battle_; //戦闘中のボスとのやりとりと手番
		BossFallFlow bossFall_; //ボスを落とす流れ

		//状態
		GamePhase phase_ = GamePhase::Battle;
		int32_t bossHealth_ = 0;
		bool isPaused_ = false;
		bool isGameOver_ = false;
		bool isCleared_ = false;

		NearFadeSwitch nearFade_; //ゲーム中だけNearFadeを切る
		BlockTransition transition_; //場面切り替えの積み木
		GameDirection direction_; //画面の演出とボスの残像
	};
}
