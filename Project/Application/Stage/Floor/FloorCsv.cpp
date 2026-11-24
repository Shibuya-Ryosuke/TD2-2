#include "FloorCsv.h"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace App
{
	namespace
	{
		//先頭のBOMと行末の改行コードを取り除く
		void CleanLine(std::string& line)
		{
			if (line.size() >= 3 &&
				static_cast<unsigned char>(line[0]) == 0xEF &&
				static_cast<unsigned char>(line[1]) == 0xBB &&
				static_cast<unsigned char>(line[2]) == 0xBF)
			{
				line.erase(0, 3);
			}
			if (!line.empty() && line.back() == '\r')
			{
				line.pop_back();
			}
		}
	}

	//CSVを読んでマスの数と床の形を決める
	bool FloorCsv::Load(const std::string& csvPath, FloorLayout& layout)
	{
		std::ifstream file(csvPath);
		if (!file.is_open())
		{
			return false;
		}

		//上の行が奥で1行ずつ読む
		std::vector<std::vector<int32_t>> rows;
		std::string line;
		while (std::getline(file, line))
		{
			CleanLine(line);
			if (line.empty())
			{
				continue;
			}

			//カンマで区切って数字にし、数字でないものは穴
			std::vector<int32_t> row;
			std::istringstream stream(line);
			std::string value;
			while (std::getline(stream, value, ','))
			{
				row.push_back(std::atoi(value.c_str()));
			}
			rows.push_back(row);
		}
		if (rows.empty())
		{
			return false;
		}

		//一番長い行に合わせて足りない所は穴
		size_t maxWidth = 0;
		for (const std::vector<int32_t>& row : rows)
		{
			maxWidth = std::max(maxWidth, row.size());
		}
		if (maxWidth == 0)
		{
			return false;
		}
		layout.width = static_cast<int32_t>(maxWidth);
		layout.depth = static_cast<int32_t>(rows.size());

		//CSVの一番下の行が手前になるように並べ直す
		layout.solid.assign(static_cast<size_t>(layout.width * layout.depth), false);
		for (int32_t z = 0; z < layout.depth; ++z)
		{
			const std::vector<int32_t>& row = rows[static_cast<size_t>(layout.depth - 1 - z)];
			for (int32_t x = 0; x < layout.width; ++x)
			{
				if (static_cast<size_t>(x) < row.size() && row[static_cast<size_t>(x)] == 1)
				{
					layout.solid[static_cast<size_t>(z * layout.width + x)] = true;
				}
			}
		}
		return true;
	}

	//全部床の形
	FloorLayout FloorCsv::CreateDefault(int32_t width, int32_t depth)
	{
		FloorLayout layout{};
		layout.width = width;
		layout.depth = depth;
		layout.solid.assign(static_cast<size_t>(width * depth), true);
		return layout;
	}
}
