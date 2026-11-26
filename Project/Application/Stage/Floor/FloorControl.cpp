#include "Floor.h"

using namespace RyoEngine;

//演出とチュートリアルから床を直接いじるFloorの関数
namespace App
{
	//見えているタイルを全部rateの大きさにして、0なら消す
	void Floor::ShrinkAll(float rate)
	{
		for (int32_t z = 0; z < depth_; ++z)
		{
			for (int32_t x = 0; x < width_; ++x)
			{
				const Cell cell = { x, z };
				FloorTile& tile = At(cell);
				if (!tile.isVisible)
				{
					continue;
				}
				if (rate <= 0.0f)
				{
					tile.isVisible = false;
					continue;
				}
				tile.scaleRate = rate;
			}
		}
	}

	//穴になったマスをCSVのときの床に戻す
	bool Floor::RestoreTile(const Cell& cell)
	{
		if (!Contains(cell))
		{
			return false;
		}
		const FloorTile& tile = GetTile(cell);
		if (tile.isSolid || !tile.isSolidAtStart)
		{
			return false;
		}
		Restore(cell);
		++solidTileCount_;
		return true;
	}

	//残っている床の崩れる予定を全部やめて、ひびの揺れも止める
	void Floor::CancelCollapses()
	{
		for (FloorTile& tile : tiles_)
		{
			if (!tile.isSolid || tile.isIsolated)
			{
				continue;
			}
			tile.remaining = -1.0f;
			tile.shakeRate = 0.0f;
		}
	}

	//崩す予定のマスの穴になるまでの残りを、ハンマーが当たった瞬間に合わせ直す
	void Floor::SyncCollapse(const Cell& cell, float remaining)
	{
		if (!Contains(cell))
		{
			return;
		}
		FloorTile& tile = At(cell);
		if (tile.isSolid && tile.remaining >= 0.0f)
		{
			tile.remaining = remaining;
		}
	}
}
