#include "TransitionView.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		constexpr float kMinDepth = 0.05f; // 0 で割らないための、いちばん近い距離
	}

	// その奥行きで、大きさを何倍にするか
	float TransitionView::GetScaleAt(float depth) const
	{
		if (!isOrthographic)
		{
			return 1.0f; // 遠近法のカメラは、近づけば勝手に大きく映る
		}
		// 平行投影: 遠近法なら「refDepth / 距離」倍に見えるので、その分だけ大きくする
		return refDepth / std::max(depth, kMinDepth);
	}

	// カメラから見た位置 → 親 (カメラの位置・向き) から見た位置
	Vector3 TransitionView::ToLocalPosition(const Vector3& position) const
	{
		if (!isOrthographic)
		{
			return position;
		}
		// 平行投影: 近いほど真ん中から外へ広げる (遠近法と同じ見た目)。奥行きはそのまま (重なりの順番に使う)
		const float scale = GetScaleAt(position.z);
		return Vector3{ position.x * scale, position.y * scale, position.z };
	}
}
