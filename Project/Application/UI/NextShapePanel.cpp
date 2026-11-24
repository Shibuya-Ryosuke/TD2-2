#include "NextShapePanel.h"
#include "../Camera/StageCamera.h"
#include "../Object/Player/Shape/BreakShape.h"
#include "../Config/GameConfig.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		constexpr Cell kFixedFacing = { 0, 1 }; //奥向きで止めておくNOW以外の形の向き
	}

	//最初の状態にする
	void NextShapePanel::Initialize(int32_t selectedIndex)
	{
		displayIndex_ = static_cast<float>(selectedIndex);
		activeCounts_.clear();
	}

	//画面の位置と奥行きから3Dの位置を求める
	//平行投影なので同じ向きに進めば画面の同じ所に映る
	Vector3 NextShapePanel::ToWorld(const ScreenBasis& basis, float screenX, float screenY, float depth)
	{
		return basis.origin
			+ basis.right * (screenX * basis.halfWidth)
			+ basis.up * (screenY * basis.halfHeight)
			+ basis.forward * depth;
	}

	//番号の差を輪の近い向きの差に収める
	float NextShapePanel::WrapOffset(float offset, float count)
	{
		float result = std::fmod(offset, count);
		if (result > count * 0.5f)
		{
			result -= count;
		}
		if (result < -count * 0.5f)
		{
			result += count;
		}
		return result;
	}

	//輪を選んでいる形の方へ回す
	void NextShapePanel::UpdateRotation(int32_t selectedIndex, size_t count)
	{
		const float countF = static_cast<float>(count);
		const float difference = WrapOffset(static_cast<float>(selectedIndex) - displayIndex_, countF);
		//続けて押したら遅れすぎないように残りが多いほど速く回す
		const float step = GetDeltaTime() / GameConfig::kRingTurnDuration * std::max(1.0f, std::abs(difference));

		//近い向きに回り、残りが少なければぴったり合わせる
		if (std::abs(difference) <= step)
		{
			displayIndex_ = static_cast<float>(selectedIndex);
			return;
		}
		if (difference > 0.0f)
		{
			displayIndex_ += step;
		}
		else
		{
			displayIndex_ -= step;
		}

		//0からcountの中に収める
		displayIndex_ = std::fmod(displayIndex_ + countF, countF);
	}

	//輪を回してブロックを並べ直す
	void NextShapePanel::Update(const StageCamera& camera, const std::vector<BreakShape>& shapes, int32_t selectedIndex, const Cell& facing, bool isNowReady)
	{
		const size_t count = shapes.size();
		if (count == 0)
		{
			return;
		}
		blocks_.resize(count);
		activeCounts_.resize(count, 0);
		UpdateRotation(selectedIndex, count);

		//カメラの向きと画面に映る広さ
		ScreenBasis basis{};
		basis.origin = camera.GetViewPosition();
		basis.right = camera.GetRight();
		basis.up = camera.GetUp();
		basis.forward = camera.GetForward();
		basis.halfHeight = camera.GetOrthoSize();
		basis.halfWidth = basis.halfHeight * camera.GetAspectRatio();

		//手前を0として形ごとに輪の上の角度を決めて置く
		const float countF = static_cast<float>(count);
		const float anglePerShape = std::numbers::pi_v<float> * 2.0f / countF;
		for (size_t i = 0; i < count; ++i)
		{
			const float offset = WrapOffset(static_cast<float>(i) - displayIndex_, countF);

			//NOWだけプレイヤーの向きに合わせ、ほかは奥向きで止める
			//NOWを置けない間は暗くする
			Cell itemFacing = kFixedFacing;
			float brightness = 1.0f;
			if (static_cast<int32_t>(i) == selectedIndex)
			{
				itemFacing = facing;
				if (!isNowReady)
				{
					brightness = GameConfig::kPanelWaitBrightness;
				}
			}
			UpdateItem(i, basis, shapes[i], itemFacing, offset * anglePerShape, brightness);
		}
	}

	//1つの形のブロックを置く
	void NextShapePanel::UpdateItem(size_t index, const ScreenBasis& basis, const BreakShape& shape, const Cell& facing, float angle, float brightness)
	{
		activeCounts_[index] = 0;

		//プレイヤーが原点にいるとして向きに合わせた形のマスを集める
		shape.CollectCells({ 0, 0 }, facing, cells_);
		if (cells_.empty())
		{
			return;
		}

		//形の真ん中が輪の上の場所に来るようにマスの範囲を調べる
		Cell minCell = cells_.front();
		Cell maxCell = cells_.front();
		for (const Cell& cell : cells_)
		{
			minCell.x = std::min(minCell.x, cell.x);
			minCell.z = std::min(minCell.z, cell.z);
			maxCell.x = std::max(maxCell.x, cell.x);
			maxCell.z = std::max(maxCell.z, cell.z);
		}
		const float centerX = static_cast<float>(minCell.x + maxCell.x) * 0.5f;
		const float centerZ = static_cast<float>(minCell.z + maxCell.z) * 0.5f;

		//足りないブロックを作って使い回す
		std::vector<std::unique_ptr<Model>>& blocks = blocks_[index];
		while (blocks.size() < cells_.size())
		{
			std::unique_ptr<Model> block = Model::Create(GameConfig::kCubeModel);
			block->SetCastShadow(false);
			blocks.push_back(std::move(block));
		}

		//奥ほど小さく暗く遠くにする手前らしさ
		const float front = (1.0f + std::cos(angle)) * 0.5f;
		const float scale = GameConfig::kRingBackScale + (1.0f - GameConfig::kRingBackScale) * front;
		const float shade = GameConfig::kRingBackBrightness + (1.0f - GameConfig::kRingBackBrightness) * front;
		const float depth = GameConfig::kPanelDepth + GameConfig::kRingDepthSpread * (1.0f - front);

		//手前が下の輪の上の場所
		const float screenX = GameConfig::kRingCenterX + std::sin(angle) * GameConfig::kRingRadiusX;
		const float screenY = GameConfig::kRingCenterY - std::cos(angle) * GameConfig::kRingRadiusY;
		const Vector3 anchor = ToWorld(basis, screenX, screenY, depth);

		//光らせずに形の色に明るさをかける
		const float cellSize = GameConfig::kRingCellSize * scale;
		const float blockSize = cellSize * GameConfig::kPanelBlockFill / GameConfig::kCubeModelSize;
		const float blockHeight = cellSize * GameConfig::kPanelBlockHeight / GameConfig::kCubeModelSize;
		const Vector4& shapeColor = shape.GetColor();
		const float light = brightness * shade;
		const Vector4 color = { shapeColor.x * light, shapeColor.y * light, shapeColor.z * light, shapeColor.w };

		//ステージの床と同じ向きに並べる
		for (size_t i = 0; i < cells_.size(); ++i)
		{
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

	//ブロックを描く
	void NextShapePanel::Draw()
	{
		for (size_t index = 0; index < blocks_.size(); ++index)
		{
			for (size_t i = 0; i < activeCounts_[index]; ++i)
			{
				blocks_[index][i]->Draw();
			}
		}
	}
}
