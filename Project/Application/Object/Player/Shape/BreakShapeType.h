#pragma once
#include <cstdint>

namespace App
{
	//上がプレイヤーの前でPがプレイヤー、Xが崩れるマスの4つの形の種類
	enum class BreakShapeType : int32_t
	{
		L, //X.. / XXX / .P.
		Totu, //.X. / .X. / XXX / .P.
		Line, //X / X / X / P
		Block, //XXX / XXX / .P.
		Count, //数えるときに使う形の数
	};

	//形の種類の数
	inline constexpr int32_t kBreakShapeTypeCount = static_cast<int32_t>(BreakShapeType::Count);
}
