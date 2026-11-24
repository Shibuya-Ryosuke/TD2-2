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

	//ぽよんを止める
	void PlayerBounce::Reset(const Vector3& feet)
	{
		lastFeet_ = feet;
		hasLastFeet_ = true;
		lastSpeedY_ = 0.0f;
		stretch_ = 0.0f;
		squash_ = 0.0f;
		squashTime_ = 10.0f;
		glow_ = 0.0f;
	}

	//のびとつぶれとピカッを進める
	void PlayerBounce::Update(const Vector3& feet)
	{
		const float deltaTime = GetDeltaTime();
		dust_.Update();
		landingPower_ = 0.0f;
		//初めてか止まっているか置き直したときはぽよんを止める
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
			squash_ = std::min(DirectionConfig::kPlayerLandSquash * power, DirectionConfig::kPlayerLandSquashMax);
			squashTime_ = 0.0f;
			glow_ = DirectionConfig::kPlayerLandGlow * std::min(power, 1.5f);
			stretch_ = 0.0f;
			dust_.Puff(feet, power);
			landingPower_ = power;
		}
		lastSpeedY_ = speedY;

		//跳んでいる間は速さに合わせて少しずつ縦にのびる
		const float targetStretch = std::min(std::abs(speedY) * DirectionConfig::kPlayerStretchPerSpeed, DirectionConfig::kPlayerStretchMax);
		const float follow = 1.0f - std::exp(-DirectionConfig::kPlayerStretchFollow * deltaTime);
		stretch_ += (targetStretch - stretch_) * follow;

		//着地のぽよよん
		squashTime_ += deltaTime;
		const float bounce = squash_ * std::exp(-DirectionConfig::kPlayerSquashDamping * squashTime_) * std::cos(DirectionConfig::kPlayerSquashSpeed * squashTime_);

		//縦がのびたら横は細く、つぶれたら横はふくらむ
		const float vertical = (1.0f + stretch_) * (1.0f - bounce);
		scaleY_ = std::max(vertical, 0.2f);
		scaleXZ_ = std::max(1.0f - (vertical - 1.0f) * DirectionConfig::kPlayerSquashWiden, 0.2f);

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
