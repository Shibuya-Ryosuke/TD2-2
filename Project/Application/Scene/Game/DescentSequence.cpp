#include "DescentSequence.h"
#include "../../Config/BossFallConfig.h"
#include "../../Config/LayerConfig.h"
#include "../../Object/Boss/Boss.h"
#include "../../Stage/Floor/Floor.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	//ボスの位置から、下の層のlandingへ落ち始める
	void DescentSequence::Begin(const Vector3& bossPosition, const Vector3& landing)
	{
		dropOrigin_ = bossPosition;
		landing_ = landing;
		isBossLanded_ = false;
		timer_ = 0.0f;
	}

	//だんだん速く落ちて、横は着地点へなめらかに寄せ、着いたフレームだけtrueを返す
	bool DescentSequence::Advance(float deltaTime, Floor& current, Boss& boss)
	{
		if (isBossLanded_)
		{
			return false;
		}
		timer_ += deltaTime;
		const float t = std::clamp(timer_ / BossFallConfig::kDropTime, 0.0f, 1.0f);
		const float fall = t * t;
		const float slide = t * t * (3.0f - 2.0f * t);

		//切り離された床も一緒に落ちる
		current.AnimateDrop(LayerConfig::kFloorSpacing * fall);
		boss.SetPosition({
			dropOrigin_.x + (landing_.x - dropOrigin_.x) * slide,
			dropOrigin_.y + (landing_.y - dropOrigin_.y) * fall,
			dropOrigin_.z + (landing_.z - dropOrigin_.z) * slide });
		if (t < 1.0f)
		{
			return false;
		}

		//着いたら落ちた床を消す
		isBossLanded_ = true;
		current.FinishDrop();
		return true;
	}
}
