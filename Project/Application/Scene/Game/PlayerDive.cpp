#include "PlayerDive.h"
#include "../../Config/BossFallConfig.h"
#include "../../Object/Player/Player.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//fromから穴の上のholeへ飛んで、下の層のlandingへ落ち始める
	void PlayerDive::Begin(const Vector3& from, const Vector3& hole, const Vector3& landing, Player& player)
	{
		from_ = from;
		hole_ = hole;
		landing_ = landing;
		position_ = from;
		timer_ = 0.0f;
		isActive_ = true;
		player.SetPose(PlayerPose::Kind::Flail);
	}

	//山なりに穴の上へ飛んでから下の層へ落ちて、横も縦も飛ぶときの速さのままなめらかにつなぐ
	bool PlayerDive::Update(float deltaTime, Player& player)
	{
		if (!isActive_)
		{
			return false;
		}
		timer_ += deltaTime;
		const float hopTime = BossFallConfig::kDiveHopTime;
		const float fallTime = BossFallConfig::kDiveFallTime;
		const Vector3 hopMove = { hole_.x - from_.x, 0.0f, hole_.z - from_.z };
		const float hopLength = std::sqrt(hopMove.x * hopMove.x + hopMove.z * hopMove.z);
		if (timer_ < hopTime)
		{
			//横は同じ速さで、縦は山なり
			const float t = timer_ / hopTime;
			position_ = from_ + (hole_ - from_) * t;
			position_.y += 4.0f * BossFallConfig::kDiveHopHeight * t * (1.0f - t);
		}
		else
		{
			const float t = std::clamp((timer_ - hopTime) / fallTime, 0.0f, 1.0f);

			//横は飛んできた速さから始めて、着地点で止まりすぎないように進む
			const Vector3 fallMove = { landing_.x - hole_.x, 0.0f, landing_.z - hole_.z };
			const float fallLength = std::sqrt(fallMove.x * fallMove.x + fallMove.z * fallMove.z);
			const float startDistance = hopLength / hopTime * fallTime;
			float slide = fallLength * (1.0f - (1.0f - t) * (1.0f - t));
			if (fallLength >= startDistance * 0.5f)
			{
				slide = startDistance * t + (fallLength - startDistance) * t * t;
			}
			Vector3 horizontal = { hole_.x, 0.0f, hole_.z };
			if (fallLength > 0.0001f)
			{
				horizontal = horizontal + fallMove * (slide / fallLength);
			}

			//縦は山なりの下りの速さから始めて、だんだん速く落ちる
			const float drop = hole_.y - landing_.y;
			float linear = 1.0f;
			if (drop > 0.0001f)
			{
				linear = std::clamp(4.0f * BossFallConfig::kDiveHopHeight / hopTime * fallTime / drop, 0.0f, 1.0f);
			}
			const float fall = linear * t + (1.0f - linear) * t * t;
			position_ = { horizontal.x, hole_.y - drop * fall, horizontal.z };
		}

		//じたばたしながらくるくる回る
		player.SetPosition(position_);
		player.SetPoseSpin(timer_ * BossFallConfig::kDiveSpinSpeed);
		if (timer_ < hopTime + BossFallConfig::kDiveFallTime)
		{
			return false;
		}
		isActive_ = false;
		player.SetPoseSpin(0.0f);
		return true;
	}
}
