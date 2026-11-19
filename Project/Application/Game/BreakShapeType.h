#pragma once
#include <cstdint>

namespace App 
{
	/// 床を崩す形の種類 (テトリスの7つの形)
	enum class BreakShapeType : int32_t {
		I,
		O,
		T,
		S,
		Z,
		L,
		J,
		Count, // 形の数 (形ではない。数えるときに使う)
	};

	//>形の種類の数
	inline constexpr int32_t kBreakShapeTypeCount = static_cast<int32_t>(BreakShapeType::Count);
}
