#include "ShapeController.h"
#include "../../../Config/DirectionConfig.h"
#include "../../../Config/GameConfig.h"
#include "../../../Stage/Floor/Floor.h"
#include <algorithm>
#include <cmath>

namespace App
{
	//形を作る
	void ShapeController::Initialize()
	{
		shapes_ = BreakShape::CreateDefaultShapes();
		Reset();
		panel_.Initialize(selectedIndex_);
	}

	//最初のLを選ぶ
	void ShapeController::Reset()
	{
		selectedIndex_ = 0;
		isReady_ = true;
	}

	//ぐるっと回して形を切り替える
	void ShapeController::Select(bool isPrevious, bool isNext)
	{
		const int32_t count = static_cast<int32_t>(shapes_.size());
		if (count == 0)
		{
			return;
		}
		if (isPrevious)
		{
			selectedIndex_ = (selectedIndex_ - 1 + count) % count;
		}
		if (isNext)
		{
			selectedIndex_ = (selectedIndex_ + 1) % count;
		}
	}

	//選んでいる形からoffsetだけずらした形
	const BreakShape* ShapeController::GetShape(int32_t offset) const
	{
		const int32_t count = static_cast<int32_t>(shapes_.size());
		if (count == 0)
		{
			return nullptr;
		}
		//マイナスでも0からcount-1に収める
		const int32_t index = ((selectedIndex_ + offset) % count + count) % count;
		return &shapes_[static_cast<size_t>(index)];
	}

	//NOWの形でプレイヤーの前の床を崩す
	bool ShapeController::TryBreak(Floor& floor, const Cell& playerCell, const Cell& facing)
	{
		const BreakShape* shape = GetShape(0);
		if (shape == nullptr || !IsReady())
		{
			return false;
		}

		//崩せる床が1つもなければ置かなかったことにする
		shape->CollectCells(playerCell, facing, cells_);
		//崩れる演出のハートをプレイヤーの向きへ向ける
		const float heartAngle = std::atan2(static_cast<float>(facing.x), static_cast<float>(facing.z));
		//ハンマーが当たるまでの分だけ遅らせて、当たった瞬間にハートが出るようにする
		const float delay = GameConfig::kCollapseDelay + DirectionConfig::kSwingImpactDelay;
		const int32_t count = floor.ScheduleCellsCollapse(cells_, delay, heartAngle);
		return count > 0;
	}

	//床にNOWの形の予告を出す
	void ShapeController::UpdatePreview(Floor& floor, const Cell& playerCell, const Cell& facing, bool isVisible)
	{
		const BreakShape* shape = GetShape(0);
		if (shape == nullptr || !isVisible)
		{
			floor.ClearPreview();
			return;
		}
		//プレイヤーの前のNOWの形のマスを光らせる
		shape->CollectCells(playerCell, facing, cells_);
		floor.SetPreview(cells_, shape->GetColor(), GameConfig::kShapePreviewBlend);
	}

	//ボスの番の間はNOWを暗くして右上の輪を合わせる
	void ShapeController::UpdatePanel(const StageCamera& camera, const Cell& facing)
	{
		panel_.Update(camera, shapes_, selectedIndex_, facing, IsReady());
	}

	void ShapeController::DrawPanel()
	{
		panel_.Draw();
	}
}
