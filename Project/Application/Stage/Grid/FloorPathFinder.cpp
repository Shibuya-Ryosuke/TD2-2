#include "FloorPathFinder.h"
#include "../Floor/Floor.h"
#include "../../Config/GameConfig.h"
#include <limits>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		//奥、右、手前、左のとなりの4マス
		constexpr Cell kDirections[4] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };

		//比べるだけなのでsqrtしない2点の距離の2乗
		float DistanceSquared(const Vector3& a, const Vector3& b)
		{
			const float x = a.x - b.x;
			const float y = a.y - b.y;
			const float z = a.z - b.z;
			return x * x + y * y + z * z;
		}
	}

	//startから行けるマスを全部調べる
	void FloorPathFinder::Explore(const Floor& floor, const Cell& start, bool canJump, std::vector<int32_t>& parents, std::vector<Cell>& order)
	{
		parents.assign(floor.GetTileCount(), -1);
		order.clear();
		if (!floor.IsSolid(start))
		{
			return;
		}

		//orderを待ち行列にして幅優先で調べる
		parents[floor.ToIndex(start)] = static_cast<int32_t>(floor.ToIndex(start));
		order.push_back(start);
		for (size_t head = 0; head < order.size(); ++head)
		{
			const Cell current = order[head];
			for (const Cell& direction : kDirections)
			{
				//飛び越えられるならボスが越えられる数まで穴を飛ばした先も行ける
				Cell next = current + direction;
				if (canJump)
				{
					next = floor.JumpLanding(current, direction, GameConfig::kBossJumpGap);
				}
				if (!floor.IsSolid(next))
				{
					continue;
				}
				//まだ見つけていないマスだけ足す
				const size_t nextIndex = floor.ToIndex(next);
				if (parents[nextIndex] >= 0)
				{
					continue;
				}
				parents[nextIndex] = static_cast<int32_t>(floor.ToIndex(current));
				order.push_back(next);
			}
		}
	}

	//上下左右ですぐには崩れないボスの逃げ先
	bool FloorPathFinder::FindLandingNeighbor(const Floor& floor, const Vector3& position, float timeToLand, Cell& landing)
	{
		const Cell current = floor.GetCellAt(position);
		landing = current;
		float bestDistance = std::numeric_limits<float>::infinity();

		//上下左右のうち一番近い床を選ぶ
		for (const Cell& direction : kDirections)
		{
			const Cell cell = current + direction;
			if (!floor.IsSolid(cell))
			{
				continue;
			}
			//着く前に崩れてしまう床はだめ
			const float remaining = floor.GetTile(cell).remaining;
			if (remaining >= 0.0f && remaining <= timeToLand)
			{
				continue;
			}
			const float distance = DistanceSquared(floor.GetCellCenter(cell), position);
			if (distance < bestDistance)
			{
				bestDistance = distance;
				landing = cell;
			}
		}
		return landing != current;
	}

	//ボスの島がほかの島より小さければ切り離す島として返す
	bool FloorPathFinder::FindIsolatedRegion(const Floor& floor, const Cell& bossCell, std::vector<Cell>& region)
	{
		//ボスのいる島を調べる
		std::vector<int32_t> bossParents;
		Explore(floor, bossCell, false, bossParents, region);
		if (region.empty())
		{
			return false;
		}

		//ボスの島と同じか大きい島がほかにあるか
		std::vector<bool> isVisited(floor.GetTileCount(), false);
		for (const Cell& cell : region)
		{
			isVisited[floor.ToIndex(cell)] = true;
		}
		std::vector<int32_t> otherParents;
		std::vector<Cell> otherRegion;
		for (int32_t z = 0; z < floor.GetDepth(); ++z)
		{
			for (int32_t x = 0; x < floor.GetWidth(); ++x)
			{
				const Cell cell = { x, z };
				if (!floor.IsSolid(cell) || isVisited[floor.ToIndex(cell)])
				{
					continue;
				}
				//まだ調べていない島の大きさを数える
				Explore(floor, cell, false, otherParents, otherRegion);
				if (otherRegion.size() >= region.size())
				{
					return true;
				}
				for (const Cell& other : otherRegion)
				{
					isVisited[floor.ToIndex(other)] = true;
				}
			}
		}
		return false;
	}

	//fromからtoまでたどり着けるか
	bool FloorPathFinder::CanReach(const Floor& floor, const Cell& from, const Cell& to, bool canJump)
	{
		std::vector<int32_t> parents;
		std::vector<Cell> order;
		Explore(floor, from, canJump, parents, order);
		if (!floor.Contains(to))
		{
			return false;
		}
		return parents[floor.ToIndex(to)] >= 0;
	}
}
