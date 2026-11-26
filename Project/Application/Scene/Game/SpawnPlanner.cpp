#include "SpawnPlanner.h"
#include "../../Config/BossConfig.h"
#include "../../Config/LayerConfig.h"
#include "../../Config/PlayerConfig.h"
#include "../../Stage/Floor/Floor.h"
#include <algorithm>
#include <cmath>
#include <limits>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		//高さを無視した2点の距離の2乗
		float FlatDistanceSquared(const Vector3& a, const Vector3& b)
		{
			const float x = a.x - b.x;
			const float z = a.z - b.z;
			return x * x + z * z;
		}

		//高さを無視した内積
		float FlatDot(const Vector3& a, const Vector3& b)
		{
			return a.x * b.x + a.z * b.z;
		}

		//高さを0にして長さを1にした向き
		Vector3 FlatNormalize(const Vector3& value)
		{
			Vector3 result = { value.x, 0.0f, value.z };
			const float length = std::sqrt(result.x * result.x + result.z * result.z);
			if (length < 0.0001f)
			{
				return { 0.0f, 0.0f, 0.0f };
			}
			return { result.x / length, 0.0f, result.z / length };
		}
	}

	//床の真ん中のマスの位置
	Vector3 SpawnPlanner::GetCenterPosition(const Floor& floor)
	{
		return floor.GetCellCenter({ floor.GetWidth() / 2, floor.GetDepth() / 2 });
	}

	//ボスの体の半分とプレイヤーの半径に少し足した近さ
	float SpawnPlanner::GetContactSpacing()
	{
		return std::max(BossConfig::kBossContactSizeX, BossConfig::kBossContactSizeZ) * 0.5f + PlayerConfig::kPlayerContactRadius + 0.1f;
	}

	//カメラの右と画面の上を地面に平らにした向き
	void SpawnPlanner::GetScreenAxes(Vector3& right, Vector3& up)
	{
		const Camera& camera = CameraManager::GetGameCamera();
		right = FlatNormalize(camera.GetRight());
		up = FlatNormalize(camera.GetUp());
	}

	//崩れていなくて切り離されていない床か
	bool SpawnPlanner::CanStand(const Floor& floor, int32_t x, int32_t z)
	{
		const Cell cell = { x, z };
		return floor.IsSolid(cell) && !floor.IsIsolated(cell);
	}

	//上から落ちてきた位置をもとに、プレイヤーを反対側に置けるボスの位置
	Vector3 SpawnPlanner::FindBossPosition(const Floor& floor, const Vector3& fallingFrom)
	{
		//床の真ん中に落ちてきたら反対側がないので画面の奥へずらす
		const float halfX = floor.GetTileSizeX() * 0.5f;
		const float halfZ = floor.GetTileSizeZ() * 0.5f;
		const bool isCenter = std::abs(fallingFrom.x) <= halfX + 0.001f && std::abs(fallingFrom.z) <= halfZ + 0.001f;
		Vector3 start = { fallingFrom.x, floor.GetHeight(), fallingFrom.z };
		if (isCenter)
		{
			start = MoveAwayFromCenter(floor, GetCenterPosition(floor));
		}
		Vector3 playerPosition{};
		if (TryFindPlayerPosition(floor, start, playerPosition))
		{
			return start;
		}

		//反対側に置けなければ、置けるマスのうち落ちてきた所にいちばん近い所へずらす
		Vector3 right{};
		Vector3 up{};
		GetScreenAxes(right, up);
		Vector3 result = start;
		float bestDistance = std::numeric_limits<float>::infinity();
		for (int32_t z = 0; z < floor.GetDepth(); ++z)
		{
			for (int32_t x = 0; x < floor.GetWidth(); ++x)
			{
				if (!CanStand(floor, x, z))
				{
					continue;
				}
				//真ん中に落ちてきたときは画面の右奥の床だけ
				const Vector3 center = floor.GetCellCenter({ x, z });
				if (isCenter && (FlatDot(center, right) <= 0.0f || FlatDot(center, up) <= 0.0f))
				{
					continue;
				}
				if (!TryFindPlayerPosition(floor, center, playerPosition))
				{
					continue;
				}
				const float distance = FlatDistanceSquared(center, start);
				if (distance < bestDistance)
				{
					bestDistance = distance;
					result = center;
				}
			}
		}
		return result;
	}

	//ボスの反対側で決めたマス数より離れたプレイヤーの位置
	Vector3 SpawnPlanner::FindPlayerPosition(const Floor& floor, const Vector3& bossPosition)
	{
		Vector3 best{};
		TryFindPlayerPosition(floor, bossPosition, best);
		return best;
	}

	//positionの真下にいちばん近い立てる床の真ん中
	Vector3 SpawnPlanner::FindNearestStand(const Floor& floor, const Vector3& position)
	{
		Vector3 best = GetCenterPosition(floor);
		float bestDistance = std::numeric_limits<float>::infinity();
		for (int32_t z = 0; z < floor.GetDepth(); ++z)
		{
			for (int32_t x = 0; x < floor.GetWidth(); ++x)
			{
				if (!CanStand(floor, x, z))
				{
					continue;
				}
				const Vector3 center = floor.GetCellCenter({ x, z });
				const float distance = FlatDistanceSquared(center, position);
				if (distance < bestDistance)
				{
					bestDistance = distance;
					best = center;
				}
			}
		}
		return best;
	}

	//プレイヤーを反対側に置けたらtrueで、置けないときはいちばん遠いマスをbestに入れる
	bool SpawnPlanner::TryFindPlayerPosition(const Floor& floor, const Vector3& bossPosition, Vector3& best)
	{
		//床の真ん中を挟んでボスの反対の点にいちばん近いマスを選ぶ
		const Vector3 mirror = { -bossPosition.x, floor.GetHeight(), -bossPosition.z };
		best = GetCenterPosition(floor);
		float bestDistance = std::numeric_limits<float>::infinity();
		float farthestTiles = -1.0f;
		Vector3 farthest = best;
		const float spacing = GetContactSpacing();
		const float minTiles = LayerConfig::kMinSpawnDistanceTiles;

		for (int32_t z = 0; z < floor.GetDepth(); ++z)
		{
			for (int32_t x = 0; x < floor.GetWidth(); ++x)
			{
				if (!CanStand(floor, x, z))
				{
					continue;
				}
				//ボスと重なるマスはだめ
				const Vector3 center = floor.GetCellCenter({ x, z });
				if (FlatDistanceSquared(center, bossPosition) < spacing * spacing)
				{
					continue;
				}
				//ボスから何マス離れているかで、いちばん遠いマスも覚えておく
				const float tilesX = (center.x - bossPosition.x) / floor.GetTileSizeX();
				const float tilesZ = (center.z - bossPosition.z) / floor.GetTileSizeZ();
				const float tiles = tilesX * tilesX + tilesZ * tilesZ;
				if (tiles > farthestTiles)
				{
					farthestTiles = tiles;
					farthest = center;
				}
				if (tiles + 0.0001f < minTiles * minTiles)
				{
					continue;
				}
				const float distance = FlatDistanceSquared(center, mirror);
				if (distance < bestDistance)
				{
					bestDistance = distance;
					best = center;
				}
			}
		}
		if (bestDistance < std::numeric_limits<float>::infinity())
		{
			return true;
		}
		best = farthest;
		return false;
	}

	//床の真ん中に落ちてきたときに画面の奥へずらしたボスの位置
	Vector3 SpawnPlanner::MoveAwayFromCenter(const Floor& floor, const Vector3& landing)
	{
		//真ん中のマスから離れていればそのまま
		const Vector3 center = GetCenterPosition(floor);
		const float spacing = GetContactSpacing();
		if (floor.GetCellAt(landing) != floor.GetCellAt(center) && FlatDistanceSquared(landing, center) >= spacing * spacing)
		{
			return landing;
		}

		//画面の右奥へ2マスくらいの所にいちばん近いマスを選ぶ
		Vector3 right{};
		Vector3 up{};
		GetScreenAxes(right, up);
		const float step = std::min(floor.GetTileSizeX(), floor.GetTileSizeZ());
		const Vector3 diagonal = FlatNormalize(right + up);
		const Vector3 target = center + diagonal * (step * 2.0f);
		Vector3 result = landing;
		float bestScore = std::numeric_limits<float>::infinity();
		for (int32_t z = 0; z < floor.GetDepth(); ++z)
		{
			for (int32_t x = 0; x < floor.GetWidth(); ++x)
			{
				if (!CanStand(floor, x, z))
				{
					continue;
				}
				//真ん中のマスに近すぎるのはだめ
				const Vector3 cellCenter = floor.GetCellCenter({ x, z });
				const Vector3 fromCenter = cellCenter - center;
				if (FlatDistanceSquared(cellCenter, center) < spacing * spacing)
				{
					continue;
				}
				//画面の右奥でないマスはなるべく選ばない
				float score = FlatDistanceSquared(cellCenter, target);
				if (FlatDot(fromCenter, right) <= 0.0f || FlatDot(fromCenter, up) <= 0.0f)
				{
					score += 100000.0f;
				}
				if (score < bestScore)
				{
					bestScore = score;
					result = cellCenter;
				}
			}
		}
		result.y = floor.GetHeight();
		return result;
	}
}
