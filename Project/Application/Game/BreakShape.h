#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <string>
#include <vector>
#include "BreakShapeType.h"
#include "../Stage/Cell.h"

namespace App 
{
	//床を崩す形 (テトリスのブロックのような形)
	//rowsは上の行が「プレイヤーから遠い」側。下の行がプレイヤーのすぐ前のマスになる。
	//'X'が崩すマス、'.' が崩さないマス。
	//anchorColumn: プレイヤーの真正面に来る列 (左から 0)
	class BreakShape 
	{
	public:
		BreakShape() = default;
		BreakShape(BreakShapeType type, const RyoEngine::Vector4& color, int32_t anchorColumn, const std::vector<std::string>& rows); // 種類・色・真正面の列・形 を決めて作る

	
		// プレイヤーのいるマスと向きから、崩すマスを集める
	
		//<param name="playerCell">プレイヤーのいるマス
		//<param name="facing">プレイヤーの向き (上下左右のどれか1マス分)
		//<param name="result">崩すマス (中は最初に空にする)
		void CollectCells(const Cell& playerCell, const Cell& facing, std::vector<Cell>& result) const;

		BreakShapeType GetType() const { return type_; }
		const RyoEngine::Vector4& GetColor() const { return color_; }

		//プロトタイプの7つの形 (I O T S Z L J)
		static std::vector<BreakShape> CreateDefaultShapes();

	private:

		int32_t GetRowCount() const; // 行の数 
		int32_t GetColumnCount() const; // 列の数 
		//>column 列目・プレイヤーから forward マス目 (0 がすぐ前) を崩すか
		bool IsFilled(int32_t column, int32_t forward) const;

		BreakShapeType type_ = BreakShapeType::I;

		RyoEngine::Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f }; //床に出す予告の色
		int32_t anchorColumn_ = 0;
		std::vector<std::string> rows_;
	};
}
