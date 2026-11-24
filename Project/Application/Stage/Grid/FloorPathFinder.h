#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <vector>
#include "Cell.h"

namespace App
{
	class Floor;

	//床を読むだけでボスの次のマスや逃げ先やたどり着けるかを調べる道さがし
	class FloorPathFinder
	{
	public:
		//startからtargetへ向かうときの次のマス
		static Cell NextStep(const Floor& floor, const Cell& start, const RyoEngine::Vector3& target, bool canJump);

		//上下左右でtimeToLand秒のうちに崩れない一番近い床を探す
		static bool FindLandingNeighbor(const Floor& floor, const RyoEngine::Vector3& position, float timeToLand, Cell& landing);

		//ボスの島が同じか大きい島から離れていたらregionに入れてtrue
		static bool FindIsolatedRegion(const Floor& floor, const Cell& bossCell, std::vector<Cell>& region);

		//fromからtoまでたどり着けるか
		static bool CanReach(const Floor& floor, const Cell& from, const Cell& to, bool canJump);

	private:
		//startから床の上をたどって行ける所を幅優先で全部さがす
		static void Explore(const Floor& floor, const Cell& start, bool canJump, std::vector<int32_t>& parents, std::vector<Cell>& order);
	};
}
