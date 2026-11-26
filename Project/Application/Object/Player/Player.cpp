#include "Player.h"
#include "../../Config/GameConfig.h"
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
		body_ = Model::Create(GameConfig::kPlayerModel);
		ModelColor::Apply(*body_, GameConfig::kPlayerModel);

		//待機から始めるアニメーションと、右手に持たせるハンマー
		animation_.Initialize(*body_);
		hammer_.Initialize(*body_);

		//着地でピカッと光るシェーダーもつける着地の演出
		bounce_.Initialize(*body_);
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
		isPosing_ = false;
		animation_.PlayIdle();
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
		const float gravity = std::max(0.01f, GameConfig::kGravity);
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
			//穴は飛び越えず、もう穴になっているマスへは跳ばない
			const Cell destination = cell + GridDirection::FromInput(move);
			if (floor.Contains(destination) && !floor.IsHole(destination))
			{
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
			const float t = std::clamp(jumpElapsed_ / GameConfig::kJumpDuration, 0.0f, 1.0f);
			Vector3 jumpPosition = Floor::AlongGrid(jumpOrigin_, floor.GetCellCenter(jumpDestination_), t);
			jumpPosition.y += 4.0f * GameConfig::kJumpHeight * t * (1.0f - t);
			position_ = jumpPosition;
			if (t < 1.0f)
			{
				return false;
			}
			//着地先が穴ならそのままの勢いで落ちていく
			isJumping_ = false;
			isGrounded_ = floor.IsSolid(jumpDestination_);
			if (!isGrounded_)
			{
				verticalSpeed_ = -4.0f * GameConfig::kJumpHeight / GameConfig::kJumpDuration;
			}
			return false;
		}

		//はじかれている間はその速さで動く
		if (knockbackTime_ > 0.0f)
		{
			position += knockbackVelocity_ * std::min(deltaTime, knockbackTime_);
			knockbackTime_ = std::max(0.0f, knockbackTime_ - deltaTime);
		}

		//ボスに触れてはじかれたとき以外は床のふちで止める
		if (!isKnockbackOverFence_)
		{
			position = floor.ClampToBoundary(position, GameConfig::kPlayerContactRadius);
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
			}
		}

		//位置を確定する
		position_ = position;

		//床より下まで落ちたら落ちたことにする
		return position.y < floorHeight - GameConfig::kFallThreshold;
	}

	//見た目
	void Player::UpdateVisuals()
	{
		//着地のピカッとほこりと、終わったアニメーションを待機に戻す
		bounce_.Update(position_);
		animation_.Update();

		//向いている向きへクルッと回る
		float lookYawDeg = facingYawDeg_;
		//クリアのポーズのときはカメラの方を向く
		if (isPosing_)
		{
			lookYawDeg = poseYawDeg_;
		}
		turnAround_.Update(lookYawDeg);

		//足の裏がposition_に来るように置いて、見た目の向きへ回す
		const float scale = GameConfig::kPlayerModelScale;
		const float yawDeg = turnAround_.GetYawDeg() + GameConfig::kPlayerModelYaw;
		const float lift = GameConfig::kPlayerModelLift * scale;
		body_->SetScale({ scale, scale, scale });
		body_->SetRotate({ 0.0f, yawDeg, 0.0f });
		body_->SetTranslate({ position_.x, position_.y + lift, position_.z });
		body_->TransferMatrix();

		//右手について振り、頭が床より下に行かないようにするハンマー
		hammer_.Update(animation_.IsSwinging(), floorHeight_);
	}

	//点滅中以外の体とハンマーと、いつものほこりを描く
	void Player::Draw()
	{
		bounce_.DrawDust();
		if (!isVisible_)
		{
			return;
		}
		body_->Draw();
		hammer_.Draw();
	}
}
