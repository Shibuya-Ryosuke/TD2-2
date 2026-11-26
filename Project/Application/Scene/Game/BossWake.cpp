#include "BossWake.h"
#include "../../Config/DirectionConfig.h"
#include "../../Config/TutorialConfig.h"
#include "../../Direction/GameDirection.h"
#include "../../Object/Boss/Boss.h"
#include "../../Stage/Floor/Floor.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	//寝ているボスの位置から始める
	void BossWake::Begin(const Vector3& bossRest)
	{
		bossRest_ = bossRest;
		timer_ = 0.0f;
	}

	//少し寝てから、はっと跳ね起きて着地したら終わる
	bool BossWake::Update(float deltaTime, Floor& floor, Boss& boss, GameDirection& direction)
	{
		timer_ += deltaTime;
		const float wake = timer_ - TutorialConfig::kWakeDozeTime;
		if (wake < 0.0f)
		{
			return false;
		}

		//跳ねた頂点で寝た姿から立った姿へ切り替わる
		const float t = std::clamp(wake / TutorialConfig::kWakeTime, 0.0f, 1.0f);
		const float hop = 4.0f * TutorialConfig::kWakeHopHeight * t * (1.0f - t);
		boss.SetPosition({ bossRest_.x, bossRest_.y + hop, bossRest_.z });
		if (t >= 0.5f)
		{
			boss.SetSleep(0.0f);
		}
		if (t < 1.0f)
		{
			return false;
		}

		//どすんと着地して画面と足場が揺れる
		boss.SetPosition(bossRest_);
		floor.Jiggle(floor.GetCellAt(bossRest_), DirectionConfig::kJiggleBossPower);
		direction.Shake(TutorialConfig::kWakeShake);
		return true;
	}
}
