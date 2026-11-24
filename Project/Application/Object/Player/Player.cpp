#include "Player.h"
#include "../../Config/GameConfig.h"
#include "../../Stage/Floor/Floor.h"
#include "../../Stage/Grid/GridDirection.h"
#include "../ModelColor.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

using namespace RyoEngine;

namespace App
{
	//mtlの色のままのプレイヤーのモデルを作る
	void Player::Initialize()
	{
		body_ = Model::Create(GameConfig::kPlayerModel);
		ModelColor::Apply(*body_, GameConfig::kPlayerModel);

		//着地でピカッと光るシェーダーもつけるぽよん
		bounce_.Initialize(*body_);
	}

	//位置に置いて動きの状態を全部戻す
	void Player::Place(const Vector3& position)
	{
		position_ = position;
		facing_ = { 0, 1 };
		verticalSpeed_ = 0.0f;
		isJumping_ = false;
		knockbackTime_ = 0.0f;
		knockbackVelocity_ = { 0.0f, 0.0f, 0.0f };
		isKnockbackOverFence_ = false;
		isGrounded_ = true;
	}

	//向きを変える
	void Player::SetFacing(const Cell& facing)
	{
		facing_ = facing;
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

	//[アイテム] アイテムは一旦使わないのでコメントアウト
	// //跳び始めたときに2マス乗り越えを使い切るか決める
	// void Player::UseJumpBoost(const Cell& from, const Cell& landing)
	// {
		// if (!hasJumpBoost_)
		// {
			// return;
		// }
		// //次のジャンプで必ず使い切る
		// if (GameConfig::kJumpBoostUseOnNextJump)
		// {
			// hasJumpBoost_ = false;
			// return;
		// }
		// //3マス先に着地して穴2つを飛び越えたときだけ使い切る
		// const int32_t distance = std::abs(landing.x - from.x) + std::abs(landing.z - from.z);
		// if (distance > GameConfig::kNormalJumpGap + 1)
		// {
			// hasJumpBoost_ = false;
		// }
	// }

	//1フレーム動かす
	bool Player::Advance(const Vector2& move, bool hasMovement, const Floor& floor, float deltaTime)
	{
		//床の高さと重力と今の位置とマス
		const float floorHeight = floor.GetHeight();
		const float gravity = std::max(0.01f, GameConfig::kGravity);
		Vector3 position = position_;
		Cell cell = floor.GetCellAt(position);

		//立っていたタイルが崩れていたら床から離れる
		if (!floor.IsSolid(cell))
		{
			isGrounded_ = false;
		}

		//移動の入力で上下左右のどれかへ1マス跳ぶ
		if (isGrounded_ && knockbackTime_ <= 0.0f && hasMovement)
		{
			//いつも飛び越えられる穴の数
			const Cell direction = GridDirection::FromInput(move);
			const int32_t maxGap = GameConfig::kNormalJumpGap;
			//[アイテム] 2マス乗り越えを持っていたら2にするのでコメントアウト
			// if (hasJumpBoost_)
			// {
			// 	maxGap = GameConfig::kBoostedJumpGap;
			// }
			const Cell landing = floor.JumpLanding(cell, direction, maxGap);
			//床の中なら跳び、飛び越えられない穴へ跳んだら落ちる
			if (floor.Contains(cell + direction))
			{
				//[アイテム] アイテムは一旦使わないのでコメントアウト
				// UseJumpBoost(cell, landing);
				jumpOrigin_ = position;
				jumpDestination_ = landing;
				jumpElapsed_ = 0.0f;
				verticalSpeed_ = 0.0f;
				isJumping_ = true;
				isGrounded_ = false;
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
			//着地先が崩れていたらそのまま落ちる
			isJumping_ = false;
			isGrounded_ = floor.IsSolid(jumpDestination_);
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
		//足元を動かさずに縦にのびたりつぶれたりする
		bounce_.Update(position_);
		const float scaleY = bounce_.GetScaleY();
		const float scaleXZ = bounce_.GetScaleXZ();

		//足の裏がposition_に来るように置いて、向いている方へ回す
		const float scale = GameConfig::kPlayerModelScale;
		const float yawDeg = GridDirection::ToYawDeg(facing_) + GameConfig::kPlayerModelYaw;
		const float lift = GameConfig::kPlayerModelLift * scale * scaleY;
		body_->SetScale({ scale * scaleXZ, scale * scaleY, scale * scaleXZ });
		body_->SetRotate({ 0.0f, yawDeg, 0.0f });
		body_->SetTranslate({ position_.x, position_.y + lift, position_.z });
		body_->TransferMatrix();
	}

	//点滅中以外の体と、いつものほこりを描く
	void Player::Draw()
	{
		bounce_.DrawDust();
		if (!isVisible_)
		{
			return;
		}
		body_->Draw();
	}
}
