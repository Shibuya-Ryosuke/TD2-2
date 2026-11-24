#pragma once
#include <RyoEngine.h>
#include <memory>
#include "../../Stage/Grid/Cell.h"
#include "../../Direction/Player/PlayerBounce.h"

namespace App
{
	class Floor;
	//入力の向きへ1マスずつ跳んで動き、はじかれたり落ちたりするプレイヤー
	class Player
	{
	public:
		Player() = default;
		~Player() = default;

		Player(const Player&) = delete;
		Player& operator=(const Player&) = delete;

		void Initialize(); //mtlの色のままのプレイヤーのモデルを作る

		//1フレーム動かして、落ちたらtrueを返す
		bool Advance(const RyoEngine::Vector2& move, bool hasMovement, const Floor& floor, float deltaTime);

		//位置に置いて動きの状態を全部戻す
		void Place(const RyoEngine::Vector3& position);

		//上下左右のどれかの向きにはじく
		void ApplyKnockback(const Floor& floor, const RyoEngine::Vector3& direction, float speed, float duration, float liftSpeed, bool overFence);

		//上下左右のどれかに向きを変える
		void SetFacing(const Cell& facing);
		//今の向き
		const Cell& GetFacing() const { return facing_; }

		//動かし終わったあとに行列を確定する
		void UpdateVisuals();
		void Draw(); //プレイヤーのモデルを描く

		const RyoEngine::Vector3& GetPosition() const { return position_; }
		//下の層へ降りる途中などで位置だけ動かす
		void SetPosition(const RyoEngine::Vector3& position) { position_ = position; }

		bool IsGrounded() const { return isGrounded_; }
		bool IsJumping() const { return isJumping_; }
		float GetLandingPower() const { return bounce_.GetLandingPower(); } //足場のぷるるんに使うこのフレームに着地した強さ
		bool IsKnockedBack() const { return knockbackTime_ > 0.0f; }
		//跳んでいる先のマス
		const Cell& GetJumpDestination() const { return jumpDestination_; }
		//跳び始めた位置
		const RyoEngine::Vector3& GetJumpOrigin() const { return jumpOrigin_; }

		//無敵の間の点滅で描くか
		void SetVisible(bool isVisible) { isVisible_ = isVisible; }

		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// //持っている間は穴2つまで飛び越えられる2マス乗り越え
		// void SetJumpBoost(bool hasJumpBoost) { hasJumpBoost_ = hasJumpBoost; }
		// bool HasJumpBoost() const { return hasJumpBoost_; }

	private:
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// //跳び始めたときに2マス乗り越えを使い切るか決める
		// void UseJumpBoost(const Cell& from, const Cell& landing);

		//仮の木箱の見た目
		std::unique_ptr<RyoEngine::Model> body_{}; //プレイヤーのモデル
		PlayerBounce bounce_; //跳ぶと着地のぽよん

		//状態
		RyoEngine::Vector3 position_{}; //足元の位置
		Cell facing_ = { 0, 1 }; //最初は奥の向き
		float verticalSpeed_ = 0.0f; //上がプラスの上下の速さ
		bool isGrounded_ = true; //床の上に立っているか

		//1マス跳ぶ
		bool isJumping_ = false;
		RyoEngine::Vector3 jumpOrigin_{};
		Cell jumpDestination_{};
		float jumpElapsed_ = 0.0f;

		//はじかれる
		RyoEngine::Vector3 knockbackVelocity_{};
		float knockbackTime_ = 0.0f;
		bool isKnockbackOverFence_ = false; //床のふちで止めないか

		bool isVisible_ = true;
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// bool hasJumpBoost_ = false;              //2マス乗り越えを持っているか
	};
}
