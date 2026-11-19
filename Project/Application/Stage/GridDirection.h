#pragma once
#include <RyoEngine.h>
#include "Cell.h"

namespace App {
	/// <summary>
	/// 向き → 上下左右のマスの向き にする (Unity のプロトタイプの GridDirection / StepDirection と同じ)。
	/// カメラが斜めなので、入力をカメラの向きでワールドの向きに直してから、近い方の軸 (x か z) にそろえる。
	/// </summary>
	class GridDirection {
	public:
		/// <summary>ワールドの向き → 上下左右のどれか (x と z の大きい方)</summary>
		static Cell FromVector(const RyoEngine::Vector3& direction);

		/// <summary>入力 (x: 画面の右 / y: 画面の奥) → 上下左右のどれか。カメラの向きで直す</summary>
		static Cell FromInput(const RyoEngine::Vector2& input);

		/// <summary>マスの向き → 角度 (度。+Z が 0、+X の方へ回ると +)</summary>
		static float ToYawDeg(const Cell& direction);
	};
}
