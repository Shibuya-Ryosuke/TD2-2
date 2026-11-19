#pragma once
#include <cstdint>

namespace App
{
	// 床を崩す形の種類 (5つ)。並び順は BreakShape::CreateDefaultShapes と同じ
	// P がプレイヤー、X が崩れるマス (上がプレイヤーの前)
	enum class BreakShapeType : int32_t
	{
		X,     // X字      X.X / .X. / XPX  (プレイヤーの真横も崩れる)
		L,     // L字      X.. / XXX / .P.
		Totu,  // 逆T(長い) .X. / .X. / XXX / .P.
		Line,  // 縦3本    X / X / X / P
		Block, // 3x2     XXX / XXX / .P.
		Count, // 形の数 (形ではない。数えるときに使う)
	};

	// 形の種類の数 (5)
	inline constexpr int32_t kBreakShapeTypeCount = static_cast<int32_t>(BreakShapeType::Count);
}
