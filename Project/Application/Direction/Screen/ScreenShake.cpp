#include "ScreenShake.h"
#include "../../Config/DirectionConfig.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//揺らし始める
	void ScreenShake::Play(float power)
	{
		power_ = std::max(power_, power);
	}

	//揺れを弱めながらカメラをずらす分を返す
	Vector3 ScreenShake::Update(float deltaTime)
	{
		if (power_ <= 0.0f)
		{
			return { 0.0f, 0.0f, 0.0f };
		}
		time_ += deltaTime;
		power_ = std::max(0.0f, power_ - deltaTime / DirectionConfig::kShakeTime);

		//速さの違う波を重ねてでたらめっぽく揺らし、弱まるほど小さくする
		const float t = time_ * DirectionConfig::kShakeSpeed;
		const float amount = power_ * power_;
		return
		{
			(std::sin(t) + 0.5f * std::sin(t * 2.3f + 1.0f)) * amount,
			(std::cos(t * 1.7f) + 0.5f * std::sin(t * 3.1f + 2.0f)) * amount,
			(std::sin(t * 1.3f + 0.5f) * 0.5f) * amount
		};
	}
}
