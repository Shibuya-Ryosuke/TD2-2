#pragma once
#include <RyoEngine.h>

namespace App
{
	//ダメージや着地でカメラを細かくゆらして、だんだん収める
	class ScreenShake
	{
	public:
		//強さpowerで揺らし始め、揺れている途中なら強い方を残す
		void Play(float power);
		//揺れを進めて、カメラをずらす分を返す
		RyoEngine::Vector3 Update(float deltaTime);

	private:
		float power_ = 0.0f; //今の揺れの強さ
		float time_ = 0.0f; //揺れの模様に使う時間
	};
}
