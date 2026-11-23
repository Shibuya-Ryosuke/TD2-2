#pragma once
#include <cstdint>
#include <random>
#include <vector>
#include "BreakShapeType.h"

namespace App
{

	//全部の形を袋に入れて混ぜて順に出す、次に来る形の順番
	class BreakShapeQueue
	{
	public:
		BreakShapeQueue(); //乱数を毎回ちがう値で始める

		//最初からやり直す
		void Reset(bool shuffle);

		//0が今の形でindex番目に来る形
		BreakShapeType Peek(int32_t index);

		//今の形を使って次の形へ進む
		void Advance();

	private:
		BreakShapeType Draw(); //袋から1つ取り出す

		std::vector<BreakShapeType> upcoming_; //これから来る形
		std::vector<BreakShapeType> bag_; //袋の中身

		bool shuffle_ = true;
		std::mt19937 random_;
	};
}
