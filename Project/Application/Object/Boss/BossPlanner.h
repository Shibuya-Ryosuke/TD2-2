#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <vector>
#include "BossPlan.h"
#include "BossTuning.h"
#include "../../Stage/Grid/Cell.h"

namespace App
{
	class Floor;

	//崩れそうなら逃げて、2回に1回近づき、それ以外は攻撃するボスの考え方
	class BossPlanner
	{
	public:
		//次の1回の行動を決める
		static BossPlannedAction Choose(const Floor& floor, const RyoEngine::Vector3& from, const RyoEngine::Vector3& player,
			const BossTuning& tuning, float speed, int32_t turn, float elapsed, bool isEvacuating, const RyoEngine::Vector3& evacuationTarget);

		//足元が崩れそうなときに逃げる先を探す
		static bool TryFindSafetyStep(const Floor& floor, const RyoEngine::Vector3& from, float speed, float elapsed, float margin, RyoEngine::Vector3& destination);

		//targetへ向かう次のマスの真ん中
		static RyoEngine::Vector3 NextStep(const Floor& floor, const RyoEngine::Vector3& from, const RyoEngine::Vector3& target, bool canJump, float speed, float elapsed, float margin);

		//time秒後にそのマスへ着地できるか
		static bool CanLand(const Floor& floor, const Cell& cell, float time);

	private:
		//time秒後も着地できる床をつながりごとに分ける
		static void StableRegions(const Floor& floor, float time, std::vector<int32_t>& regionOf, std::vector<int32_t>& sizes);
		//2つの位置の横の距離
		static float HorizontalDistance(const RyoEngine::Vector3& a, const RyoEngine::Vector3& b);
	};
}
