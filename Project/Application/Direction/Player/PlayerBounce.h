#pragma once
#include <RyoEngine.h>
#include "HeartBurst.h"

namespace App
{
	//着地でハートがはじけてピカッと光る見た目だけの動き
	class PlayerBounce
	{
	public:
		PlayerBounce() = default;
		~PlayerBounce() = default;

		PlayerBounce(const PlayerBounce&) = delete;
		PlayerBounce& operator=(const PlayerBounce&) = delete;

		//ハートを作って体にシェーダーをつける
		void Initialize(RyoEngine::Model& body);
		//足元の位置から着地を見つけてハートを出し、ピカッと光る
		void Update(const RyoEngine::Vector3& feet);
		//好きな所からハートをはじけさせる
		void Burst(const RyoEngine::Vector3& feet, float power) { hearts_.Burst(feet, power); }
		//着地の見つけ方を今の位置からやり直す
		void Reset(const RyoEngine::Vector3& feet);

		float GetLandingPower() const { return landingPower_; } //このフレームに着地した強さ
		void DrawHearts(); //ハートを描く

	private:
		RyoEngine::Model* body_ = nullptr;
		HeartBurst hearts_;

		RyoEngine::Vector3 lastFeet_{};
		bool hasLastFeet_ = false;
		float lastSpeedY_ = 0.0f; //前のフレームの上下の速さ
		float glow_ = 0.0f; //ピカッの強さ
		float landingPower_ = 0.0f; //このフレームに着地した強さ
	};
}
