#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <string>
#include <vector>
#include "BreakShapeType.h"
#include "../../../Stage/Grid/Cell.h"

namespace App
{
	//上の行がプレイヤーから遠く、Xが崩すマスでPがプレイヤーの床を崩す形
	class BreakShape
	{
	public:
		BreakShape() = default;
		//Pが1つある形と種類と色を決めて作る
		BreakShape(BreakShapeType type, const RyoEngine::Vector4& color, const std::vector<std::string>& rows);

		//プレイヤーのいるマスと向きから崩すマスを集める
		void CollectCells(const Cell& playerCell, const Cell& facing, std::vector<Cell>& result) const;

		BreakShapeType GetType() const { return type_; }
		const RyoEngine::Vector4& GetColor() const { return color_; }

		//4つの形
		static std::vector<BreakShape> CreateDefaultShapes();

	private:
		BreakShapeType type_ = BreakShapeType::L;
		RyoEngine::Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f }; //床に出す予告の色
		std::vector<std::string> rows_; //上の行が遠い側の形
		int32_t playerRow_ = 0; //上から数えたPの行
		int32_t playerColumn_ = 0; //左から数えたPの列
	};
}
