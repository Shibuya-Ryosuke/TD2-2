#include "FloorGrid.h"
#include "../Game/GameConfig.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		// current から target へ、最大 maxDistance だけ近づける
		float MoveTowardsFloat(float current, float target, float maxDistance)
		{
			if (std::abs(target - current) <= maxDistance)
			{
				return target;
			}
			if (target > current)
			{
				return current + maxDistance;
			}
			return current - maxDistance;
		}
	}

	// マスの数と高さを決める
	void FloorGrid::SetupGrid(int32_t width, int32_t depth, float height)
	{
		width_ = width;
		depth_ = depth;
		height_ = height;

		// 床全体の大きさはいつも同じ。マスの数で割って1マスの大きさにする
		tileSizeX_ = GameConfig::kFloorSize / static_cast<float>(width_);
		tileSizeZ_ = GameConfig::kFloorSize / static_cast<float>(depth_);
	}

	// 位置 → マス
	Cell FloorGrid::GetCellAt(const Vector3& position) const
	{
		// 床の真ん中が 0 なので、マスの数の半分ずらして数える
		Cell cell{};
		cell.x = static_cast<int32_t>(std::floor(position.x / tileSizeX_ + static_cast<float>(width_) * 0.5f));
		cell.z = static_cast<int32_t>(std::floor(position.z / tileSizeZ_ + static_cast<float>(depth_) * 0.5f));
		return cell;
	}

	// マス → マスの真ん中の位置
	Vector3 FloorGrid::GetCellCenter(const Cell& cell) const
	{
		Vector3 center{};
		center.x = (static_cast<float>(cell.x) + 0.5f - static_cast<float>(width_) * 0.5f) * tileSizeX_;
		center.y = height_;
		center.z = (static_cast<float>(cell.z) + 0.5f - static_cast<float>(depth_) * 0.5f) * tileSizeZ_;
		return center;
	}

	// タイルのいつもの位置 (タイルの真ん中)
	Vector3 FloorGrid::GetRestPosition(const Cell& cell) const
	{
		Vector3 position = GetCellCenter(cell);
		position.y -= GameConfig::kTileThickness * 0.5f;
		return position;
	}

	// 床の外に出ないようにする
	Vector3 FloorGrid::ClampToBoundary(const Vector3& position, float radius) const
	{
		// はみ出さないための余白と、動ける範囲
		const float marginX = std::min(std::max(0.001f, radius), tileSizeX_ * 0.5f);
		const float marginZ = std::min(std::max(0.001f, radius), tileSizeZ_ * 0.5f);
		const float limitX = static_cast<float>(width_) * tileSizeX_ * 0.5f - marginX;
		const float limitZ = static_cast<float>(depth_) * tileSizeZ_ * 0.5f - marginZ;

		// 範囲の中におさめる
		Vector3 result = position;
		result.x = std::clamp(result.x, -limitX, limitX);
		result.z = std::clamp(result.z, -limitZ, limitZ);
		return result;
	}

	// 床の範囲の中か
	bool FloorGrid::Contains(const Cell& cell) const
	{
		if (cell.x < 0 || cell.x >= width_)
		{
			return false;
		}
		if (cell.z < 0 || cell.z >= depth_)
		{
			return false;
		}
		return true;
	}

	// マス → 配列の番号
	size_t FloorGrid::ToIndex(const Cell& cell) const
	{
		return static_cast<size_t>(cell.z * width_ + cell.x);
	}

	// 配列の番号 → マス
	Cell FloorGrid::ToCell(size_t index) const
	{
		const int32_t value = static_cast<int32_t>(index);
		return Cell{ value % width_, value / width_ };
	}

	// マスの向き → 1マス分のずれ
	Vector3 FloorGrid::StepOffset(const Cell& direction) const
	{
		return Vector3{ static_cast<float>(direction.x) * tileSizeX_, 0.0f, static_cast<float>(direction.z) * tileSizeZ_ };
	}

	// マスの線にそって進む (先に x、次に z)
	Vector3 FloorGrid::AlongGrid(const Vector3& start, const Vector3& end, float progress)
	{
		if (progress >= 1.0f)
		{
			return end;
		}
		// 進んだ距離 (x と z の合計の長さのうち、progress の分)。先に x を進めて、残りで z を進める
		const float distanceX = std::abs(end.x - start.x);
		const float distanceZ = std::abs(end.z - start.z);
		const float travel = (distanceX + distanceZ) * progress;
		Vector3 result{};
		result.x = MoveTowardsFloat(start.x, end.x, travel);
		result.y = start.y + (end.y - start.y) * progress;
		result.z = MoveTowardsFloat(start.z, end.z, std::max(0.0f, travel - distanceX));
		return result;
	}
}
