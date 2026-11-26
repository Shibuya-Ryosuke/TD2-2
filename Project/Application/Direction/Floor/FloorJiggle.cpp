#include "FloorJiggle.h"
#include "../../Config/DirectionConfig.h"
#include "../../Stage/Grid/FloorGrid.h"
#include <RyoEngine.h>
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//タイルの数に合わせて用意する
	void FloorJiggle::Initialize(size_t tileCount)
	{
		wobbles_.assign(tileCount, Wobble{});
	}

	//着地したマスからまわりへ波のように広げる
	void FloorJiggle::Trigger(const FloorGrid& grid, const Cell& center, float power)
	{
		if (!grid.Contains(center))
		{
			return;
		}
		const float radius = DirectionConfig::kJiggleRadius;
		const int32_t reach = static_cast<int32_t>(std::ceil(radius));
		for (int32_t dz = -reach; dz <= reach; ++dz)
		{
			for (int32_t dx = -reach; dx <= reach; ++dx)
			{
				const Cell cell = { center.x + dx, center.z + dz };
				const float distance = std::sqrt(static_cast<float>(dx * dx + dz * dz));
				if (!grid.Contains(cell) || distance > radius)
				{
					continue;
				}

				//遠いほど小さく遅れて揺れる
				const float falloff = 1.0f - distance / (radius + 1.0f);
				const float amount = power * falloff * falloff;
				Wobble& wobble = wobbles_[grid.ToIndex(cell)];
				//もっと大きく揺れているマスはそのままにする
				if (std::abs(GetWave(wobble)) > amount)
				{
					continue;
				}
				wobble.time = -distance * DirectionConfig::kJiggleRippleDelay;
				wobble.amount = amount;
				wobble.isBounce = false;
			}
		}
	}

	//歩数だけ遅らせて、マスを上に跳ねさせる
	void FloorJiggle::TriggerBounce(const std::vector<int32_t>& steps, float height, float stepDelay, float speed, float damping) {
		const size_t count = std::min(steps.size(), wobbles_.size());
		for (size_t i = 0; i < count; ++i) {
			//波が届かないマスは揺らさない
			if (steps[i] < 0) {
				continue;
			}
			Wobble& wobble = wobbles_[i];
			wobble.time = -static_cast<float>(steps[i]) * stepDelay;
			wobble.amount = height;
			wobble.isBounce = true;
			wobble.speed = speed;
			wobble.damping = damping;
		}
	}

	//揺れを進める
	void FloorJiggle::Update()
	{
		const float deltaTime = GetDeltaTime();
		for (Wobble& wobble : wobbles_)
		{
			wobble.time += deltaTime;
		}
	}

	//沈んで戻ってぷるぷる小さくなる揺れの値
	float FloorJiggle::GetWave(const Wobble& wobble)
	{
		if (wobble.time < 0.0f || wobble.amount <= 0.0f)
		{
			return 0.0f;
		}
		//跳ねる揺れは、地面パンチが決めた速さで揺らす
		if (wobble.isBounce) {
			return wobble.amount * std::exp(-wobble.damping * wobble.time) * std::sin(wobble.speed * wobble.time);
		}
		return wobble.amount * std::exp(-DirectionConfig::kJiggleDamping * wobble.time)
			* std::sin(DirectionConfig::kJiggleSpeed * wobble.time);
	}

	//そのタイルの今の揺れ
	FloorJiggle::Offset FloorJiggle::Get(size_t index) const
	{
		Offset offset{};
		if (index >= wobbles_.size())
		{
			return offset;
		}
		const Wobble& wobble = wobbles_[index];
		const float wave = GetWave(wobble);
		//上に跳ねるだけ。つぶれたりふくらんだりはしない
		if (wobble.isBounce) {
			offset.offsetY = std::abs(wave);
			return offset;
		}
		offset.offsetY = -wave * DirectionConfig::kJiggleDepth;
		offset.scaleY = std::max(1.0f - wave * DirectionConfig::kJiggleSquash, 0.2f);
		offset.scaleXZ = 1.0f + wave * DirectionConfig::kJiggleWiden;
		return offset;
	}
}
