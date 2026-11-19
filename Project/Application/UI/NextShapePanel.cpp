#include "NextShapePanel.h"
#include "../Camera/StageCamera.h"
#include "../Game/BreakShape.h"
#include "../Game/GameConfig.h"
#include <algorithm>

using namespace RyoEngine;

namespace App {
	// 最初の状態にする (まだ何も出さない)
	void NextShapePanel::Initialize() {
		activeCounts_.fill(0);
	}

	// パネルの真ん中の横の位置 (画面の右はしから、パネルの幅の半分だけ内側)
	float NextShapePanel::GetCenterX() const {
		return 1.0f - GameConfig::kPanelWidthRate;
	}

	// 画面の位置 (-1 〜 1) と奥行きから、3Dの位置を求める
	// 平行投影なので、カメラから同じ向きに進めば、遠くても近くても画面の同じ所に映る
	Vector3 NextShapePanel::ToWorld(const ScreenBasis& basis, float screenX, float screenY, float depth) {
		return basis.origin
			+ basis.right * (screenX * basis.halfWidth)
			+ basis.up * (screenY * basis.halfHeight)
			+ basis.forward * depth;
	}

	// カメラに合わせて、ブロックを並べ直す
	void NextShapePanel::Update(const StageCamera& camera, const std::array<const BreakShape*, kSlotCount>& shapes, const Cell& facing) {
		// カメラの向きと、画面に映る広さ
		ScreenBasis basis{};
		basis.origin = camera.GetViewPosition();
		basis.right = camera.GetRight();
		basis.up = camera.GetUp();
		basis.forward = camera.GetForward();
		basis.halfHeight = camera.GetOrthoSize();
		basis.halfWidth = basis.halfHeight * camera.GetAspectRatio();

		for (int32_t slot = 0; slot < kSlotCount; ++slot) {
			UpdateSlot(slot, basis, shapes[static_cast<size_t>(slot)], facing);
		}
	}

	// 1つの形のブロックを置く
	void NextShapePanel::UpdateSlot(int32_t slot, const ScreenBasis& basis, const BreakShape* shape, const Cell& facing) {
		const size_t index = static_cast<size_t>(slot);
		activeCounts_[index] = 0;
		if (shape == nullptr) {
			return;
		}

		// プレイヤーが (0, 0) にいるとして、向きに合わせた形のマスを集める (ステージと同じ決め方)
		shape->CollectCells({ 0, 0 }, facing, cells_);
		if (cells_.empty()) {
			return;
		}

		// 形の真ん中がパネルの置き場所に来るように、マスの範囲を調べる
		Cell minCell = cells_.front();
		Cell maxCell = cells_.front();
		for (const Cell& cell : cells_) {
			minCell.x = std::min(minCell.x, cell.x);
			minCell.z = std::min(minCell.z, cell.z);
			maxCell.x = std::max(maxCell.x, cell.x);
			maxCell.z = std::max(maxCell.z, cell.z);
		}
		const float centerX = static_cast<float>(minCell.x + maxCell.x) * 0.5f;
		const float centerZ = static_cast<float>(minCell.z + maxCell.z) * 0.5f;

		// ブロックが足りなければ作る (同じものを使い回す)
		std::vector<std::unique_ptr<Model>>& blocks = blocks_[index];
		while (blocks.size() < cells_.size()) {
			std::unique_ptr<Model> block = Model::Create(GameConfig::kCubeModel);
			block->SetCastShadow(false);
			blocks.push_back(std::move(block));
		}

		// 見た目 (形の色そのまま。光らせない)
		const float cellSize = GameConfig::kPanelCellSize[index];
		const float blockSize = cellSize * GameConfig::kPanelBlockFill / GameConfig::kCubeModelSize;
		const float blockHeight = cellSize * GameConfig::kPanelBlockHeight / GameConfig::kCubeModelSize;
		const Vector4& color = shape->GetColor();

		// ステージの床と同じ向き (x と z が床の横と奥) に並べる
		const Vector3 anchor = ToWorld(basis, GetCenterX(), GameConfig::kPanelSlotY[index], GameConfig::kPanelDepth);
		for (size_t i = 0; i < cells_.size(); ++i) {
			const Vector3 offset = {
				(static_cast<float>(cells_[i].x) - centerX) * cellSize,
				0.0f,
				(static_cast<float>(cells_[i].z) - centerZ) * cellSize };

			Model* block = blocks[i].get();
			block->SetScale({ blockSize, blockHeight, blockSize });
			block->SetRotate({ 0.0f, 0.0f, 0.0f });
			block->SetTranslate(anchor + offset);
			block->SetColor(color);
			block->TransferMatrix();
		}
		activeCounts_[index] = cells_.size();
	}

	// ブロックを描く
	void NextShapePanel::Draw() {
		for (size_t slot = 0; slot < blocks_.size(); ++slot) {
			for (size_t i = 0; i < activeCounts_[slot]; ++i) {
				blocks_[slot][i]->Draw();
			}
		}
	}

	// NOW / NEXT の文字 (形の少し上に出す)
	void NextShapePanel::DrawLabels() const {
		// 文字の位置はゲーム画面の解像度 (今は 1920 x 1080) が基準
		const float screenWidth = static_cast<float>(DirectXCommon::GetGameWidth());
		const float screenHeight = static_cast<float>(DirectXCommon::GetGameHeight());

		// 画面の位置 (-1 〜 1) を、文字の位置 (ピクセル。左上が 0) に直す
		const float left = (GetCenterX() - GameConfig::kPanelWidthRate + 1.0f) * 0.5f * screenWidth + 12.0f;
		const float nowY = (1.0f - (GameConfig::kPanelSlotY[0] + GameConfig::kPanelLabelOffsetY)) * 0.5f * screenHeight;
		const float nextY = (1.0f - (GameConfig::kPanelSlotY[1] + GameConfig::kPanelLabelOffsetY)) * 0.5f * screenHeight;
		PrintText("NOW", { left, nowY });
		PrintText("NEXT", { left, nextY });
	}
}
