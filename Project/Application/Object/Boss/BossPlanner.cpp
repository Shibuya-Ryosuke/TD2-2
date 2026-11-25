#include "BossPlanner.h"
#include "../../Stage/Floor/Floor.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
#include <limits>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		//上下左右
		constexpr std::array<Cell, 4> kDirections = { { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } } };
		constexpr float kInfinity = std::numeric_limits<float>::infinity();

		//2つの位置の距離の2乗
		float DistanceSquared(const Vector3& a, const Vector3& b)
		{
			const Vector3 d = a - b;
			return d.x * d.x + d.y * d.y + d.z * d.z;
		}
	}

	//2つの位置の横の距離
	float BossPlanner::HorizontalDistance(const Vector3& a, const Vector3& b)
	{
		const float dx = a.x - b.x;
		const float dz = a.z - b.z;
		return std::sqrt(dx * dx + dz * dz);
	}

	//time秒後にそのマスへ着地できるか
	bool BossPlanner::CanLand(const Floor& floor, const Cell& cell, float time)
	{
		if (!floor.Contains(cell) || !floor.IsSolid(cell) || floor.IsIsolated(cell))
		{
			return false;
		}
		const float remaining = floor.GetTile(cell).remaining;
		if (remaining < 0.0f)
		{
			return true; //崩す予定はない
		}
		return remaining > time;
	}

	//次の1回の行動を決める
	BossPlannedAction BossPlanner::Choose(const Floor& floor, const Vector3& from, const Vector3& player,
		const BossTuning& tuning, float speed, int32_t turn, float elapsed, bool isEvacuating, const Vector3& evacuationTarget)
	{
		BossPlannedAction action{};
		action.to = from;
		action.duration = 0.4f;

		//足元が崩れるなら落ちる
		if (!CanLand(floor, floor.GetCellAt(from), elapsed))
		{
			action.kind = BossActionKind::Fall;
			return action;
		}

		//足元が崩れそうなら逃げる
		Vector3 destination{};
		if (TryFindSafetyStep(floor, from, speed, elapsed, tuning.landingSafetyMargin, destination))
		{
			action.kind = BossActionKind::Move;
			action.isEvading = true;
			action.to = destination;
			action.duration = std::max(0.05f, HorizontalDistance(from, destination) / speed);
			return action;
		}

		//遠ければ2回に1回は近づき、逃げている途中なら逃げ先へ
		const float distance = HorizontalDistance(player, from);
		const bool isMoveTurn = isEvacuating || turn % 2 == 0;
		if (isMoveTurn && speed > 0.0f && (isEvacuating || distance > tuning.meleeRange))
		{
			Vector3 goal = player;
			bool canJump = tuning.canJump;
			if (isEvacuating)
			{
				goal = evacuationTarget;
				canJump = false;
			}
			action.kind = BossActionKind::Move;
			action.isEvading = isEvacuating;
			action.to = NextStep(floor, from, goal, canJump, speed, elapsed, tuning.landingSafetyMargin);
			action.duration = std::max(0.05f, HorizontalDistance(from, action.to) / speed);
			action.isBlocked = HorizontalDistance(from, action.to) < 0.001f;
			if (!action.isBlocked || isEvacuating)
			{
				return action;
			}
		}

		//パンチと近接を順番に、届くものを選ぶ
		std::vector<BossActionKind> attacks;
		if (tuning.punch)
		{
			attacks.push_back(BossActionKind::Punch);
		}
		attacks.push_back(BossActionKind::Melee);
		const size_t first = static_cast<size_t>(turn / 2) % attacks.size();
		action.isBlocked = false;
		action.to = from;
		for (size_t i = 0; i < attacks.size(); ++i)
		{
			const BossActionKind kind = attacks[(first + i) % attacks.size()];
			if (kind == BossActionKind::Melee && distance > tuning.meleeRange)
			{
				continue;
			}
			if (kind == BossActionKind::Punch && distance > tuning.punchRange)
			{
				continue;
			}
			action.kind = kind;
			if (kind == BossActionKind::Punch)
			{
				action.duration = tuning.punchWindup + tuning.punchStrikeDuration + tuning.punchRecovery;
				return action;
			}
			action.duration = tuning.meleeWindup + 0.15f;
			return action;
		}

		//どれも届かなければ近づき、行けなければその場で近接攻撃
		Vector3 step = from;
		if (speed > 0.0f)
		{
			step = NextStep(floor, from, player, tuning.canJump, speed, elapsed, tuning.landingSafetyMargin);
		}
		if (DistanceSquared(step, from) > 0.001f)
		{
			action.kind = BossActionKind::Move;
			action.to = step;
			action.duration = std::max(0.05f, HorizontalDistance(from, step) / speed);
			return action;
		}
		action.kind = BossActionKind::Melee;
		action.duration = tuning.meleeWindup + 0.15f;
		return action;
	}

	//time秒後も着地できる床をつながりごとに分ける
	void BossPlanner::StableRegions(const Floor& floor, float time, std::vector<int32_t>& regionOf, std::vector<int32_t>& sizes)
	{
		regionOf.assign(floor.GetTileCount(), -1);
		sizes.clear();
		std::deque<Cell> queue;
		for (size_t i = 0; i < floor.GetTileCount(); ++i)
		{
			const Cell start = floor.ToCell(i);
			if (regionOf[i] >= 0 || !CanLand(floor, start, time))
			{
				continue;
			}
			//新しい島のつながっている所を全部数える
			const int32_t region = static_cast<int32_t>(sizes.size());
			int32_t size = 0;
			regionOf[i] = region;
			queue.push_back(start);
			while (!queue.empty())
			{
				const Cell cell = queue.front();
				queue.pop_front();
				++size;
				for (const Cell& direction : kDirections)
				{
					const Cell next = cell + direction;
					if (CanLand(floor, next, time) && regionOf[floor.ToIndex(next)] < 0)
					{
						regionOf[floor.ToIndex(next)] = region;
						queue.push_back(next);
					}
				}
			}
			sizes.push_back(size);
		}
	}

	//足元が崩れそうなときに逃げる先を探す
	bool BossPlanner::TryFindSafetyStep(const Floor& floor, const Vector3& from, float speed, float elapsed, float margin, Vector3& destination)
	{
		const Cell cell = floor.GetCellAt(from);
		destination = from;
		if (speed <= 0.0f || !CanLand(floor, cell, elapsed))
		{
			return false;
		}
		const float remainingHere = floor.GetTile(cell).remaining;
		std::vector<int32_t> regionOf;
		std::vector<int32_t> sizes;
		StableRegions(floor, elapsed + 4.0f, regionOf, sizes);

		//足元が平気でも今の島が小さければいちばん大きい島へ向かう
		if (remainingHere < 0.0f)
		{
			int32_t largest = 0;
			for (const int32_t size : sizes)
			{
				largest = std::max(largest, size);
			}
			const int32_t here = regionOf[floor.ToIndex(cell)];
			if (largest == 0 || (here >= 0 && sizes[static_cast<size_t>(here)] >= largest))
			{
				return false;
			}
			Vector3 target = from;
			float nearest = kInfinity;
			for (size_t i = 0; i < regionOf.size(); ++i)
			{
				if (regionOf[i] < 0 || sizes[static_cast<size_t>(regionOf[i])] != largest)
				{
					continue;
				}
				const Vector3 center = floor.GetCellCenter(floor.ToCell(i));
				const float distance = DistanceSquared(center, from);
				if (distance < nearest)
				{
					nearest = distance;
					target = center;
				}
			}
			destination = NextStep(floor, from, target, false, speed, elapsed, std::max(0.5f, margin));
			return DistanceSquared(destination, from) > 0.001f;
		}

		//足元が崩れる予定ならとなりのもっと長くもつマスへ
		float bestRemaining = remainingHere;
		float bestDistance = kInfinity;
		int32_t bestSize = -1;
		for (const Cell& direction : kDirections)
		{
			const Cell next = cell + direction;
			const Vector3 center = floor.GetCellCenter(next);
			const float distance = HorizontalDistance(from, center);
			if (!CanLand(floor, next, elapsed + distance / speed + std::max(0.5f, margin)))
			{
				continue;
			}
			float remaining = floor.GetTile(next).remaining;
			if (remaining < 0.0f)
			{
				remaining = kInfinity;
			}
			int32_t size = 0;
			const int32_t region = regionOf[floor.ToIndex(next)];
			if (region >= 0)
			{
				size = sizes[static_cast<size_t>(region)];
			}
			//今の足元より長くもたないマスは選ばない
			if (remaining <= remainingHere || remaining < bestRemaining)
			{
				continue;
			}
			//同じくらいもつなら大きい島で近いマス
			if (remaining == bestRemaining && (size < bestSize || (size == bestSize && distance >= bestDistance)))
			{
				continue;
			}
			bestRemaining = remaining;
			bestSize = size;
			bestDistance = distance;
			destination = center;
		}
		return DistanceSquared(destination, from) > 0.001f;
	}

	//targetへ向かう次のマスの真ん中
	Vector3 BossPlanner::NextStep(const Floor& floor, const Vector3& from, const Vector3& target, bool canJump, float speed, float elapsed, float margin)
	{
		const Cell start = floor.GetCellAt(from);
		const Cell goal = floor.GetCellAt(target);
		if (!floor.Contains(start))
		{
			return from;
		}
		//幅優先で行けるマスを全部さがす
		std::vector<int32_t> parents(floor.GetTileCount(), -1);
		const int32_t startIndex = static_cast<int32_t>(floor.ToIndex(start));
		parents[static_cast<size_t>(startIndex)] = startIndex;
		std::deque<Cell> queue;
		queue.push_back(start);
		Cell best = start;
		float bestDistance = DistanceSquared(floor.GetCellCenter(start), target);
		while (!queue.empty())
		{
			const Cell cell = queue.front();
			queue.pop_front();
			for (const Cell& direction : kDirections)
			{
				//着地できないマスは跳べるなら1つ飛び越える
				Cell next = cell + direction;
				if (!CanLand(floor, next, elapsed) && canJump)
				{
					next = next + direction;
				}
				if (!CanLand(floor, next, elapsed) || parents[floor.ToIndex(next)] >= 0)
				{
					continue;
				}
				//最初の1歩は余裕も入れて着地までに崩れないか見る
				if (cell == start)
				{
					const float landTime = elapsed + HorizontalDistance(from, floor.GetCellCenter(next)) / std::max(0.1f, speed) + margin;
					if (!CanLand(floor, next, landTime))
					{
						continue;
					}
				}
				parents[floor.ToIndex(next)] = static_cast<int32_t>(floor.ToIndex(cell));
				queue.push_back(next);
				const float distance = DistanceSquared(floor.GetCellCenter(next), target);
				if (distance < bestDistance)
				{
					best = next;
					bestDistance = distance;
				}
			}
		}
		//目的のマスかいちばん近づけるマスへ向かう最初の1歩をさかのぼる
		if (floor.Contains(goal) && parents[floor.ToIndex(goal)] >= 0)
		{
			best = goal;
		}
		while (!(best == start) && parents[floor.ToIndex(best)] != startIndex)
		{
			best = floor.ToCell(static_cast<size_t>(parents[floor.ToIndex(best)]));
		}
		if (best == start)
		{
			return from;
		}
		return floor.GetCellCenter(best);
	}
}
