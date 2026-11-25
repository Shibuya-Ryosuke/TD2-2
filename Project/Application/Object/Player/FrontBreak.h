#pragma once
#include <vector>
#include "../../Stage/Grid/Cell.h"

namespace App
{
	class Floor;

	//プレイヤーの正面の1マスを崩して、床にその予告を出す
	class FrontBreak
	{
	public:
		//正面の1マスを崩せたらtrue
		bool TryBreak(Floor& floor, const Cell& playerCell, const Cell& facing);

		//床に正面の1マスの予告を出す
		void UpdatePreview(Floor& floor, const Cell& playerCell, const Cell& facing, bool isVisible);

	private:
		//正面の1マスが床の中ならcells_に入れる
		void CollectCell(const Floor& floor, const Cell& playerCell, const Cell& facing);

		std::vector<Cell> cells_; //崩すマス
	};
}
