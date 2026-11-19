#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace App
{
	// CSV から読んだ床の形 (0: 穴 / 1: 床)
	struct FloorLayout
	{
		int32_t width = 0;        // 横のマスの数
		int32_t depth = 0;        // 奥のマスの数
		std::vector<bool> solid;  // 手前の行から順に、左から右へ。床なら true
	};

	// 床の CSV を読むクラス
	// CSV の上の行が奥、下の行が手前。1 が床、それ以外 (0・空欄・文字) は穴
	class FloorCsv
	{
	public:
		// CSV を読む。読めなかったら false
		static bool Load(const std::string& csvPath, FloorLayout& layout);

		// CSV が読めなかったときの床 (全部床)
		static FloorLayout CreateDefault(int32_t width, int32_t depth);
	};
}
