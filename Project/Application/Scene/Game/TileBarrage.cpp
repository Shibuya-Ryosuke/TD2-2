#include "TileBarrage.h"
#include "../../Config/GameConfig.h"
#include "../../Stage/Floor/Floor.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	// 飛ばし始める
	int32_t TileBarrage::Begin(const Floor* upper)
	{
		tiles_.clear();
		timer_ = 0.0f;
		arrived_ = 0;
		damageApplied_ = 0;
		if (upper == nullptr)
		{
			return 0;
		}

		// 残っている床を全部、飛ばす足場にする
		for (int32_t z = 0; z < upper->GetDepth(); ++z)
		{
			for (int32_t x = 0; x < upper->GetWidth(); ++x)
			{
				const Cell cell = { x, z };
				if (upper->IsSolid(cell))
				{
					tiles_.push_back(BarrageTile{ cell, upper->GetBarrageMultiplier(cell) });
				}
			}
		}
		return GetTileCount();
	}

	// 1フレーム進める
	int32_t TileBarrage::Advance(Floor& upper, const Vector3& target, float deltaTime)
	{
		timer_ += deltaTime;
		arrived_ = 0;
		int64_t damageUnits = 0;
		const int32_t count = GetTileCount();

		for (int32_t i = 0; i < count; ++i)
		{
			// 1枚ずつ、少しずつ遅れて飛び立つ
			const float launchTime = GameConfig::kTileLaunchWindow * static_cast<float>(i) / static_cast<float>(std::max(1, count - 1));
			const float t = std::clamp((timer_ - launchTime) / GameConfig::kTileFlightDuration, 0.0f, 1.0f);
			const BarrageTile& tile = tiles_[static_cast<size_t>(i)];

			// 届いた: 消して、ダメージを数える
			if (t >= 1.0f)
			{
				++arrived_;
				damageUnits += tile.multiplier;
				upper.HideTile(tile.cell);
				continue;
			}
			if (t <= 0.0f)
			{
				continue;
			}

			// 山なりに飛んで、小さくなりながら回る (となりどうしで回る向きを変える)
			Vector3 position = Lerp(upper.GetRestPosition(tile.cell), target, t * t);
			position.y += 4.0f * t * (1.0f - t);
			float spin = 1.0f;
			if (i % 2 != 0)
			{
				spin = -1.0f;
			}
			upper.SetTileMotion(tile.cell, position, { 180.0f * t, 90.0f * t * spin, 0.0f }, 1.0f + (0.25f - 1.0f) * t);
		}

		// 届いた分だけダメージ (上限あり)。前のフレームから増えた分を返す
		const int64_t totalDamage = std::min(damageUnits * GameConfig::kDamagePerRemainingTile, static_cast<int64_t>(GameConfig::kMaxTileDamage));
		const int32_t damage = static_cast<int32_t>(totalDamage);
		int32_t added = 0;
		if (damage > damageApplied_)
		{
			added = damage - damageApplied_;
			damageApplied_ = damage;
		}
		return added;
	}
}
