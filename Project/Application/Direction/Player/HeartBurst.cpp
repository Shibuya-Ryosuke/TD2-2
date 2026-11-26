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
		size_t colorIndex = 0;
		for (Heart& heart : hearts_)
		{
			heart.model = Model::Create(ModelConfig::kCubeModel);
			heart.model->SetShader(shader);
			heart.model->SetColor(DirectionConfig::kHeartColors[colorIndex % DirectionConfig::kHeartColors.size()]);
			heart.model->SetShaderParam(2, static_cast<float>(colorIndex % 7) / 7.0f); //キラッのタイミングをずらす
			heart.model->SetCastShadow(false);
			heart.life = 0.0f;
			++colorIndex;
		}
	}

	//足元の輪からハートをはじけさせて、ほとんど真上へ飛ばす
	void HeartBurst::Burst(const Vector3& feet, float power)
	{
		std::uniform_real_distribution<float> unit(0.0f, 1.0f);
		const float strength = std::clamp(power, 0.4f, DirectionConfig::kHeartMaxPower);
		const int32_t count = static_cast<int32_t>(static_cast<float>(DirectionConfig::kHeartPerBurst) * strength);
		const float twoPi = std::numbers::pi_v<float> * 2.0f;
		for (int32_t i = 0; i < count; ++i)
		{
			Heart& heart = hearts_[next_];
			next_ = (next_ + 1) % hearts_.size();

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
			heart.size = DirectionConfig::kHeartSize * (0.6f + unit(random_) * 0.8f);
			heart.maxLife = DirectionConfig::kHeartLife * (0.7f + unit(random_) * 0.6f);
			heart.life = heart.maxLife;
		}
	}

	//ふわっと減速しながら左右にゆれて舞い上がり、最後にきゅっと小さくなる
	void HeartBurst::Update()
	{
		const float deltaTime = GetDeltaTime();
		const float drag = std::exp(-DirectionConfig::kHeartDrag * deltaTime);
		const Vector3 cameraRotate = CameraManager::GetGameCamera().GetRotate();
		const float radToDeg = 180.0f / std::numbers::pi_v<float>;
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
			Vector3 position = heart.position + heart.velocity * deltaTime;
			heart.position = position;
			position.x += std::sin(heart.sway) * DirectionConfig::kHeartSwayWidth;

			//出た瞬間にぽんっと大きくなりすぎてから戻り、消える前にきゅっと縮む
			const float age = heart.maxLife - heart.life;
			const float pop = std::min(age / DirectionConfig::kHeartBurstPopTime, 1.0f);
			const float overshoot = 1.0f + std::sin(pop * std::numbers::pi_v<float>) * 0.35f;
			const float fade = std::clamp(heart.life / (heart.maxLife * 0.3f), 0.0f, 1.0f);
			const float scale = heart.size * pop * overshoot * fade / ModelConfig::kCubeModelSize;

			//前の面がカメラを向くように、カメラと同じ向きにする
			heart.model->SetScale({ scale, scale, scale * 0.05f });
			heart.model->SetRotate({ cameraRotate.x * radToDeg, cameraRotate.y * radToDeg, cameraRotate.z * radToDeg });
			heart.model->SetTranslate(position);
			heart.model->SetShaderParam(0, DirectionConfig::kHeartGlow * (0.6f + 0.4f * fade));
			heart.model->SetShaderParam(1, heart.roll);
			heart.model->TransferMatrix();
		}
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
