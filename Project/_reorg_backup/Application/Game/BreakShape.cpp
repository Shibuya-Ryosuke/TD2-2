#include "BreakShape.h"

using namespace RyoEngine;

namespace App
{
	// 種類・色・形 を決めて作る
	BreakShape::BreakShape(BreakShapeType type, const Vector4& color, const std::vector<std::string>& rows)
		: type_(type), color_(color), rows_(rows)
	{
		// 'P' (プレイヤーのいるマス) の場所を探しておく
		for (size_t row = 0; row < rows_.size(); ++row)
		{
			const size_t column = rows_[row].find('P');
			if (column != std::string::npos)
			{
				playerRow_ = static_cast<int32_t>(row);
				playerColumn_ = static_cast<int32_t>(column);
			}
		}
	}

	// プレイヤーの前に、向きに合わせて形を置いたときのマスを集める
	void BreakShape::CollectCells(const Cell& playerCell, const Cell& facing, std::vector<Cell>& result) const
	{
		// 前の結果を消す
		result.clear();

		// 向きに対して右側の向き (奥を向いているときは右 = +x)
		const Cell side = { facing.z, -facing.x };

		// 'X' のマスを1つずつ: 'P' から見て 前へ forward、右へ right ずらした所
		for (size_t row = 0; row < rows_.size(); ++row)
		{
			const std::string& line = rows_[row];
			for (size_t column = 0; column < line.size(); ++column)
			{
				if (line[column] != 'X')
				{
					continue;
				}
				const int32_t forward = playerRow_ - static_cast<int32_t>(row);
				const int32_t right = static_cast<int32_t>(column) - playerColumn_;
				result.push_back(playerCell + facing * forward + side * right);
			}
		}
	}

	// 5つの形 (色はここで変えられる。BreakShapeType と同じ順に並べる)
	std::vector<BreakShape> BreakShape::CreateDefaultShapes()
	{
		std::vector<BreakShape> shapes;
		shapes.emplace_back(BreakShapeType::X, Vector4{ 0.95f, 0.3f, 0.35f, 1.0f }, std::vector<std::string>{ "X.X", ".X.", "XPX" });
		shapes.emplace_back(BreakShapeType::L, Vector4{ 1.0f, 0.6f, 0.2f, 1.0f }, std::vector<std::string>{ "X..", "XXX", ".P." });
		shapes.emplace_back(BreakShapeType::Totu, Vector4{ 0.72f, 0.4f, 0.95f, 1.0f }, std::vector<std::string>{ ".X.", ".X.", "XXX", ".P." });
		shapes.emplace_back(BreakShapeType::Line, Vector4{ 0.2f, 0.9f, 0.95f, 1.0f }, std::vector<std::string>{ "X", "X", "X", "P" });
		shapes.emplace_back(BreakShapeType::Block, Vector4{ 0.98f, 0.86f, 0.2f, 1.0f }, std::vector<std::string>{ "XXX", "XXX", ".P." });
		return shapes;
	}
}
