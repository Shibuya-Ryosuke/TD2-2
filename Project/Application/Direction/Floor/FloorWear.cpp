#include "FloorWear.h"
#include "../../Config/DirectionConfig.h"
#include <algorithm>

namespace App
{
	//タイルの数に合わせて用意する
	void FloorWear::Initialize(size_t tileCount)
	{
		rates_.assign(tileCount, 0.0f);
	}

	//穴のとなりならヒビを広げて、床が元に戻ったらすぐ消す
	float FloorWear::Advance(size_t index, bool isWorn, float deltaTime)
	{
		if (index >= rates_.size())
		{
			return 0.0f;
		}
		float& rate = rates_[index];
		if (!isWorn)
		{
			rate = 0.0f;
			return rate;
		}
		rate = std::min(1.0f, rate + deltaTime / std::max(0.01f, DirectionConfig::kWearGrowTime));
		return rate;
	}
}
