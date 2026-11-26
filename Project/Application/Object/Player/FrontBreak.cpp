#include "FrontBreak.h"
#include "../../Config/DirectionConfig.h"
#include "../../Config/FloorConfig.h"
#include "../../Stage/Floor/Floor.h"
#include <cmath>

namespace App
{
	//正面の1マスが床の中ならcells_に入れる
	void FrontBreak::CollectCell(const Floor& floor, const Cell& playerCell, const Cell& facing)
	{
		cells_.clear();
		const Cell front = playerCell + facing;
		if (floor.Contains(playerCell) && floor.Contains(front))
		{
			cells_.push_back(front);
		}
	}

	//正面の1マスを崩す
	bool FrontBreak::TryBreak(Floor& floor, const Cell& playerCell, const Cell& facing)
	{
		CollectCell(floor, playerCell, facing);
		//崩れる演出のハートをプレイヤーの向きへ向ける
		const float heartAngle = std::atan2(static_cast<float>(facing.x), static_cast<float>(facing.z));
		//ハンマーが当たるまでの分だけ遅らせて、当たった瞬間にハートが出るようにする
		const float delay = FloorConfig::kCollapseDelay + DirectionConfig::kSwingImpactDelay;
		return floor.ScheduleCellsCollapse(cells_, delay, heartAngle) > 0;
	}

	//床に正面の1マスの予告を出す
	void FrontBreak::UpdatePreview(Floor& floor, const Cell& playerCell, const Cell& facing, bool isVisible)
	{
		if (!isVisible)
		{
			floor.ClearPreview();
			return;
		}
		CollectCell(floor, playerCell, facing);
		floor.SetPreview(cells_, FloorConfig::kBreakPreviewColor, FloorConfig::kShapePreviewBlend);
	}
}
