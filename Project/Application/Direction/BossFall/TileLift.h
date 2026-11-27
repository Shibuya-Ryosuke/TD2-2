#pragma once
#include <RyoEngine.h>
#include <vector>
#include "../../Stage/Grid/Cell.h"

namespace App
{
	class Floor;

	//ハンマーのドンを合図に、上の層に残った足場がふわっと浮いてピンクに光る
	class TileLift
	{
	public:
		//upperの残った足場を浮かせ始める
		void Begin(Floor& upper);
		//浮きと光を進めて足場の位置と光を入れる
		void Update(float deltaTime, Floor& upper);

	private:
		std::vector<Cell> cells_; //浮かせる足場のマス
		float timer_ = 0.0f; //浮き始めてからの秒数
	};
}
