#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <vector>
#include "Cell.h"

namespace App
{
	class Floor;

	// 床の上の道さがし (Floor から分けたもの。床は読むだけで、書きかえない)
	// ・ボスの次のマス (NextStep)・逃げ先 (FindLandingNeighbor)
	// ・ボスの島が切り離されたか (FindIsolatedRegion)・おたがいにたどり着けるか (CanReach)
	class FloorPathFinder
	{
	public:
		// start から target へ向かうときの、次のマス (canJump: 穴を1つ飛び越えられる)
		static Cell NextStep(const Floor& floor, const Cell& start, const RyoEngine::Vector3& target, bool canJump);

		// 上下左右のマスで、timeToLand 秒のうちに崩れない一番近い床を探す。見つかったら true
		static bool FindLandingNeighbor(const Floor& floor, const RyoEngine::Vector3& position, float timeToLand, Cell& landing);

		// ボスのいる島が、それと同じか大きい島から離れていたら、ボスの島のマスを region に入れて true
		static bool FindIsolatedRegion(const Floor& floor, const Cell& bossCell, std::vector<Cell>& region);

		// from から to までたどり着けるか (canJump: 穴を1つ飛び越えられる)
		static bool CanReach(const Floor& floor, const Cell& from, const Cell& to, bool canJump);

	private:
		// start から、床の上をたどって行ける所を全部さがす (幅優先)
		// parents: マスごとの「1つ前のマス」の番号 (-1: 行けない) / order: 見つけた順
		static void Explore(const Floor& floor, const Cell& start, bool canJump, std::vector<int32_t>& parents, std::vector<Cell>& order);
	};
}
