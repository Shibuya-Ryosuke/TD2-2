#pragma once
#include <RyoEngine.h>
#include "HeartBurst.h"

namespace App
{
	//着地で体を光らせて打撃でハートを描く
	class PlayerBounce
	{
	public:
		PlayerBounce() = default;
		~PlayerBounce() = default;

		PlayerBounce(const PlayerBounce&) = delete;
		PlayerBounce& operator=(const PlayerBounce&) = delete;

		//ハートを作って体にシェーダーをつける
		void Initialize(RyoEngine::Model& body);
		//足元の位置から着地を見つけて体を光らせる
		void Update(const RyoEngine::Vector3& feet);
		//好きな所からハートをはじけさせる
		void Burst(const RyoEngine::Vector3& feet, float power, float sizeScale = 1.0f) { hearts_.Burst(feet, power, sizeScale); }
		//ポーズが決まったときに肩の外側へハートを飛ばす
		void BurstVictory(const RyoEngine::Vector3& origin, const RyoEngine::Vector3& outward, const RyoEngine::Vector3& up) { hearts_.BurstVictory(origin, outward, up); }
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
