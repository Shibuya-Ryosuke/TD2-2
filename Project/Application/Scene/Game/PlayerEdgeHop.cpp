#include "PlayerEdgeHop.h"
#include "../../Config/BossFallConfig.h"
#include "../../Object/Player/Player.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App
{
	//fromからedgeまで跳び始めて、遠いほど1回の跳びを速くして回数を決める
	void PlayerEdgeHop::Begin(const Vector3& from, const Vector3& edge, float tileSize, Player& player)
	{
		from_ = from;
		edge_ = edge;
		timer_ = 0.0f;
		isActive_ = true;
		const float distance = std::hypot(edge.x - from.x, edge.z - from.z);
		const float tile = std::max(tileSize, 0.01f);
		count_ = 0;
		if (distance > 0.01f)
		{
			count_ = std::clamp(static_cast<int32_t>(std::round(distance / tile)), 1, BossFallConfig::kHopMaxCount);
			player.SetFacingDirection(edge - from);
		}
	}

	//1回ずつ山なりに跳んで、着いたらぐっと沈んでためる
	bool PlayerEdgeHop::Update(float deltaTime, Player& player)
	{
		if (!isActive_)
		{
			return false;
		}
		timer_ += deltaTime;
		float hopTime = 0.0f;
		if (count_ > 0)
		{
			hopTime = BossFallConfig::kHopTotalTime;
		}
		if (timer_ < hopTime)
		{
			const float progress = timer_ / hopTime * static_cast<float>(count_);
			const float hop = std::floor(progress);
			const float t = progress - hop;
			const float along = (hop + t) / static_cast<float>(count_);
			Vector3 position = from_ + (edge_ - from_) * along;
			position.y += 4.0f * BossFallConfig::kHopHeight * t * (1.0f - t);
			player.SetPosition(position);
			return false;
		}

		//ふちでぐっと沈んでためる
		const float crouch = std::clamp((timer_ - hopTime) / BossFallConfig::kCrouchTime, 0.0f, 1.0f);
		player.SetPosition(edge_ - Vector3{ 0.0f, BossFallConfig::kCrouchSink * std::sin(crouch * std::numbers::pi_v<float> * 0.5f), 0.0f });
		if (crouch < 1.0f)
		{
			return false;
		}
		isActive_ = false;
		return true;
	}
}
