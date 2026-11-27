#include "HeartBurst.h"
#include "../../Config/ModelConfig.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App
{
	//カメラの方を向けてハートを描く薄い箱を色ちがいで作る
	void HeartBurst::Initialize()
	{
		const uint32_t shader = ShaderManager::Load(DirectionConfig::kHeartPopShader);
		for (Heart& heart : hearts_)
		{
			heart.model = Model::Create(ModelConfig::kCubeModel);
			heart.model->SetShader(shader);
			heart.model->SetCastShadow(false);
			heart.life = 0.0f;
			//一度も行列を確定していないモデルは原点に元の大きさで出てしまうので、最初から大きさ0にしておく
			heart.model->SetScale({ 0.0f, 0.0f, 0.0f });
			heart.model->TransferMatrix();
		}
	}

	//足元の輪からハートをはじけさせて、ほとんど真上へ飛ばす
	void HeartBurst::Burst(const Vector3& feet, float power, float sizeScale)
	{
		std::uniform_real_distribution<float> unit(0.0f, 1.0f);
		const float strength = std::clamp(power, 0.4f, DirectionConfig::kHeartMaxPower);
		const int32_t count = static_cast<int32_t>(static_cast<float>(DirectionConfig::kHeartPerBurst) * strength);
		const float twoPi = std::numbers::pi_v<float> * 2.0f;
		const Vector3 cameraRotate = GetCameraRotateDeg();
		for (int32_t i = 0; i < count; ++i)
		{
			const size_t index = next_;
			Heart& heart = hearts_[index];
			next_ = (next_ + 1) % hearts_.size();
			heart.model->SetColor(DirectionConfig::kHeartColors[index % DirectionConfig::kHeartColors.size()]);
			heart.model->SetShaderParam(2, static_cast<float>(index % 7) / 7.0f);

			//足元のまわりから外へ少し、上へいきおいよく
			const float angle = (static_cast<float>(i) + unit(random_) * 0.7f) / static_cast<float>(std::max(count, 1)) * twoPi;
			const float ring = DirectionConfig::kHeartRingRadius * (0.5f + unit(random_) * 0.7f);
			const float outward = DirectionConfig::kHeartOutSpeed * (0.5f + unit(random_)) * std::sqrt(strength);
			const float upward = DirectionConfig::kHeartUpSpeed * (0.6f + unit(random_) * 0.8f) * std::sqrt(strength);
			heart.position = { feet.x + std::cos(angle) * ring, feet.y + 0.05f, feet.z + std::sin(angle) * ring };
			heart.velocity = { std::cos(angle) * outward, upward, std::sin(angle) * outward };
			heart.roll = (unit(random_) - 0.5f) * DirectionConfig::kHeartTilt;
			heart.rollSpeed = (unit(random_) - 0.5f) * DirectionConfig::kHeartTiltSpeed;
			heart.sway = unit(random_) * twoPi;
			heart.size = DirectionConfig::kHeartSize * sizeScale * (0.6f + unit(random_) * 0.8f);
			heart.maxLife = DirectionConfig::kHeartLife * (0.7f + unit(random_) * 0.6f);
			heart.life = heart.maxLife;
			//出したフレームに描いても原点に出ないように、すぐ今の位置で行列を確定する
			Place(heart, cameraRotate);
		}
	}

	//肩の外で高さをずらしたハートを顔から離れる方向に弾けさせる
	void HeartBurst::BurstVictory(const Vector3& origin, const Vector3& outward, const Vector3& up)
	{
		using namespace DirectionConfig;
		std::uniform_real_distribution<float> unit(0.0f, 1.0f);
		const Vector3 cameraRotate = GetCameraRotateDeg();
		//縦にならべたまん中が肩の高さに来るようにずらす
		const float middle = static_cast<float>(kVictoryHeartsPerSide - 1) * 0.5f;
		for (int32_t i = 0; i < kVictoryHeartsPerSide; ++i)
		{
			Heart& heart = hearts_[next_];
			next_ = (next_ + 1) % hearts_.size();

			const float height = (static_cast<float>(i) - middle) * kVictoryHeartSpacing;
			heart.position = origin + outward * (kVictoryHeartOutMin + unit(random_) * kVictoryHeartOutRange) + up * height;
			heart.velocity = outward * (kVictoryHeartSpeedMin + unit(random_) * kVictoryHeartSpeedRange) + up * (kVictoryHeartRiseMin + unit(random_) * kVictoryHeartRiseRange);
			heart.roll = (unit(random_) - 0.5f) * kHeartTilt;
			heart.rollSpeed = (unit(random_) - 0.5f) * kHeartTiltSpeed;
			heart.sway = unit(random_) * (2.0f * std::numbers::pi_v<float>);
			heart.size = (kVictoryHeartSizeMin + unit(random_) * kVictoryHeartSizeRange) * kVictoryParticleScale;
			heart.maxLife = kVictoryHeartLifeMin + unit(random_) * kVictoryHeartLifeRange;
			heart.life = heart.maxLife - kHeartBurstPopTime * kVictoryHeartPopSkip;
			const float pale = unit(random_);
			heart.model->SetColor(kVictoryHeartPale + kVictoryHeartPaleRange * pale);
			heart.model->SetShaderParam(2, unit(random_));
			Place(heart, cameraRotate);
		}
	}

	//ふわっと減速しながら左右にゆれて舞い上がり、最後にきゅっと小さくなる
	void HeartBurst::Update()
	{
		const float deltaTime = GetDeltaTime();
		const float drag = std::exp(-DirectionConfig::kHeartDrag * deltaTime);
		const Vector3 cameraRotate = GetCameraRotateDeg();
		for (Heart& heart : hearts_)
		{
			if (heart.life <= 0.0f)
			{
				continue;
			}
			heart.life -= deltaTime;
			heart.velocity = heart.velocity * drag;
			heart.velocity.y += DirectionConfig::kHeartBuoyancy * deltaTime;
			heart.sway += deltaTime * DirectionConfig::kHeartSwaySpeed;
			//形がわかるように傾きすぎない
			heart.roll = std::clamp(heart.roll + heart.rollSpeed * deltaTime, -DirectionConfig::kHeartTilt, DirectionConfig::kHeartTilt);
			heart.position = heart.position + heart.velocity * deltaTime;
			Place(heart, cameraRotate);
		}
	}

	//今のカメラの回転を度で
	Vector3 HeartBurst::GetCameraRotateDeg()
	{
		const Vector3 rotate = CameraManager::GetGameCamera().GetRotate();
		const float radToDeg = 180.0f / std::numbers::pi_v<float>;
		return { rotate.x * radToDeg, rotate.y * radToDeg, rotate.z * radToDeg };
	}

	//ぽんっと大きくなりすぎてから戻り、消える前にきゅっと縮む大きさで、前の面をカメラへ向けて行列を確定する
	void HeartBurst::Place(Heart& heart, const Vector3& cameraRotateDeg)
	{
		Vector3 position = heart.position;
		position.x += std::sin(heart.sway) * DirectionConfig::kHeartSwayWidth;
		const float age = heart.maxLife - heart.life;
		const float pop = std::min(age / DirectionConfig::kHeartBurstPopTime, 1.0f);
		const float overshoot = 1.0f + std::sin(pop * std::numbers::pi_v<float>) * 0.35f;
		const float fade = std::clamp(heart.life / (heart.maxLife * 0.3f), 0.0f, 1.0f);
		const float scale = heart.size * pop * overshoot * fade / ModelConfig::kCubeModelSize;
		heart.model->SetScale({ scale, scale, scale * 0.05f });
		heart.model->SetRotate(cameraRotateDeg);
		heart.model->SetTranslate(position);
		heart.model->SetShaderParam(0, DirectionConfig::kHeartGlow * (0.6f + 0.4f * fade));
		heart.model->SetShaderParam(1, heart.roll);
		heart.model->TransferMatrix();
	}

	//出ているハートだけ描く
	void HeartBurst::Draw()
	{
		for (Heart& heart : hearts_)
		{
			if (heart.life > 0.0f)
			{
				heart.model->Draw();
			}
		}
	}
}
