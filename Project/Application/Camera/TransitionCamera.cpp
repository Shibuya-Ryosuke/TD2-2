#include "TransitionCamera.h"

using namespace RyoEngine;

namespace App
{
	// 原点から +Z を向く (画角・縦横比はエンジンの初期値)
	void TransitionCamera::Initialize()
	{
		Camera::Initialize();
		rotate_ = { 0.0f, 0.0f, 0.0f };
		translate_ = { 0.0f, 0.0f, 0.0f };
		Update();
	}
}
