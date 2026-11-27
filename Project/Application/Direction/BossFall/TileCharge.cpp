#include "TileCharge.h"
#include "../../Config/BossFallConfig.h"
#include "../../Stage/Floor/Floor.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//upperの残った足場をためる準備をして、heldのマスはプレイヤーが乗っているので浮かせない
	void TileCharge::Begin(Floor& upper, const Cell& held)
	{
		upper.PrepareBarrage();
		cells_.clear();
		for (int32_t z = 0; z < upper.GetDepth(); ++z)
		{
			for (int32_t x = 0; x < upper.GetWidth(); ++x)
			{
				const Cell cell = { x, z };
				if (upper.IsSolid(cell))
				{
					cells_.push_back(cell);
				}
			}
		}
		held_ = held;
		isHolding_ = true;
		heldSink_ = 0.0f;
		releaseTimer_ = 0.0f;
		timer_ = 0.0f;
	}

	//プレイヤーが飛び立ったので、乗っていた足場もほかの足場と一緒に浮かせる
	void TileCharge::Release()
	{
		isHolding_ = false;
		releaseTimer_ = 0.0f;
	}

	//ドクンと光と浮きを進めて足場の位置と光を入れる
	void TileCharge::Update(float deltaTime, Floor& upper)
	{
		using namespace BossFallConfig;
		timer_ += deltaTime;
		if (!isHolding_)
		{
			releaseTimer_ += deltaTime;
		}

		//ドクンの回数だけ段階的に浮いて光が強くなり、ドクンの瞬間だけはねて強く光る
		const int32_t beat = std::clamp(static_cast<int32_t>(timer_ / kChargePulseTime), 0, kChargePulses - 1);
		const float level = static_cast<float>(beat + 1) / static_cast<float>(kChargePulses);
		const float sinceBeat = timer_ - static_cast<float>(beat) * kChargePulseTime;
		const float kick = std::exp(-kChargePulseDecay * sinceBeat);
		const float lift = kChargeLift * level + kChargePulseLift * kick;
		upper.SetChargeGlow(kChargeColor, kChargeGlowMin + (kChargeGlowMax - kChargeGlowMin) * level + kChargePulseGlow * kick);

		for (size_t i = 0; i < cells_.size(); ++i)
		{
			const Cell& cell = cells_[i];
			if (!upper.IsSolid(cell))
			{
				continue;
			}
			//ためきったあとはマスごとに少しずらしてふわふわ
			const float hover = kChargeHoverHeight * std::sin(timer_ * kChargeHoverSpeed + static_cast<float>(i) * kChargeHoverPhaseStep) * level;
			float height = lift + hover;
			if (cell == held_)
			{
				if (isHolding_)
				{
					height = -heldSink_;
				}
				else
				{
					const float t = std::clamp(releaseTimer_ / kChargeCatchUpTime, 0.0f, 1.0f);
					height = -heldSink_ + (height + heldSink_) * t;
				}
			}
			upper.SetTileMotion(cell, upper.GetRestPosition(cell) + Vector3{ 0.0f, height, 0.0f }, { 0.0f, 0.0f, 0.0f }, 1.0f);
		}
	}
}
