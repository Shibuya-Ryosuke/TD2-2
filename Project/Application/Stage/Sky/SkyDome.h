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
		SkyDome() = default;
		~SkyDome() = default;

		SkyDome(const SkyDome&) = delete;
		SkyDome& operator=(const SkyDome&) = delete;

		//いちばん奥に描く光の当たらない天球を作る
		void Initialize();
		//カメラの位置と向きに合わせて置き、雲をゆっくり横に流す
		void Update(const StageCamera& camera, float deltaTime = RyoEngine::GetDeltaTime());
		//タイトルなどの遠近法のカメラに合わせて置いて雲を流す
		void UpdatePerspective(const RyoEngine::Camera& camera);
		//ほかのものより先に描く
		void Draw();

	private:
		//カメラの向きと位置と画面の縦の半分の長さと縦横比に合わせて置き、雲を流す
		void Place(const RyoEngine::Vector3& rotate, const RyoEngine::Vector3& position, float halfHeight, float aspectRatio, float deltaTime);

		std::unique_ptr<RyoEngine::Model> model_{}; //天球のモデル
		float scroll_ = 0.0f; //横に流れた量
	};
}
