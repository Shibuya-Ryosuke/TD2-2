#pragma once
#include <RyoEngine.h>
#include "Cell.h"

namespace App
{
	//入力や向きをカメラに合わせて上下左右のマスの向きにそろえる
	class GridDirection
	{
	public:
		//ワールドの向きをxとzの大きい方で上下左右にする
		static Cell FromVector(const RyoEngine::Vector3& direction);

		//画面の入力をカメラの向きで直して上下左右にする
		static Cell FromInput(const RyoEngine::Vector2& input);

		//画面の入力をカメラの向きで直した斜めもあるワールドの向き
		static RyoEngine::Vector3 ToWorld(const RyoEngine::Vector2& input);

		//+Zが0のマスの向きの度の角度
		static float ToYawDeg(const Cell& direction);

		//+Zが0のワールドの向きの度の角度
		static float ToYawDeg(const RyoEngine::Vector3& direction);
	};
}
