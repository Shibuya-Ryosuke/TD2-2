#pragma once
#include <RyoEngine.h>
#include <memory>

namespace App
{
	class StageCamera;

	//カメラについてきて画面の上がピンク、下が水色に見えるゲームシーンの背景の天球
	class SkyDome
	{
	public:
		//いちばん奥に描く光の当たらない天球を作る
		void Initialize();
		//カメラの位置と向きに合わせて置き、雲をゆっくり横に流す
		void Update(const StageCamera& camera);
		//ほかのものより先に描く
		void Draw();

	private:
		std::unique_ptr<RyoEngine::Model> model_{}; //天球のモデル
		float scroll_ = 0.0f; //横に流れた量
	};
}
