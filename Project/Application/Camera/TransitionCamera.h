#pragma once
#include <RyoEngine.h>

namespace App
{
	// カメラを使っていないシーン (タイトル・クリア・ゲームオーバー) 用の、場面切り替えの演出のカメラ
	// 原点から +Z (奥) を向いた、ふつうの遠近法のカメラ
	class TransitionCamera : public RyoEngine::Camera
	{
	public:
		TransitionCamera() = default;
		~TransitionCamera() override = default;

		TransitionCamera(const TransitionCamera&) = delete;
		TransitionCamera& operator=(const TransitionCamera&) = delete;

		void Initialize() override; // 位置・向き
	};
}
