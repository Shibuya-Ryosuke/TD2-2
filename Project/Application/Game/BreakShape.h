#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <string>
#include <vector>
#include "BreakShapeType.h"
#include "../Stage/Cell.h"

namespace App
{
	// 床を崩す形
	// rows は上の行が「プレイヤーから遠い」側。プレイヤーが前 (上) を向いたときの見た目のまま書く
	// 'X' が崩すマス、'P' がプレイヤーのいるマス (崩さない)、'.' が崩さないマス
	// 例: L字 { "X..", "XXX", ".P." }
	class BreakShape
	{
	public:
		BreakShape() = default;
		// 種類・色・形 を決めて作る (rows に 'P' が1つ必要)
		BreakShape(BreakShapeType type, const RyoEngine::Vector4& color, const std::vector<std::string>& rows);

		// プレイヤーのいるマスと向きから、崩すマスを集める
		// playerCell: プレイヤーのいるマス
		// facing: プレイヤーの向き (上下左右のどれか1マス分)
		// result: 崩すマス (中は最初に空にする)
		void CollectCells(const Cell& playerCell, const Cell& facing, std::vector<Cell>& result) const;

		BreakShapeType GetType() const { return type_; }
		const RyoEngine::Vector4& GetColor() const { return color_; }

		// 5つの形 (X L Totu Line Block)
		static std::vector<BreakShape> CreateDefaultShapes();

	private:
		BreakShapeType type_ = BreakShapeType::X;
		RyoEngine::Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f }; // 床に出す予告の色
		std::vector<std::string> rows_;                        // 形 (上の行が遠い側)
		int32_t playerRow_ = 0;                                // 'P' の行 (上から 0)
		int32_t playerColumn_ = 0;                             // 'P' の列 (左から 0)
	};
}
