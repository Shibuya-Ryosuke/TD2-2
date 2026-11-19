#include "ShapeController.h"
#include "GameConfig.h"
#include "../Stage/Floor.h"
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

	// 形の順番を最初からにする
	void ShapeController::Reset()
	{
		queue_.Reset(GameConfig::kShuffleShapes);
		cooldownTimer_ = 0.0f;
	}

	// 待ち時間を進める
	void ShapeController::Update(float deltaTime)
	{
		cooldownTimer_ = std::max(0.0f, cooldownTimer_ - deltaTime);
	}

	// 種類から形を探す
	const BreakShape* ShapeController::FindShape(BreakShapeType type) const
	{
		for (const BreakShape& shape : shapes_)
		{
			if (shape.GetType() == type)
			{
				return &shape;
			}
		}
		return nullptr;
	}

	// 今の形
	const BreakShape* ShapeController::GetCurrentShape()
	{
		return FindShape(queue_.Peek(0));
	}

	// 今の形で、プレイヤーの前の床を崩す
	bool ShapeController::TryBreak(Floor& floor, const Cell& playerCell, const Cell& facing)
	{
		const BreakShape* shape = GetCurrentShape();
		if (shape == nullptr || !IsReady())
		{
			return false;
		}

		// 形のマスを崩す。崩せる床が1つもなければ、形は使わない
		shape->CollectCells(playerCell, facing, cells_);
		const int32_t count = floor.ScheduleCellsCollapse(cells_, GameConfig::kCollapseDelay);
		if (count == 0)
		{
			return false;
		}

		// 使ったので、待ち時間を入れて次の形へ
		cooldownTimer_ = GameConfig::kShapeCooldown;
		queue_.Advance();
		return true;
	}

	// 床に今の形の予告を出す
	void ShapeController::UpdatePreview(Floor& floor, const Cell& playerCell, const Cell& facing, bool isVisible)
	{
		const BreakShape* shape = GetCurrentShape();
		if (shape == nullptr || !isVisible)
		{
			floor.ClearPreview();
			return;
		}
		// プレイヤーの前に今の形のマスを集めて、床を光らせる
		shape->CollectCells(playerCell, facing, cells_);
		floor.SetPreview(cells_, shape->GetColor(), GameConfig::kShapePreviewBlend);
	}

	// 画面の右はしのパネルを合わせる
	void ShapeController::UpdatePanel(const StageCamera& camera, const Cell& facing)
	{
		// 今・次・その次 の形を集めて、パネルに渡す (待ち時間中は NOW を暗くする)
		std::array<const BreakShape*, NextShapePanel::kSlotCount> shapes{};
		for (int32_t i = 0; i < NextShapePanel::kSlotCount; ++i)
		{
			shapes[static_cast<size_t>(i)] = FindShape(queue_.Peek(i));
		}
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
