#include "Player.h"
#include "../../Config/DirectionConfig.h"
#include "../../Direction/Player/PlayerParticles.h"
#include "../../Config/PlayerConfig.h"
#include "../../Stage/Floor/Floor.h"
#include "../../Stage/Grid/GridDirection.h"
#include "../ModelColor.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//glbの色のままのアニメーションするプレイヤーのモデルとハンマーを作る
	void Player::Initialize()
	{
		body_ = Model::Create(PlayerConfig::kPlayerModel);
		ModelColor::Apply(*body_, PlayerConfig::kPlayerModel);

		//待機から始めるアニメーションと、右手に持たせるハンマー
		animation_.Initialize(*body_);
		hammer_.Initialize(*body_);

		//着地でピカッと光るシェーダーをつける着地の演出と、じたばたと尻もちで見せる分けたパーツ
		bounce_.Initialize(*body_);
		rig_.Initialize();

		//体のシェーダーをつけたあとに、腕のメッシュだけ肩から大きくするシェーダーにする
		arms_.Initialize(*body_, PlayerConfig::kPlayerModel);
	}

	//位置に置いて動きの状態を全部戻す
	void Player::Place(const Vector3& position)
	{
		position_ = position;
		facing_ = { 0, 1 };
		facingYawDeg_ = 0.0f;
		floorHeight_ = position.y;
		verticalSpeed_ = 0.0f;
		isJumping_ = false;
		knockbackTime_ = 0.0f;
		knockbackVelocity_ = { 0.0f, 0.0f, 0.0f };
		isKnockbackOverFence_ = false;
		isGrounded_ = true;
		turnAround_.Reset(facingYawDeg_);
		isDefeatPose_ = false;
		isPosing_ = false;
		pose_.Reset();
		animation_.PlayIdle();
		hammer_.Hide();
	}

	//落ちるときのじたばたや尻もちは待機の最初の姿勢で止めてから脚を動かす
	void Player::SetPose(PlayerPose::Kind kind)
	{
		if (kind == pose_.Get())
		{
			return;
		}
		pose_.Set(kind);
		if (kind == PlayerPose::Kind::Normal)
		{
			animation_.PlayIdle();
			return;
		}
		animation_.Freeze();
		hammer_.Hide();
	}

	//足場を壊すときにハンマーを持って振り下ろす
	void Player::StartSwing()
	{
		animation_.PlaySwing();
		hammer_.Start();
	}

	//クリアのときにyawDeg度の向きを向いて待機する
	void Player::TurnForPose(float yawDeg)
	{
		isPosing_ = true;
		poseYawDeg_ = yawDeg;
		animation_.PlayIdle();
		hammer_.Hide();
	}

	//クリアのポーズを1回する
	void Player::StartPose()
	{
		animation_.PlayPose();
	}

	//斜めもある向きを向いて、床を崩す向きは上下左右にそろえる
	void Player::SetFacingDirection(const Vector3& direction)
	{
		if (direction.x * direction.x + direction.z * direction.z < 0.0001f)
		{
			return;
		}
		facingYawDeg_ = GridDirection::ToYawDeg(direction);
		facing_ = GridDirection::FromVector(direction);
	}

	//上下左右の向きにはじく
	void Player::ApplyKnockback(const Floor& floor, const Vector3& direction, float speed, float duration, float liftSpeed, bool overFence)
	{
		isJumping_ = false;

		//ふちを越えてはじかれたら床の上に戻るまでふちで止めない
		if (overFence)
		{
			isKnockbackOverFence_ = true;
		}
		else if (isKnockbackOverFence_)
		{
			isKnockbackOverFence_ = !floor.Contains(floor.GetCellAt(position_));
		}

		//はじく向きを上下左右にそろえる
		const Vector3 offset = floor.StepOffset(GridDirection::FromVector(direction));
		knockbackVelocity_ = Normalize(offset) * speed;
		knockbackTime_ = duration;
		verticalSpeed_ = liftSpeed;
		isGrounded_ = false;
	}

	//1フレーム動かす
	bool Player::Advance(const Vector2& move, bool hasMovement, const Floor& floor, float deltaTime)
	{
		//床の高さと重力と今の位置とマス
		const float floorHeight = floor.GetHeight();
		floorHeight_ = floorHeight;
		const float gravity = std::max(0.01f, PlayerConfig::kGravity);
		Vector3 position = position_;
		Cell cell = floor.GetCellAt(position);

		//立っていたタイルが崩れていたら床から離れる
		if (!floor.IsSolid(cell))
		{
			isGrounded_ = false;
		}

		//移動の入力で上下左右のとなりのマスへ跳び、押しっぱなしなら着地してすぐ次へ跳ぶ
		if (isGrounded_ && knockbackTime_ <= 0.0f && hasMovement && !isJumping_)
		{
			//ふだんは見えない壁があって穴と床の外へは跳ばず、チュートリアルだけは飛び降りられる
			const Cell destination = cell + GridDirection::FromInput(move);
			const bool isStandable = floor.Contains(destination) && !floor.IsHole(destination);
			if (isStandable || canLeaveFloor_)
			{
				++jumpCount_;
				jumpOrigin_ = position;
				jumpDestination_ = destination;
				jumpElapsed_ = 0.0f;
				verticalSpeed_ = 0.0f;
				isJumping_ = true;
				isGrounded_ = false;
				//ハンマーを振っている間は振るアニメーションのまま跳ぶ
				animation_.PlayJump();
			}
		}

		//マスの線にそって山なりに跳ぶ
		if (isJumping_)
		{
			jumpElapsed_ += deltaTime;
			const float t = std::clamp(jumpElapsed_ / PlayerConfig::kJumpDuration, 0.0f, 1.0f);
			Vector3 jumpPosition = Floor::AlongGrid(jumpOrigin_, floor.GetCellCenter(jumpDestination_), t);
			jumpPosition.y += 4.0f * PlayerConfig::kJumpHeight * t * (1.0f - t);
			position_ = jumpPosition;
			if (t < 1.0f)
			{
				return false;
			}
			//着地先が穴ならそのままの勢いで落ちていく
			isJumping_ = false;
			isGrounded_ = floor.IsSolid(jumpDestination_);
			if (isGrounded_)
			{
				PlayLandingParticles();
			}
			else
			{
				verticalSpeed_ = -4.0f * PlayerConfig::kJumpHeight / PlayerConfig::kJumpDuration;
			}
			return false;
		}

		//はじかれている間はその速さで動く
		if (knockbackTime_ > 0.0f)
		{
			position += knockbackVelocity_ * std::min(deltaTime, knockbackTime_);
			knockbackTime_ = std::max(0.0f, knockbackTime_ - deltaTime);
		}

		//ボスに触れてはじかれたときと、飛び降りられるとき以外は床のふちで止める
		if (!isKnockbackOverFence_ && !canLeaveFloor_)
		{
			position = floor.ClampToBoundary(position, PlayerConfig::kPlayerContactRadius);
		}

		//今のマスに床があるか
		cell = floor.GetCellAt(position);
		const bool isSolid = floor.IsSolid(cell);
		if (!isSolid)
		{
			isGrounded_ = false;
		}

		//空中は重力で動かして、床の高さを通り過ぎたら着地
		if (!isGrounded_)
		{
			const float previousY = position.y;
			position.y += (verticalSpeed_ - 0.5f * gravity * deltaTime) * deltaTime;
			verticalSpeed_ -= gravity * deltaTime;

			if (isSolid && verticalSpeed_ <= 0.0f && previousY >= floorHeight && position.y <= floorHeight)
			{
				position.y = floorHeight;
				verticalSpeed_ = 0.0f;
				isGrounded_ = true;
				isKnockbackOverFence_ = false;
				PlayerParticles::Landing(position);
			}
		}

		//位置を確定する
		position_ = position;

		//床より下まで落ちたら落ちたことにする
		return position.y < floorHeight - PlayerConfig::kFallThreshold;
	}

	//ハンマーが当たった位置からハートを出す
	void Player::BurstHammerHearts(const Vector3& contact)
	{
		bounce_.Burst(contact, DirectionConfig::kHammerImpactHearts, DirectionConfig::kHammerHeartSizeScale);
	}

	//顔の前を空けて肩の外で淡いハートと白い星を弾けさせる
	void Player::PlayVictoryParticles(const Vector3& center, const Vector3& right, const Vector3& up, const Vector3& forward)
	{
		const Vector3 foreground = center - forward * DirectionConfig::kVictoryForward + up * DirectionConfig::kVictoryLift;
		const Vector3 left = right * -1.0f;
		const Vector3 leftShoulder = foreground + left * DirectionConfig::kVictoryShoulder;
		const Vector3 rightShoulder = foreground + right * DirectionConfig::kVictoryShoulder;
		bounce_.BurstVictory(leftShoulder, left, up);
		bounce_.BurstVictory(rightShoulder, right, up);
		PlayerParticles::VictoryStars(leftShoulder + left * DirectionConfig::kVictoryStarOut);
		PlayerParticles::VictoryStars(rightShoulder + right * DirectionConfig::kVictoryStarOut);
	}

	//足元からピンクの光を弾けさせる
	void Player::PlayLandingParticles() const
	{
		PlayerParticles::Landing(position_);
	}

	//見た目
	void Player::UpdateVisuals()
	{
		//着地のピカッとほこりと、終わったアニメーションを待機に戻す
		bounce_.Update(position_);
		animation_.Update();
		pose_.Update(GetDeltaTime());

		//向いている向きへクルッと回る
		float lookYawDeg = facingYawDeg_;
		//クリアのポーズのときはカメラの方を向く
		if (isPosing_)
		{
			lookYawDeg = poseYawDeg_;
		}
		turnAround_.Update(lookYawDeg);

		//足の裏がposition_に来るように置いて、見た目の向きへ回す
		const float scale = PlayerConfig::kPlayerModelScale;
		const float yawDeg = turnAround_.GetYawDeg() + PlayerConfig::kPlayerModelYaw;
		const float lift = PlayerConfig::kPlayerModelLift * scale;
		body_->SetScale({ scale, scale, scale });
		body_->SetRotate({ 0.0f, yawDeg, 0.0f });
		body_->SetTranslate({ position_.x, position_.y + lift, position_.z });
		body_->TransferMatrix();
		arms_.Update();

		//じたばたと尻もちの間は分けたパーツをコードで動かす
		if (pose_.IsRigVisible())
		{
			rig_.Apply(pose_.MakeRigPose(), position_, turnAround_.GetYawDeg(), scale);
		}

		//右手について振り、頭が床より下に行かないようにするハンマー
		hammer_.Update(animation_.IsSwinging(), floorHeight_, arms_);
	}

	//やられ演出の姿勢をパーツのモデルに反映する
	void Player::ApplyDefeatPose(const RigPose& pose, const Vector3& position, float yaw)
	{
		isDefeatPose_ = true;
		position_ = position;
		rig_.Apply(pose, position_, yaw, PlayerConfig::kPlayerModelScale);
	}

	//点滅中以外の体とハンマーと、いつものほこりを描く
	void Player::Draw()
	{
		bounce_.DrawHearts();
		if (isDefeatPose_)
		{
			rig_.Draw();
			return;
		}
		if (!isVisible_)
		{
			return;
		}
		if (pose_.IsRigVisible())
		{
			rig_.Draw();
			return;
		}
		body_->Draw();
		hammer_.Draw();
	}
}
