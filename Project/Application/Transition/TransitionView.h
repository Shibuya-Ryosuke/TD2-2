#pragma once
#include <RyoEngine.h>

namespace App
{
	// 場面切り替えの演出で使う「カメラから見た空間」
	// ・演出のブロックは、カメラから見た位置 (x: 右 / y: 上 / z: 奥への距離) で動かす
	// ・z = refDepth の所に、画面ぴったりの大きさ (縦の半分 = halfHeight) の壁ができる
	// ・遠近法のカメラはそのまま。平行投影のカメラ (ゲームシーン) は遠くても小さくならないので、
	//   近づくほど大きく・真ん中から外へ広がるように自分で計算する (遠近法と同じ見た目にする)
	struct TransitionView
	{
		bool isOrthographic = false; // 平行投影のカメラか
		float refDepth = 10.0f;      // 壁を置く距離 (カメラから見た奥行き)
		float halfHeight = 1.0f;     // refDepth の所で、画面の縦の半分に映る長さ
		float aspectRatio = 1.0f;    // 画面の 横 / 縦

		// 画面の横の半分に映る長さ (refDepth の所で)
		float GetHalfWidth() const { return halfHeight * aspectRatio; }

		// カメラから見た位置 → 親 (カメラの位置・向き) から見た位置
		RyoEngine::Vector3 ToLocalPosition(const RyoEngine::Vector3& position) const;
		// その奥行きで、大きさを何倍にするか (平行投影のときだけ。遠近法は 1)
		float GetScaleAt(float depth) const;
	};
}
