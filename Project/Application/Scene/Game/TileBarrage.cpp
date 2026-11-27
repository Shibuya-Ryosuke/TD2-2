#include "TileBarrage.h"
#include "../../Config/BossConfig.h"
#include "../../Config/BossFallConfig.h"
#include "../../Stage/Floor/Floor.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	//飛ばし始める
	int32_t TileBarrage::Begin(Floor* upper)
	{
		tiles_.clear();
		adjacentCount_ = 0;
		timer_ = 0.0f;
		arrived_ = 0;
		arrivedNow_ = 0;
		damageApplied_ = 0;
		if (upper == nullptr)
		{
			return 0;
		}
		upper->PrepareBarrage();

		//穴のとなりの床を先に、ふつうの床をあとに並べる
		for (int32_t group = 0; group < 2; ++group)
		{
			const bool isAdjacentGroup = group == 0;
			for (int32_t z = 0; z < upper->GetDepth(); ++z)
			{
				for (int32_t x = 0; x < upper->GetWidth(); ++x)
				{
					const Cell cell = { x, z };
					if (!upper->IsSolid(cell) || upper->IsHoleAdjacent(cell) != isAdjacentGroup)
					{
						continue;
					}
					int32_t damage = BossConfig::kOrdinaryTileDamage;
					if (isAdjacentGroup)
					{
						damage = BossConfig::kAdjacentTileDamage;
					}
					tiles_.push_back(BarrageTile{ cell, std::max(0, damage), upper->GetTile(cell).position });
				}
			}
			if (isAdjacentGroup)
			{
				adjacentCount_ = GetTileCount();
			}
		}
		return GetTileCount();
	}

	//穴のとなりの足場が飛び立ち終わったら続けてふつうの足場が飛び立つ
	float TileBarrage::GetLaunchTime(int32_t index) const
	{
		const float window = std::max(0.0f, BossFallConfig::kBarrageWindow);
		const int32_t count = GetTileCount();

		//穴のとなりの足場が飛び立ち終わるまでの秒数で、テンポよく続けてふつうの足場が飛び立つ
		float ordinaryStart = 0.0f;
		if (adjacentCount_ > 0)
		{
			float adjacentWindow = 0.0f;
			if (adjacentCount_ > 1)
			{
				adjacentWindow = window;
			}
			ordinaryStart = adjacentWindow + std::max(0.0f, BossFallConfig::kBarrageGroupPause);
		}

		//まとまりの中で少しずつ遅れて飛び立つ
		if (index < adjacentCount_)
		{
			return window * static_cast<float>(index) / static_cast<float>(std::max(1, adjacentCount_ - 1));
		}
		const int32_t ordinaryCount = count - adjacentCount_;
		const int32_t ordinaryIndex = index - adjacentCount_;
		return ordinaryStart + window * static_cast<float>(ordinaryIndex) / static_cast<float>(std::max(1, ordinaryCount - 1));
	}

	//1フレーム進める
	int32_t TileBarrage::Advance(Floor& upper, const Vector3& target, float deltaTime)
	{
		timer_ += deltaTime;
		const int32_t arrivedBefore = arrived_;
		arrived_ = 0;
		int64_t damageUnits = 0;
		const int32_t count = GetTileCount();
		const float flight = std::max(0.01f, BossFallConfig::kBarrageFlight);

		for (int32_t i = 0; i < count; ++i)
		{
			const float t = std::clamp((timer_ - GetLaunchTime(i)) / flight, 0.0f, 1.0f);
			const BarrageTile& tile = tiles_[static_cast<size_t>(i)];

			//届いたら消してダメージを数える
			if (t >= 1.0f)
			{
				++arrived_;
				damageUnits += tile.damage;
				upper.HideTile(tile.cell);
				continue;
			}
			if (t <= 0.0f)
			{
				continue;
			}

			//山なりに飛んで小さくなりながら回る
			Vector3 position = Lerp(tile.start, target, t * t);
			position.y += 4.0f * t * (1.0f - t);
			float spin = 1.0f;
			if (i % 2 != 0)
			{
				spin = -1.0f;
			}
			upper.SetTileMotion(tile.cell, position, { 180.0f * t, 90.0f * t * spin, 0.0f }, 1.0f + (0.25f - 1.0f) * t);
		}

		arrivedNow_ = std::max(0, arrived_ - arrivedBefore);

		//上限ありで、前のフレームから増えたダメージを返す
		const int64_t totalDamage = std::min(damageUnits, static_cast<int64_t>(BossConfig::kMaxTileDamage));
		const int32_t damage = static_cast<int32_t>(totalDamage);
		int32_t added = 0;
		if (damage > damageApplied_)
		{
			added = damage - damageApplied_;
			damageApplied_ = damage;
		}
		return added;
	}

	//そのマスの足場が飛び立ったか、飛ばす足場がないか
	bool TileBarrage::IsLaunched(const Cell& cell) const
	{
		for (int32_t i = 0; i < GetTileCount(); ++i)
		{
			if (tiles_[static_cast<size_t>(i)].cell == cell)
			{
				return timer_ > GetLaunchTime(i);
			}
		}
		return true;
	}
}
