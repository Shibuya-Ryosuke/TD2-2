#include "TransitionCamera.h"

using namespace RyoEngine;

namespace App
{
	//原点から奥を向く
	void TransitionCamera::Initialize()
	{
		Camera::Initialize();
		rotate_ = { 0.0f, 0.0f, 0.0f };
		translate_ = { 0.0f, 0.0f, 0.0f };
		Update();
	}
}
