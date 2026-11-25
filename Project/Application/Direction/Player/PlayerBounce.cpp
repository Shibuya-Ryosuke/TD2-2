#include "PlayerBounce.h"
#include "../../Config/DirectionConfig.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//ほこりを作って体にシェーダーをつける
	void PlayerBounce::Initialize(Model& body)
	{
		body_ = &body;
		const uint32_t shader = ShaderManager::Load(DirectionConfig::kPlayerPopShader);
		body_->SetShader(shader);
		dust_.Initialize();
	}

	//着地の見つけ方を今の位置からやり直す
	void PlayerBounce::Reset(const Vector3& feet)
	{
		lastFeet_ = feet;
		hasLastFeet_ = true;
		lastSpeedY_ = 0.0f;
		glow_ = 0.0f;
	}

	//足元の位置から着地を見つけてピカッとほこりを出す
	void PlayerBounce::Update(const Vector3& feet)
	{
		const float deltaTime = GetDeltaTime();
		dust_.Update();
		landingPower_ = 0.0f;
		//初めてか止まっているか置き直したときはやり直す
		const Vector3 moved = feet - lastFeet_;
		const float movedLength = std::sqrt(moved.x * moved.x + moved.y * moved.y + moved.z * moved.z);
		if (!hasLastFeet_ || deltaTime <= 0.0f || movedLength > DirectionConfig::kPlayerTeleportDistance)
		{
			Reset(feet);
		}

		//前のフレームの位置から上下の速さを出す
		const float speedY = (feet.y - lastFeet_.y) / std::max(deltaTime, 0.0001f);
		lastFeet_ = feet;

		//速く落ちていて急に止まったか上へ跳んだら着地
		const bool wasFalling = lastSpeedY_ < -DirectionConfig::kPlayerLandMinSpeed;
		const bool isStopped = std::abs(speedY) < DirectionConfig::kPlayerStopSpeed;
		if (wasFalling && (isStopped || speedY > 0.0f))
		{
			const float power = -lastSpeedY_ / DirectionConfig::kPlayerLandSpeedRef; //1でふつうの着地
			glow_ = DirectionConfig::kPlayerLandGlow * std::min(power, 1.5f);
			dust_.Puff(feet, power);
			landingPower_ = power;
		}
		lastSpeedY_ = speedY;

		//ピカッをだんだん消す
		glow_ *= std::exp(-DirectionConfig::kPlayerLandGlowDecay * deltaTime);
		body_->SetShaderParams(0, { glow_, 0.0f, 0.0f, 0.0f });
	}

	//ほこりを描く
	void PlayerBounce::DrawDust()
	{
		dust_.Draw();
	}
}
