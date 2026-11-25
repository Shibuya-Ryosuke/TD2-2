#include "PlayerTurnAround.h"
#include "../../Config/DirectionConfig.h"
#include <RyoEngine.h>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//目標の度数へ近い回り方で少しずつ回す
	void PlayerTurnAround::Update(float targetYawDeg)
	{
		//マイナス180から180の差にして近い方へ回す
		float difference = std::fmod(targetYawDeg - yawDeg_, 360.0f);
		if (difference > 180.0f)
		{
			difference -= 360.0f;
		}
		if (difference < -180.0f)
		{
			difference += 360.0f;
		}

		//1フレームに回れる分だけ回り、残りが少なければぴったり合わせる
		const float step = DirectionConfig::kPlayerTurnSpeed * GetDeltaTime();
		if (std::abs(difference) <= step)
		{
			yawDeg_ = targetYawDeg;
			return;
		}
		if (difference > 0.0f)
		{
			yawDeg_ += step;
		}
		else
		{
			yawDeg_ -= step;
		}
	}
}
