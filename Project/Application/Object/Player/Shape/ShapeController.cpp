#include "ShapeController.h"
#include "../../../Config/GameConfig.h"
#include "../../../Stage/Floor/Floor.h"
#include <algorithm>
#include <array>

namespace App
{
	// 形を作る
	void ShapeController::Initialize()
	{
		shapes_ = BreakShape::CreateDefaultShapes();
		panel_.Initialize();
		Reset();
	}

	// 最初の形 (X) を選んで、待ち時間を消す
	void ShapeController::Reset()
	{
		selectedIndex_ = 0;
		cooldownTimer_ = 0.0f;
	}

	// 待ち時間を進める
	void ShapeController::Update(float deltaTime)
	{
		cooldownTimer_ = std::max(0.0f, cooldownTimer_ - deltaTime);
	}

	// 形を切り替える (最初の形の前は最後の形、最後の形の次は最初の形)
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

	// 選んでいる形から offset だけずらした形 (ぐるっと回る)
	const BreakShape* ShapeController::GetShape(int32_t offset) const
	{
		const int32_t count = static_cast<int32_t>(shapes_.size());
		if (count == 0)
		{
			return nullptr;
		}
		// マイナスでも 0 〜 count - 1 に収まるように、count を足してから割る
		const int32_t index = ((selectedIndex_ + offset) % count + count) % count;
		return &shapes_[static_cast<size_t>(index)];
	}

	// NOW の形で、プレイヤーの前の床を崩す
	bool ShapeController::TryBreak(Floor& floor, const Cell& playerCell, const Cell& facing)
	{
		const BreakShape* shape = GetShape(0);
		if (shape == nullptr || !IsReady())
		{
			return false;
		}

		// 形のマスを崩す。崩せる床が1つもなければ、置かなかったことにする
		shape->CollectCells(playerCell, facing, cells_);
		const int32_t count = floor.ScheduleCellsCollapse(cells_, GameConfig::kCollapseDelay);
		if (count == 0)
		{
			return false;
		}

		// 置いたので、待ち時間を入れる (形はそのまま)
		cooldownTimer_ = GameConfig::kShapeCooldown;
		return true;
	}

	// 床に NOW の形の予告を出す
	void ShapeController::UpdatePreview(Floor& floor, const Cell& playerCell, const Cell& facing, bool isVisible)
	{
		const BreakShape* shape = GetShape(0);
		if (shape == nullptr || !isVisible)
		{
			floor.ClearPreview();
			return;
		}
		// プレイヤーの前に NOW の形のマスを集めて、床を光らせる
		shape->CollectCells(playerCell, facing, cells_);
		floor.SetPreview(cells_, shape->GetColor(), GameConfig::kShapePreviewBlend);
	}

	// 画面の右はしのパネルを合わせる
	void ShapeController::UpdatePanel(const StageCamera& camera, const Cell& facing)
	{
		// パネルの並び: NOW (真ん中) / LB で来る形 (上) / RB で来る形 (下)。待ち時間中は NOW を暗くする
		std::array<const BreakShape*, NextShapePanel::kSlotCount> shapes{};
		shapes[NextShapePanel::kNowSlot] = GetShape(0);
		shapes[NextShapePanel::kPreviousSlot] = GetShape(-1);
		shapes[NextShapePanel::kNextSlot] = GetShape(1);
		panel_.Update(camera, shapes, facing, IsReady());
	}

	void ShapeController::DrawPanel()
	{
		panel_.Draw();
	}

	void ShapeController::DrawPanelLabels() const
	{
		panel_.DrawLabels();
	}
}
