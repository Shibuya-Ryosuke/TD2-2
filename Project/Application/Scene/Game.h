#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <memory>
#include <vector>
#include "../Game/BossTuning.h"
#include "../Game/BreakShape.h"
#include "../Game/BreakShapeQueue.h"
#include "../Stage/Floor.h"
#include "../UI/NextShapePanel.h"

namespace App {
	class Player;
	class Boss;
	class StageCamera;

	/// <summary>
	/// ゲームシーン (Unity のプロトタイプの PrototypeGame と同じ流れ)
	///
	/// 1. 戦闘: プレイヤーはマスからマスへ跳んで動き、テトリスの形で前の床を崩す。ボスは追いかけて攻撃してくる
	/// 2. ボスのいる島を切り離すと、揺れてからボスごと落ちる (切り離された床にいたら落ちる)
	///    ボスの足元の床が消えて、逃げられなかったときもボスが落ちる
	/// 3. ボスが下の層へ落ちる (落下ダメージ)。プレイヤーも跳んで下の層へ
	/// 4. 上の層に残った足場がボスへ飛んでいく (追加ダメージ。穴のとなりの足場は 4 倍)
	/// 5. 方向を入力すると次の戦闘が始まる
	/// ボスの体力が 0 でクリア、ハートが 0 でゲームオーバー
	///
	/// 毎フレーム動かす順番 (床を崩す → プレイヤー → 床 → ボス → 当たり) が大事なので、
	/// プレイヤー・ボス・床は GameObject にせず、このシーンが順番に動かしている
	/// </summary>
	class Game : public RyoEngine::BaseScene {
	public:
		Game(); // 作る (Player・Boss はここで中身が決まる)
		~Game() override;

		Game(const Game&) = delete;
		Game& operator=(const Game&) = delete;

		void Initialize() override; // カメラ・床・プレイヤー・ボスを用意してゲーム開始
		void Update() override; // ポーズ・ゲームを進める・クリア / ゲームオーバーへ
		void Draw() override; // 床・プレイヤー・ボス・デバッグ表示を描く
		void Finalize() override; // シーンを出るとき (エンジンの設定を戻す)

	private:
		/// <summary>今どの場面か</summary>
		enum class Phase {
			Waiting,       // 方向を入力すると戦闘開始
			Battle,        // 戦闘中
			Escape,        // ボスの島が切り離された (揺れている)
			Descent,       // ボスが落ちて、プレイヤーも下の層へ
			LandingImpact, // ボスが着地した直後の待ち
			TileBarrage,   // 上の層の足場がボスへ飛んでいく
		};

		// --- 1フレームの流れ ---
		void UpdateGame(float deltaTime);
		void UpdateBattle(bool isFloorChanged, const Cell& bossCell, bool isJumpImpact, float deltaTime); // 戦闘中: 切り離し・ボス・当たり
		void UpdateVisuals(); // 見た目を確定する
		void DrawDebugText(); // デバッグ表示

		// --- 床を崩す (テトリスの形) ---
		void ResetShapes();
		const BreakShape* FindShape(BreakShapeType type) const; // 種類から形を探す (なければ nullptr)
		const BreakShape* GetCurrentShape();   // 今の形 (なければ nullptr)
		void BreakFloor();                     // 今の形で、プレイヤーの前の床を崩す
		void UpdateShapePreview();             // 床に今の形の予告を出す
		void UpdateNextShapePanel();           // 画面の右はしに 今・次・その次 の形を出す

		// --- 床・配置 ---
		std::unique_ptr<Floor> CreateFloor(int32_t index) const; // index 層目の床を作る (CSV を読む)
		void ResetFloor(bool showBelow); // 床を元に戻してやり直す
		void PlaceActors(bool keepBossLanding); // プレイヤーとボスを最初の位置に置く
		RyoEngine::Vector3 GetSpawnPosition(const Floor& floor, int32_t firstLayerX, int32_t firstLayerZ) const; // 1層目のマスで決めた位置を、この層のマスに直す
		BossTuning GetTuning() const { return GetBossTuning(layer_); } // 今の層のボスの強さ

		// --- 当たり・ダメージ ---
		void CheckBossContact(bool isJumpImpact); // ボスに触れたか (跳んでぶつかったらボスもはじく)
		void ApplyContactKnockback(const RyoEngine::Vector3& away); // ボスに触れたときのはじき
		void CheckForSeparatedFloor(); // たどり着けなくなったら床を戻す
		void FailFall(); // 穴に落ちた
		void Damage(int32_t amount); // プレイヤーのダメージ
		void DamageBoss(int32_t amount); // ボスのダメージ
		void UpdatePlayerVisibility(); // 無敵の間の点滅

		// --- 場面の切り替え ---
		void WaitForMovement(); // 方向の入力待ちにする
		void BeginEscape(); // ボスの島が切り離された
		void AdvanceEscape(float deltaTime); // 揺れる → ボスごと落とす
		void BeginBossDrop(); // ボスが落ち始める
		void AdvanceDescent(float deltaTime); // 下の層へ
		void BeginTileBarrage(); // 足場を飛ばし始める
		void AdvanceTileBarrage(float deltaTime); // 足場を飛ばす
		void FinishTileBarrage(); // 飛ばし終わった

		// --- 持ち物 ---
		std::unique_ptr<Floor> current_{}; // 今いる層
		std::unique_ptr<Floor> below_{};   // 下の層 (暗く見えている)
		std::unique_ptr<Floor> upper_{};   // 降りてきた上の層 (足場がボスへ飛んでいく間だけ)
		std::unique_ptr<Player> player_{};
		std::unique_ptr<Boss> boss_{};
		StageCamera* camera_ = nullptr;    // 持ち主は CameraManager

		// --- 状態 ---
		Phase phase_ = Phase::Battle;
		int32_t layer_ = 0;                // 何層目か (0 から)
		int32_t hearts_ = 0;
		int32_t bossHealth_ = 0;
		float timer_ = 0.0f;               // 場面ごとの時間
		float invincible_ = 0.0f;          // 無敵の残り時間
		bool isPaused_ = false;
		bool isGameOver_ = false;
		bool isCleared_ = false;
		bool separationNeedsCheck_ = false; // 床が消えたので「おたがいにたどり着けるか」を調べる
		bool isTouchingBoss_ = false;

		// 下の層へ
		RyoEngine::Vector3 dropOrigin_{};   // ボスが落ち始めた位置
		RyoEngine::Vector3 descentStart_{}; // プレイヤーが降り始めた位置
		RyoEngine::Vector3 descentEnd_{};   // プレイヤーが降りる先
		bool isBossLanded_ = false;

		// 床を崩す形
		std::vector<BreakShape> shapes_;   // 形の種類 (I O T S Z L J)
		BreakShapeQueue shapeQueue_;       // 次に来る形の順番
		std::vector<Cell> shapeCells_;     // 今の形で崩すマス (使い回し)
		float shapeCooldownTimer_ = 0.0f;  // 次の形を使えるまで
		NextShapePanel nextShapePanel_;    // 今・次・その次 の形の表示

		// 足場を飛ばす
		int32_t remainingTiles_ = 0;
		int32_t tileDamageApplied_ = 0;

		// 入る前の「カメラに近いモデルを消す距離」(出るときに戻す)
		float savedNearFadeStart_ = 0.0f;
		float savedNearFadeEnd_ = 0.0f;
	};
}
