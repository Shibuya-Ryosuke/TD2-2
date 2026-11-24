#pragma once
#include <RyoEngine.h>
#include <cmath>
#include <numbers>

//ボスと残像で使う値と計算
namespace App::BossDetail
{
	//ボスのローカルの拳の位置
	inline constexpr RyoEngine::Vector3 kFistRest = { 1.15f, 0.85f, 0.2f }; //拳のいつもの位置
	inline constexpr float kFistPullBack = 0.65f; //予告で拳を後ろに引く距離
	inline constexpr RyoEngine::Vector3 kFistSize = { 0.75f, 0.75f, 0.9f }; //パンチの箱の拳の大きさ

	//比べるだけの長さの2乗
	inline float LengthSquared(const RyoEngine::Vector3& v)
	{
		return v.x * v.x + v.y * v.y + v.z * v.z;
	}

	//Y軸で回す
	inline RyoEngine::Vector3 RotateY(const RyoEngine::Vector3& v, float degree)
	{
		const float radian = degree * std::numbers::pi_v<float> / 180.0f;
		const float c = std::cos(radian);
		const float s = std::sin(radian);
		return RyoEngine::Vector3{ v.x * c + v.z * s, v.y, -v.x * s + v.z * c };
	}
}
