#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <memory>
#include "../../Stage/Grid/Cell.h"
#include "../../Direction/Player/HammerSwing.h"
#include "../../Direction/Player/PlayerAnimation.h"
#include "../../Direction/Player/PlayerBounce.h"
#include "../../Direction/Player/PlayerPose.h"
#include "../../Direction/Player/PlayerTurnAround.h"
#include "PlayerArms.h"
#include "Rig/PlayerRig.h"

namespace App
{
	class Floor;
	//入力の向きのとなりのマスへ跳んで動き、はじかれたり落ちたりするプレイヤー
	class Player
	{
	public:
		Player() = default;
		~Player() = default;

		Player(const Player&) = delete;
		Player& operator=(const Player&) = delete;

		void Initialize(); //glbの色のままのアニメーションするプレイヤーのモデルとハンマーを作る

		//1フレーム動かして、落ちたらtrueを返す
		bool Advance(const RyoEngine::Vector2& move, bool hasMovement, const Floor& floor, float deltaTime);

		//位置に置いて動きの状態を全部戻す
		void Place(const RyoEngine::Vector3& position);

		//上下左右のどれかの向きにはじく
		void ApplyKnockback(const Floor& floor, const RyoEngine::Vector3& direction, float speed, float duration, float liftSpeed, bool overFence);

		//斜めもある向きを向いて、床を崩す向きはそれを上下左右にそろえる
		void SetFacingDirection(const RyoEngine::Vector3& direction);
		//足場を壊すときにハンマーを持って振り下ろす
		void StartSwing();
		//クリアのときにyawDeg度の向きを向いて待機する
		void TurnForPose(float yawDeg);
		//クリアのポーズを1回する
		void StartPose();
		//クリアのポーズが終わったか
		bool IsPoseFinished() const { return animation_.IsPoseFinished(); }
		//落ちるときのじたばたや尻もちの姿勢にして、Normalでいつもに戻す
		void SetPose(PlayerPose::Kind kind);
		void SetPoseSpin(float spin) { pose_.SetSpin(spin); }
		void ApplyDefeatPose(const RigPose& pose, const RyoEngine::Vector3& position, float yaw);
		//足元からハートをはじけさせる
		void BurstHearts(float power) { bounce_.Burst(position_, power); }
		void BurstHammerHearts(const RyoEngine::Vector3& contact);
		//atから強さpowerと大きさの倍率sizeScaleでハートをはじけさせる
		void BurstHeartsAt(const RyoEngine::Vector3& at, float power, float sizeScale) { bounce_.Burst(at, power, sizeScale); }
		void PlayVictoryParticles(const RyoEngine::Vector3& center, const RyoEngine::Vector3& right, const RyoEngine::Vector3& up, const RyoEngine::Vector3& forward);
		void PlayLandingParticles() const;
		//このフレームでハンマーが床に当たったか
		bool WasHammerImpact() const { return animation_.WasImpact(); }
		//trueなら見えない壁がなくなり、穴にも床の外へも跳べて落ちる
		void SetCanLeaveFloor(bool canLeave) { canLeaveFloor_ = canLeave; }
		//跳び始めた回数
		int32_t GetJumpCount() const { return jumpCount_; }
		//床を崩す上下左右の向き
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

		//無敵の間の点滅で描くか
		void SetVisible(bool isVisible) { isVisible_ = isVisible; }

		//[アイテム] アイテムは一旦使わないのでコメントアウト
		//ジャンプ強化中は穴を2マスまで飛び越える
		// void SetJumpBoost(bool hasJumpBoost) { hasJumpBoost_ = hasJumpBoost; }
		// bool HasJumpBoost() const { return hasJumpBoost_; }

	private:
		//見た目
		std::unique_ptr<RyoEngine::Model> body_{}; //ボーンつきのプレイヤーのモデル
		PlayerAnimation animation_; //待機と移動と足場を壊すとポーズのアニメーション
		HammerSwing hammer_; //足場を壊すときだけ右手に持つハンマー
		PlayerBounce bounce_; //着地のピカッとほこり
		PlayerTurnAround turnAround_; //向きが変わったときにクルッと回る見た目の向き
		PlayerPose pose_; //じたばたと尻もち
		PlayerRig rig_; //じたばたと尻もちの間だけ見せる分けたパーツのプレイヤー
		PlayerArms arms_; //ボーンつきのモデルの腕を肩から大きくする
		bool isDefeatPose_ = false; //やられ演出中はパーツの姿勢を描く
		bool isPosing_ = false; //クリアのポーズでカメラの方を向いているか
		float poseYawDeg_ = 0.0f; //クリアのポーズで向く度数

		//状態
		RyoEngine::Vector3 position_{}; //足元の位置
		Cell facing_ = { 0, 1 }; //床を崩す上下左右の向きで最初は奥
		float facingYawDeg_ = 0.0f; //斜めもある向きの度数で最初は奥
		float floorHeight_ = 0.0f; //ハンマーが床にめり込まないようにする今の床の高さ
		float verticalSpeed_ = 0.0f; //上がプラスの上下の速さ
		bool isGrounded_ = true; //床の上に立っているか

		//となりのマスへ跳ぶ
		bool isJumping_ = false;
		RyoEngine::Vector3 jumpOrigin_{};
		Cell jumpDestination_{};
		float jumpElapsed_ = 0.0f;

		//はじかれる
		RyoEngine::Vector3 knockbackVelocity_{};
		float knockbackTime_ = 0.0f;
		bool isKnockbackOverFence_ = false; //床のふちで止めないか

		bool isVisible_ = true;
		bool canLeaveFloor_ = false; //穴と床の外へも跳べるか
		int32_t jumpCount_ = 0; //跳び始めた回数
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// bool hasJumpBoost_ = false;              //2マス乗り越えを持っているか
	};
}
