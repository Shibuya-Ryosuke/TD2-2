#include "TransitionView.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		constexpr float kMinDepth = 0.05f; //0で割らないためのいちばん近い距離
	}

	//その奥行きで大きさを何倍にするか
	float TransitionView::GetScaleAt(float depth) const
	{
		if (!isOrthographic)
		{
			return 1.0f; //遠近法は近づけば勝手に大きく映る
		}
		//平行投影は遠近法で見える倍率の分だけ大きくする
		return refDepth / std::max(depth, kMinDepth);
	}

	//カメラから見た位置を親から見た位置にする
	Vector3 TransitionView::ToLocalPosition(const Vector3& position) const
	{
		if (!isOrthographic)
		{
			return position;
		}
		//平行投影は近いほど真ん中から外へ広げる
		const float scale = GetScaleAt(position.z);
		return Vector3{ position.x * scale, position.y * scale, position.z };
	}
}
