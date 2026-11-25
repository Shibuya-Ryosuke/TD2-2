#pragma once
#include <RyoEngine.h>
#include <memory>

namespace App
{
	class StageCamera;

	//カメラについてきて遠近感のあるカメラで見上げた空を描くゲームシーンの背景の天球
	class SkyDome
	{
	public:
		//いちばん奥に描く光の当たらない天球を作る
		void Initialize();
		//カメラの位置と向きに合わせて画面を覆い、天球をゆっくり回す
		void Update(const StageCamera& camera);
		//ほかのものより先に描く
		void Draw();

	private:
		std::unique_ptr<RyoEngine::Model> model_{}; //天球のモデル
		float turnDeg_ = 0.0f; //天球が回った度数
	};
}
