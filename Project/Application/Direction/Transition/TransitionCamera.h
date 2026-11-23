#pragma once
#include <RyoEngine.h>

namespace App
{
	//カメラのないシーンで場面切り替えに使う遠近法のカメラ
	class TransitionCamera : public RyoEngine::Camera
	{
	public:
		TransitionCamera() = default;
		~TransitionCamera() override = default;

		TransitionCamera(const TransitionCamera&) = delete;
		TransitionCamera& operator=(const TransitionCamera&) = delete;

		void Initialize() override; //原点から奥を向く
	};
}
