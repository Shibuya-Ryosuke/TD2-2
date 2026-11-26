#include "TutorialMap.h"
#include "../../../Config/TutorialConfig.h"
#include "../../../Stage/Floor/Floor.h"
#include "../../../Stage/Floor/FloorCsv.h"

namespace App
{
	//csvを読んで数字を覚える
	bool TutorialMap::Load()
	{
		FloorLayout layout{};
		if (!FloorCsv::Load(TutorialConfig::kCsvPath, layout))
		{
			width_ = 0;
			depth_ = 0;
			codes_.clear();
			return false;
		}
		width_ = layout.width;
		depth_ = layout.depth;
		codes_ = layout.codes;
		return true;
	}

	//その数字のマスを全部集める
	std::vector<Cell> TutorialMap::Find(int32_t code) const
	{
		std::vector<Cell> cells;
		for (int32_t z = 0; z < depth_; ++z)
		{
			for (int32_t x = 0; x < width_; ++x)
			{
				if (codes_[static_cast<size_t>(z * width_ + x)] == code)
				{
					cells.push_back({ x, z });
				}
			}
		}
		return cells;
	}

	//その数字の最初のマス
	Cell TutorialMap::FindFirst(int32_t code) const
	{
		const std::vector<Cell> cells = Find(code);
		if (cells.empty())
		{
			return { width_ / 2, depth_ / 2 };
		}
		return cells.front();
	}

	//そのマスの数字
	int32_t TutorialMap::GetCode(const Cell& cell) const
	{
		if (cell.x < 0 || cell.z < 0 || cell.x >= width_ || cell.z >= depth_)
		{
			return 0;
		}
		return codes_[static_cast<size_t>(cell.z * width_ + cell.x)];
	}

	//島をちゃんと切り離せるように、島と積み木とつなぐマスと、つなぐマスのまわりを床に戻す
	void TutorialMap::RestoreIsland(Floor& floor) const
	{
		constexpr Cell kDirections[4] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };
		for (int32_t z = 0; z < depth_; ++z)
		{
			for (int32_t x = 0; x < width_; ++x)
			{
				const Cell cell = { x, z };
				const int32_t code = GetCode(cell);
				if (code != TutorialConfig::kCodeIsland && code != TutorialConfig::kCodeSecondBlock && code != TutorialConfig::kCodeBridge)
				{
					continue;
				}
				floor.RestoreTile(cell);
				if (code != TutorialConfig::kCodeBridge)
				{
					continue;
				}
				//つなぐマスの先も穴だとすぐ切り離されてしまうので戻す
				for (const Cell& direction : kDirections)
				{
					const Cell next = cell + direction;
					if (GetCode(next) != TutorialConfig::kCodeWall)
					{
						floor.RestoreTile(next);
					}
				}
			}
		}
	}
}
