#pragma once
#include <RyoEngine.h>
#include "PlayerDust.h"

namespace App
{
	//跳ぶとのびて着地でつぶれる見た目だけのぽよん
	class PlayerBounce
	{
	public:
		PlayerBounce() = default;
		~PlayerBounce() = default;

		PlayerBounce(const PlayerBounce&) = delete;
		PlayerBounce& operator=(const PlayerBounce&) = delete;

		//ほこりを作って体と正面の板にシェーダーをつける
		void Initialize(RyoEngine::Model& body, RyoEngine::Model& front);
		//足元の位置からのびとつぶれとピカッを進める
		void Update(const RyoEngine::Vector3& feet);
		//ぽよんを止める
		void Reset(const RyoEngine::Vector3& feet);

		float GetScaleY() const { return scaleY_; } //縦の倍率
		float GetScaleXZ() const { return scaleXZ_; } //横と奥行きの倍率
		float GetLandingPower() const { return landingPower_; } //このフレームに着地した強さ
		void DrawDust(); //ほこりを描く

	private:
		RyoEngine::Model* body_ = nullptr;
		RyoEngine::Model* front_ = nullptr;
		PlayerDust dust_;

		RyoEngine::Vector3 lastFeet_{};
		bool hasLastFeet_ = false;
		float lastSpeedY_ = 0.0f; //前のフレームの上下の速さ
		float stretch_ = 0.0f; //今の縦ののび
		float squash_ = 0.0f; //着地のつぶれの強さ
		float squashTime_ = 10.0f; //着地してからの時間
		float glow_ = 0.0f; //ピカッの強さ
		float landingPower_ = 0.0f; //このフレームに着地した強さ
		float scaleY_ = 1.0f;
		float scaleXZ_ = 1.0f;
	};
}
