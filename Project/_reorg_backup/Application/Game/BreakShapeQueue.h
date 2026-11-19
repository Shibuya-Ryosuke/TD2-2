#pragma once
#include <cstdint>
#include <random>
#include <vector>
#include "BreakShapeType.h"

namespace App
{

	//次に来る形の順番
	//テトリスと同じく、全部の形を1つずつ袋に入れて混ぜ、袋が空になるまで順に出す。
	//形は BreakShapeType (I O T S Z L J) で扱ってます！
	class BreakShapeQueue
	{
	public:
		BreakShapeQueue(); // 乱数を毎回ちがう値で始める

		//最初からやり直す
		// <param name="shuffle">混ぜるか (false なら I, O, T ... の順)
		void Reset(bool shuffle);

		//index 番目に来る形 (0 が今の形、1 が次、2 がその次)
		BreakShapeType Peek(int32_t index);

		//今の形を使って、次の形へ進む
		void Advance();

	private:
		BreakShapeType Draw(); // 袋から1つ取り出す

		std::vector<BreakShapeType> upcoming_; // これから来る形
		std::vector<BreakShapeType> bag_;      // 袋の中身

		bool shuffle_ = true;
		std::mt19937 random_;
	};
}
