#include "TileLift.h"
#include "../../Config/BossFallConfig.h"
#include "../../Stage/Floor/Floor.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//upperの残った足場を浮かせ始める
	void TileLift::Begin(Floor& upper)
	{
		upper.PrepareBarrage();
		cells_.clear();
		for (int32_t z = 0; z < upper.GetDepth(); ++z)
		{
			for (int32_t x = 0; x < upper.GetWidth(); ++x)
			{
				const Cell cell = { x, z };
				if (upper.IsSolid(cell))
				{
					cells_.push_back(cell);
				}
			}
		}
		timer_ = 0.0f;
	}

	//ゆっくり浮いて光が強くなり、浮ききったらマスごとに少しずらしてふわふわ
	void TileLift::Update(float deltaTime, Floor& upper)
	{
		using namespace BossFallConfig;
		timer_ += deltaTime;
		const float t = std::clamp(timer_ / kLiftTime, 0.0f, 1.0f);
		const float rise = t * t * (3.0f - 2.0f * t);
		upper.SetChargeGlow(kLiftColor, kLiftGlow * rise);
		for (size_t i = 0; i < cells_.size(); ++i)
		{
			const Cell& cell = cells_[i];
			if (!upper.IsSolid(cell))
			{
				continue;
			}
			const float hover = kLiftHoverHeight * std::sin(timer_ * kLiftHoverSpeed + static_cast<float>(i) * kLiftHoverPhaseStep) * rise;
			upper.SetTileMotion(cell, upper.GetRestPosition(cell) + Vector3{ 0.0f, kLiftHeight * rise + hover, 0.0f }, { 0.0f, 0.0f, 0.0f }, 1.0f);
		}
	}
}
