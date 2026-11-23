#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace App
{
	//CSVから読んだ0が穴で1が床の形
	struct FloorLayout
	{
		int32_t width = 0; //横のマスの数
		int32_t depth = 0; //奥のマスの数
		std::vector<bool> solid; //手前の行から順に左から右へ、床ならtrue
	};

	//上の行が奥で1が床、それ以外は穴の床のCSVを読む
	class FloorCsv
	{
	public:
		//CSVを読んで、読めなかったらfalse
		static bool Load(const std::string& csvPath, FloorLayout& layout);

		//CSVが読めなかったときの全部床の形
		static FloorLayout CreateDefault(int32_t width, int32_t depth);
	};
}
