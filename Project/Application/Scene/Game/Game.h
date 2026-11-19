#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <memory>
#include "BossBattle.h"
#include "BossFallFlow.h"
#include "../../Object/Boss/BossTuning.h"
#include "GamePhase.h"
// [アイテム] アイテムは一旦使わない。コメントアウト
// #include "../../Object/Item/ItemManager.h"
#include "../../Stage/LayerFloors.h"
#include "../../Object/Player/PlayerHealth.h"
#include "../../Object/Player/Shape/ShapeController.h"
#include "../../Camera/NearFadeSwitch.h"

namespace App
{
	class Player;
	class Boss;
	class StageCamera;

	// ゲームシーン (Unity のプロトタイプの PrototypeGame と同じ流れ)
	// 1. 戦闘: プレイヤーはマスからマスへ跳んで動き、形を使って前の床を崩す。ボスは追いかけて攻撃してくる
	//    形は LB / RB (Q / E) で5つから選ぶ
	//    ([アイテム] HP が少ないと、アイテムが1層に1個降ってくる … 一旦コメントアウト)
	// 2. ボスのいる島を切り離すと、揺れてからボスごと落ちる (切り離された床にいたら落ちる)
	//    ボスの足元の床が消えて、逃げられなかったときもボスが落ちる
	// 3. ボスが下の層へ落ちる (落下ダメージ)。プレイヤーも跳んで下の層へ
	// 4. 上の層に残った足場がボスへ飛んでいく (追加ダメージ。穴のとなりの足場は 4 倍)
	// 5. 方向を入力すると次の戦闘が始まる
	// ボスの体力が 0 でクリア、ハートが 0 でゲームオーバー
	// 毎フレーム動かす順番 (床を崩す → プレイヤー → 床 → アイテム → ボス → 当たり) が大事なので、
	// プレイヤー・ボス・床は GameObject にせず、このシーンが順番に動かしている
	// 役割ごとに分けたクラス:
	// ・形 (選ぶ・崩す・予告・パネル) → ShapeController / アイテム → ItemManager (一旦コメントアウト) / 3つの層の床 → LayerFloors
	// ・HP と無敵 → PlayerHealth / 戦闘中のボスとのやりとり (攻撃・接触・切り離し・分断) → BossBattle
	// ・ボスを落とす流れ (揺れ → 落下 → 足場を飛ばす) → BossFallFlow / デバッグ表示 → GameHud
	class Game : public RyoEngine::BaseScene
	{
	public:
		Game(); // 作る (Player・Boss はここで中身が決まる)
		~Game() override;

		Game(const Game&) = delete;
		Game& operator=(const Game&) = delete;

		void Initialize() override; // カメラ・床・プレイヤー・ボスを用意してゲーム開始
		void Update() override; // ポーズ・ゲームを進める・クリア / ゲームオーバーへ
		void Draw() override; // 床・プレイヤー・ボス・アイテム・デバッグ表示を描く
		void Finalize() override; // シーンを出るとき (エンジンの設定を戻す)

	private:
		// --- 1フレームの流れ ---
		void UpdateGame(float deltaTime);
		void UpdateBossFall(float deltaTime); // ボスを落とす流れを進める (結果に合わせてダメージ・やり直し など)
		void HandleBattleEvent(BossBattle::Event event); // 戦闘中に起きたことに合わせて場面を変える
		void UpdateVisuals(); // 見た目を確定する
		void DrawDebugText(); // デバッグ表示

		// --- 床・配置 ---
		void ResetFloor(bool showBelow); // 床を元に戻してやり直す
		void PlaceActors(bool keepBossLanding); // プレイヤーとボスを最初の位置に置く
		Cell GetPlayerCell() const; // プレイヤーのいるマス
		BossTuning GetTuning() const { return GetBossTuning(floors_.GetLayer()); } // 今の層のボスの強さ

		// --- ダメージ ---
		void FailFall(); // 穴に落ちた
		void DamageBoss(int32_t amount); // ボスのダメージ
		void Defeat(); // プレイヤーの HP が 0 になった (ゲームオーバー)

		// --- 場面の切り替え ---
		void WaitForMovement(); // 方向の入力待ちにする
		void BeginEscape(); // ボスの島が切り離された
		void BeginBossDrop(); // ボスが落ち始める
		void OnArrivedBelow(); // 下の層に着いた
		void FinishBossFall(); // 足場を飛ばし終わった

		// --- 持ち物 ---
		LayerFloors floors_;               // 今いる層・下の層・上の層
		std::unique_ptr<Player> player_{};
		std::unique_ptr<Boss> boss_{};
		StageCamera* camera_ = nullptr;    // 持ち主は CameraManager
		ShapeController shapes_;           // 床を崩す形
		// [アイテム] アイテムは一旦使わない。コメントアウト
		// ItemManager items_;                // アイテム
		PlayerHealth health_;              // プレイヤーの HP と無敵
		BossBattle battle_;                // 戦闘中のボスとのやりとり
		BossFallFlow bossFall_;            // ボスを落とす流れ

		// --- 状態 ---
		GamePhase phase_ = GamePhase::Battle;
		int32_t bossHealth_ = 0;
		bool isPaused_ = false;
		bool isGameOver_ = false;
		bool isCleared_ = false;

		NearFadeSwitch nearFade_;          // エンジンの「カメラに近いモデルを消す」を切る・戻す
	};
}
