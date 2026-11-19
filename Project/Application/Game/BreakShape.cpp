#include "BreakShape.h"
#include <algorithm>

using namespace RyoEngine;

namespace App 
{
	//種類・色・真正面の列・形を決めて作る
	BreakShape::BreakShape(BreakShapeType type, const Vector4& color, int32_t anchorColumn, const std::vector<std::string>& rows)
		: type_(type), color_(color), anchorColumn_(anchorColumn), rows_(rows) {
	}

	//行の数 (前後の長さ)
	int32_t BreakShape::GetRowCount() const {
		return static_cast<int32_t>(rows_.size());
	}

	//列の数 (横の幅。一番長い行に合わせる)
	int32_t BreakShape::GetColumnCount() const
	{
		size_t count = 0;
		for (const std::string& row : rows_) 
		{
			count = std::max(count, row.size());
		}
		return static_cast<int32_t>(count);
	}

	//そのマスを崩すか (形の文字が X か)
	bool BreakShape::IsFilled(int32_t column, int32_t forward) const
	{
		//下の行がプレイヤーのすぐ前 (forward = 0)
		const int32_t rowIndex = GetRowCount() - 1 - forward;

		if (rowIndex < 0 || rowIndex >= GetRowCount()) 
		{
			return false;
		}
		const std::string& row = rows_[static_cast<size_t>(rowIndex)];
		if (column < 0 || column >= static_cast<int32_t>(row.size())) 
		{
			return false;
		}
		const char mark = row[static_cast<size_t>(column)];
		if (mark == '.' || mark == ' ' || mark == '_')
		{
			return false;
		}
		return true;
	}

	//プレイヤーの前に、向きに合わせて形を置いたときのマスを集める
	void BreakShape::CollectCells(const Cell& playerCell, const Cell& facing, std::vector<Cell>& result) const 
	{
		//前の結果を消す
		result.clear();

		//向きに対して右側の向き (奥を向いているときは右 = +x)
		const Cell side = { facing.z, -facing.x };
		const int32_t columnCount = GetColumnCount();

		//形の1マスずつずらした所
		for (int32_t forward = 0; forward < GetRowCount(); ++forward)
		{
			for (int32_t column = 0; column < columnCount; ++column) 
			{
				if (!IsFilled(column, forward)) 
				{
					continue;
				}

				result.push_back(playerCell + facing * (forward + 1) + side * (column - anchorColumn_));
			}
		}
	}

	//7つの形 (色はここで変えられる。BreakShapeTypeと同じ順に並べる)
	std::vector<BreakShape> BreakShape::CreateDefaultShapes() {
		std::vector<BreakShape> shapes;
		shapes.emplace_back(BreakShapeType::I, Vector4{ 0.2f, 0.9f, 0.95f, 1.0f }, 0, std::vector<std::string>{ "X", "X", "X", "X" });
		shapes.emplace_back(BreakShapeType::O, Vector4{ 0.98f, 0.86f, 0.2f, 1.0f }, 0, std::vector<std::string>{ "XX", "XX" });
		shapes.emplace_back(BreakShapeType::T, Vector4{ 0.72f, 0.4f, 0.95f, 1.0f }, 1, std::vector<std::string>{ "XXX", ".X." });
		shapes.emplace_back(BreakShapeType::S, Vector4{ 0.35f, 0.9f, 0.4f, 1.0f }, 1, std::vector<std::string>{ ".XX", "XX." });
		shapes.emplace_back(BreakShapeType::Z, Vector4{ 0.95f, 0.3f, 0.35f, 1.0f }, 1, std::vector<std::string>{ "XX.", ".XX" });
		shapes.emplace_back(BreakShapeType::L, Vector4{ 1.0f, 0.6f, 0.2f, 1.0f }, 0, std::vector<std::string>{ "X.", "X.", "XX" });
		shapes.emplace_back(BreakShapeType::J, Vector4{ 0.35f, 0.5f, 1.0f, 1.0f }, 1, std::vector<std::string>{ ".X", ".X", "XX" });
		return shapes;
	}
}
