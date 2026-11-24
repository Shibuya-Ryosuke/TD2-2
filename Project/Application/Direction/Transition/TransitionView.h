#pragma once
#include <RyoEngine.h>

namespace App
{
	//カメラから見た位置でブロックを動かす空間で、平行投影でも遠近法のように見せる
	struct TransitionView
	{
		bool isOrthographic = false; //平行投影のカメラか
		float refDepth = 10.0f; //カメラから壁までの距離
		float halfHeight = 1.0f; //壁の所で画面の縦の半分に映る長さ
		float aspectRatio = 1.0f; //画面の横と縦の比

		//壁の所で画面の横の半分に映る長さ
		float GetHalfWidth() const { return halfHeight * aspectRatio; }

		//カメラから見た位置を親から見た位置にする
		RyoEngine::Vector3 ToLocalPosition(const RyoEngine::Vector3& position) const;
		//その奥行きで大きさを何倍にするか
		float GetScaleAt(float depth) const;
	};
}
