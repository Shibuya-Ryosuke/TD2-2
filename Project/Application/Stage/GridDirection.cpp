#include "GridDirection.h"
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App {
	// 向き → 上下左右 (大きい方の軸)
	Cell GridDirection::FromVector(const Vector3& direction) {
		// x の方が大きければ左右、そうでなければ奥・手前
		if (std::abs(direction.x) > std::abs(direction.z)) {
			if (direction.x > 0.0f) {
				return Cell{ 1, 0 };
			}
			return Cell{ -1, 0 };
		}
		if (direction.z > 0.0f) {
			return Cell{ 0, 1 };
		}
		return Cell{ 0, -1 };
	}

	// 入力 → カメラの向きで直して → 上下左右
	Cell GridDirection::FromInput(const Vector2& input) {
		// カメラの右向きを地面に平らにしたものが「右」、そこから「奥」を作る
		Vector3 right = CameraManager::GetGameCamera().GetRight();
		right.y = 0.0f;
		if (Length(right) > 0.0001f) {
			right = Normalize(right);
		} else {
			right = { 1.0f, 0.0f, 0.0f };
		}
		const Vector3 forward = Cross(right, Vector3{ 0.0f, 1.0f, 0.0f });
		return FromVector(right * input.x + forward * input.y);
	}

	// 上下左右 → 角度 (度)
	float GridDirection::ToYawDeg(const Cell& direction) {
		// atan2 で角度を出して、ラジアン → 度
		return std::atan2(static_cast<float>(direction.x), static_cast<float>(direction.z)) * 180.0f / std::numbers::pi_v<float>;
	}
}
