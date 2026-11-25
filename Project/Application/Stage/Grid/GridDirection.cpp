#include "GridDirection.h"
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App
{
	//向きを大きい方の軸で上下左右にする
	Cell GridDirection::FromVector(const Vector3& direction)
	{
		//xの方が大きければ左右、そうでなければ奥か手前
		if (std::abs(direction.x) > std::abs(direction.z))
		{
			if (direction.x > 0.0f)
			{
				return Cell{ 1, 0 };
			}
			return Cell{ -1, 0 };
		}
		if (direction.z > 0.0f)
		{
			return Cell{ 0, 1 };
		}
		return Cell{ 0, -1 };
	}

	//入力をカメラの向きで直して上下左右にする
	Cell GridDirection::FromInput(const Vector2& input)
	{
		return FromVector(ToWorld(input));
	}

	//入力をカメラの向きで直した斜めもある向き
	Vector3 GridDirection::ToWorld(const Vector2& input)
	{
		//カメラの右向きを地面に平らにして右と奥を作る
		Vector3 right = CameraManager::GetGameCamera().GetRight();
		right.y = 0.0f;
		if (Length(right) > 0.0001f)
		{
			right = Normalize(right);
		}
		else
		{
			right = { 1.0f, 0.0f, 0.0f };
		}
		const Vector3 forward = Cross(right, Vector3{ 0.0f, 1.0f, 0.0f });
		return right * input.x + forward * input.y;
	}

	//上下左右を度の角度にする
	float GridDirection::ToYawDeg(const Cell& direction)
	{
		//atan2の角度をラジアンから度にする
		return std::atan2(static_cast<float>(direction.x), static_cast<float>(direction.z)) * 180.0f / std::numbers::pi_v<float>;
	}

	//ワールドの向きを度の角度にする
	float GridDirection::ToYawDeg(const Vector3& direction)
	{
		return std::atan2(direction.x, direction.z) * 180.0f / std::numbers::pi_v<float>;
	}
}
