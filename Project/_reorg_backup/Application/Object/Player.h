#pragma once
#include <RyoEngine.h>
#include <memory>
#include "../Stage/Cell.h"

namespace App
{
	class Floor;
	// プレイヤー (Unity のプロトタイプと同じ動き)。
	// ・移動の入力で、カメラから見た向きに一番近い上下左右のマスへ 1マス跳ぶ (押しっぱなしなら続けて跳ぶ)
	// となりが穴で、その先が床なら飛び越える。穴・床の外へは跳ばない
	// ・向き (床を崩す形の向き) は SetFacing で変える。移動しても向きは変わらない
	// ・はじかれている間 (ノックバック) は操作できない。ボスに触れてはじかれたときは、床のふちで止まらない
	// ・足元の床がなくなったら落ちる
	// 動かす順番はゲームシーン (Game) が決めるので、GameObject にはしていない。
	// Game が毎フレーム Advance → UpdateVisuals → Draw の順に呼ぶ。
	class Player
	{
	public:
		Player() = default;
		~Player() = default;

		Player(const Player&) = delete;
		Player& operator=(const Player&) = delete;

		void Initialize(); // モデルを作る (体と正面の板)

		// 1フレーム動かす。穴・床の外へ落ちたら true
		// move: 移動の入力 (長さ 1 まで)
		// hasMovement: 移動の入力があるか
		// floor: 今いる層の床
		bool Advance(const RyoEngine::Vector2& move, bool hasMovement, const Floor& floor, float deltaTime);

		//位置に置いて、動きの状態を全部戻す (やり直し・次の層)。向きは奥 (+Z) にする
		void Place(const RyoEngine::Vector3& position);

		// はじく (ボスに触れた・パンチが当たった)。向きは上下左右のどれかにそろえる
		// overFence: 床のふちで止めないか (ボスに触れたときは true)
		void ApplyKnockback(const Floor& floor, const RyoEngine::Vector3& direction, float speed, float duration, float liftSpeed, bool overFence);

		//向きを変える (上下左右のどれか)
		void SetFacing(const Cell& facing);
		//今の向き (上下左右のどれか)
		const Cell& GetFacing() const { return facing_; }

		//行列を確定する (動かし終わったあとに呼ぶ)
		void UpdateVisuals();
		void Draw(); // 体と正面の板を描く

		const RyoEngine::Vector3& GetPosition() const { return position_; }
		//位置だけ動かす (下の層へ降りる途中など)
		void SetPosition(const RyoEngine::Vector3& position) { position_ = position; }

		bool IsGrounded() const { return isGrounded_; }
		bool IsJumping() const { return isJumping_; }
		bool IsKnockedBack() const { return knockbackTime_ > 0.0f; }
		//跳んでいる先のマス
		const Cell& GetJumpDestination() const { return jumpDestination_; }
		//跳び始めた位置
		const RyoEngine::Vector3& GetJumpOrigin() const { return jumpOrigin_; }

		//描くか (無敵の間の点滅)
		void SetVisible(bool isVisible) { isVisible_ = isVisible; }

		// [アイテム] アイテムは一旦使わない。コメントアウト
		// //2マス乗り越えのアイテム (持っている間は、穴2つまで飛び越えられる)
		// void SetJumpBoost(bool hasJumpBoost) { hasJumpBoost_ = hasJumpBoost; }
		// bool HasJumpBoost() const { return hasJumpBoost_; }

	private:
		// [アイテム] アイテムは一旦使わない。コメントアウト
		// //2マス乗り越えのアイテムを使い切るか決める (from から landing へ跳び始めたとき)
		// void UseJumpBoost(const Cell& from, const Cell& landing);

		//見た目 (仮で使ってます（涙） Kari/Model/cube.obj の木箱)
		std::unique_ptr<RyoEngine::Model> body_{};  // 体
		std::unique_ptr<RyoEngine::Model> front_{}; // 正面の板 (向きがわかるように色を変える)

		//状態
		RyoEngine::Vector3 position_{};          // 足元の位置
		Cell facing_ = { 0, 1 };                 // 向き (最初は奥)
		float verticalSpeed_ = 0.0f;             // 上下の速さ (上が +。落ちるとき・はじかれたとき)
		bool isGrounded_ = true;                 // 床の上に立っているか

		//1マス跳ぶ
		bool isJumping_ = false;
		RyoEngine::Vector3 jumpOrigin_{};
		Cell jumpDestination_{};
		float jumpElapsed_ = 0.0f;

		//はじかれる
		RyoEngine::Vector3 knockbackVelocity_{};
		float knockbackTime_ = 0.0f;
		bool isKnockbackOverFence_ = false;      //床のふちで止めないか

		bool isVisible_ = true;
		// [アイテム] アイテムは一旦使わない。コメントアウト
		// bool hasJumpBoost_ = false;              //2マス乗り越えのアイテムを持っているか
	};
}
