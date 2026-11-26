#pragma once
#include <cstdint>
#include <vector>
#include "../../Stage/Grid/Cell.h"

namespace App
{
	class Floor;

	//チュートリアルのcsvの数字を読んで、最初の位置や積み木のマスを教える
	class TutorialMap
	{
	public:
		//csvを読んで、読めなかったらfalse
		bool Load();

		//その数字のマスを全部集める
		std::vector<Cell> Find(int32_t code) const;
		//その数字の最初のマスで、なければ床の真ん中
		Cell FindFirst(int32_t code) const;
		//そのマスの数字で、範囲の外は0
		int32_t GetCode(const Cell& cell) const;

		//島と積み木と島をつなぐマスと、そのまわりで穴になっている所を床に戻す
		void RestoreIsland(Floor& floor) const;
		//穴になったマスを全部ぽこっと床に戻す
		void RestoreAll(Floor& floor) const;

	private:
		int32_t width_ = 0;
		int32_t depth_ = 0;
		std::vector<int32_t> codes_; //手前の行から順に左から右へ
	};
}
