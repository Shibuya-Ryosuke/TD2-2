#include "Player.h"
#include "../../Config/GameConfig.h"
#include "../../Stage/Floor/Floor.h"
#include "../../Stage/Grid/GridDirection.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		constexpr float kFrontThickness = 0.04f; // 正面の板の厚さ
	}

	// モデルを作る (体と正面の板)
	void Player::Initialize()
	{
		// 体 (白い箱)
		body_ = Model::Create(GameConfig::kCubeModel);
		body_->SetColor(GameConfig::kPlayerColor);

		// 正面の板 (向きがわかるように色を変える)
		front_ = Model::Create(GameConfig::kCubeModel);
		front_->SetColor(GameConfig::kPlayerFrontColor);
	}

	// 位置に置いて、動きの状態を全部戻す
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

	// 向きを変える
	void Player::SetFacing(const Cell& facing)
	{
		facing_ = facing;
	}

	// はじく (向きは上下左右にそろえる)
	void Player::ApplyKnockback(const Floor& floor, const Vector3& direction, float speed, float duration, float liftSpeed, bool overFence)
	{
		isJumping_ = false;

		// 一度ふちを越えてはじかれたら、床の上に戻るまではふちで止めない
		if (overFence)
		{
			isKnockbackOverFence_ = true;
		}
		else if (isKnockbackOverFence_)
		{
			isKnockbackOverFence_ = !floor.Contains(floor.GetCellAt(position_));
		}

		// はじく向きは上下左右のどれかにそろえる
		const Vector3 offset = floor.StepOffset(GridDirection::FromVector(direction));
		knockbackVelocity_ = Normalize(offset) * speed;
		knockbackTime_ = duration;
		verticalSpeed_ = liftSpeed;
		isGrounded_ = false;
	}

	// [アイテム] アイテムは一旦使わない。コメントアウト
	// // 2マス乗り越えのアイテムを使い切るか決める (跳び始めたときに呼ぶ)
	// void Player::UseJumpBoost(const Cell& from, const Cell& landing)
	// {
		// if (!hasJumpBoost_)
		// {
			// return;
		// }
		// // 次のジャンプで必ず使い切る
		// if (GameConfig::kJumpBoostUseOnNextJump)
		// {
			// hasJumpBoost_ = false;
			// return;
		// }
		// // 穴2つを飛び越えた (3マス先に着地した) ときだけ使い切る
		// const int32_t distance = std::abs(landing.x - from.x) + std::abs(landing.z - from.z);
		// if (distance > GameConfig::kNormalJumpGap + 1)
		// {
			// hasJumpBoost_ = false;
		// }
	// }

	// 1フレーム動かす
	bool Player::Advance(const Vector2& move, bool hasMovement, const Floor& floor, float deltaTime)
	{
		// 床の高さ・重力・今の位置とマス
		const float floorHeight = floor.GetHeight();
		const float gravity = std::max(0.01f, GameConfig::kGravity);
		Vector3 position = position_;
		Cell cell = floor.GetCellAt(position);

		// 立っていたタイルが崩れていたら、床から離れる
		if (!floor.IsSolid(cell))
		{
			isGrounded_ = false;
		}

		// 移動の入力: 上下左右のどれかへ1マス跳ぶ (跳んだ先が床のときだけ)
		if (isGrounded_ && knockbackTime_ <= 0.0f && hasMovement)
		{
			// 飛び越えられる穴の数 (いつもは 1)
			const Cell direction = GridDirection::FromInput(move);
			const int32_t maxGap = GameConfig::kNormalJumpGap;
			// [アイテム] アイテムは一旦使わない。コメントアウト (2マス乗り越えを持っていたら 2)
			// if (hasJumpBoost_)
			// {
			// 	maxGap = GameConfig::kBoostedJumpGap;
			// }
			const Cell landing = floor.JumpLanding(cell, direction, maxGap);
			if (floor.IsSolid(landing))
			{
				// [アイテム] アイテムは一旦使わない。コメントアウト
				// UseJumpBoost(cell, landing);
				jumpOrigin_ = position;
				jumpDestination_ = landing;
				jumpElapsed_ = 0.0f;
				verticalSpeed_ = 0.0f;
				isJumping_ = true;
				isGrounded_ = false;
			}
		}

		//跳んでいる途中: マスの線にそって進みながら、山なりに上下する
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
			// 着地 (跳んでいる間に着地先が崩れていたら、そのまま落ちる)
			isJumping_ = false;
			isGrounded_ = floor.IsSolid(jumpDestination_);
			return false;
		}

		// はじかれている間は、その速さで動く
		if (knockbackTime_ > 0.0f)
		{
			position += knockbackVelocity_ * std::min(deltaTime, knockbackTime_);
			knockbackTime_ = std::max(0.0f, knockbackTime_ - deltaTime);
		}

		// 床のふちで止める (ボスに触れてはじかれたときは止めない)
		if (!isKnockbackOverFence_)
		{
			position = floor.ClampToBoundary(position, GameConfig::kPlayerContactRadius);
		}

		// 今のマスに床があるか
		cell = floor.GetCellAt(position);
		const bool isSolid = floor.IsSolid(cell);
		if (!isSolid)
		{
			isGrounded_ = false;
		}

		// 空中: 重力で動かして、上から床の高さを通り過ぎたら着地
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

		// 位置を確定する
		position_ = position;

		// 床の高さより下まで落ちたら「落ちた」
		return position.y < floorHeight - GameConfig::kFallThreshold;
	}

	// 見た目
	void Player::UpdateVisuals()
	{
		// cube.obj は1辺が 2 なので、合わせて1辺を kPlayerSize にする。足元が position_ になるように上げる
		const float size = GameConfig::kPlayerSize;
		const float scale = size / GameConfig::kCubeModelSize;
		const float yawDeg = GridDirection::ToYawDeg(facing_);
		const Vector3 rotate = { 0.0f, yawDeg, 0.0f };
		const Vector3 center = { position_.x, position_.y + size * 0.5f, position_.z };

		//体
		body_->SetScale({ scale, scale, scale });
		body_->SetRotate(rotate);
		body_->SetTranslate(center);
		body_->TransferMatrix();

		//正面の板: 体の正面の面にぴったり重ねる (少しだけ前に出して、体の面と重ならないようにする)
		const Vector3 forward = { static_cast<float>(facing_.x), 0.0f, static_cast<float>(facing_.z) };
		const float frontDistance = size * 0.5f + kFrontThickness * 0.5f;
		front_->SetScale({ scale, scale, kFrontThickness / GameConfig::kCubeModelSize });
		front_->SetRotate(rotate);
		front_->SetTranslate(center + forward * frontDistance);
		front_->TransferMatrix();
	}

	//体と正面の板を描く (点滅中は描かないことがある)
	void Player::Draw()
	{
		if (!isVisible_)
		{
			return;
		}
		body_->Draw();
		front_->Draw();
	}
}
