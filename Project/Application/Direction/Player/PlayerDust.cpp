#include "PlayerDust.h"
#include "../../Config/GameConfig.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App
{
	//木目を消したパステルのほこりを作る
	void PlayerDust::Initialize()
	{
		const uint32_t shader = ShaderManager::Load(DirectionConfig::kTransitionBlockShader);
		size_t colorIndex = 0;
		for (Puffball& puff : puffs_)
		{
			puff.model = Model::Create(GameConfig::kCubeModel);
			puff.model->SetShader(shader);
			puff.model->SetShaderParams(0, { 0.0f, 0.0f, 0.0f, 1.0f }); //木目を消す
			puff.model->SetColor(DirectionConfig::kTransitionPopColors[colorIndex % DirectionConfig::kTransitionPopColors.size()]);
			puff.model->SetCastShadow(false);
			puff.life = 0.0f;
			++colorIndex;
		}
	}

	//足元から輪になって外へ広がるほこりを出す
	void PlayerDust::Puff(const Vector3& feet, float power)
	{
		std::uniform_real_distribution<float> unit(0.0f, 1.0f);
		const int32_t count = static_cast<int32_t>(static_cast<float>(DirectionConfig::kDustPerLanding) * std::clamp(power, 0.5f, 2.0f));
		const float twoPi = std::numbers::pi_v<float> * 2.0f;
		for (int32_t i = 0; i < count; ++i)
		{
			Puffball& puff = puffs_[next_];
			next_ = (next_ + 1) % puffs_.size();

			const float angle = (static_cast<float>(i) + unit(random_) * 0.5f) / static_cast<float>(count) * twoPi;
			const float speed = DirectionConfig::kDustSpeed * (0.7f + unit(random_) * 0.6f) * std::sqrt(power);
			puff.position = feet;
			puff.velocity = { std::cos(angle) * speed, DirectionConfig::kDustLift * (0.6f + unit(random_) * 0.8f), std::sin(angle) * speed };
			puff.spin = unit(random_) * 360.0f;
			puff.size = DirectionConfig::kDustSize * (0.7f + unit(random_) * 0.6f) * std::clamp(power, 0.8f, 1.5f);
			puff.life = DirectionConfig::kDustLife;
		}
	}

	//だんだん遅く広げて回して小さくして消す
	void PlayerDust::Update()
	{
		const float deltaTime = GetDeltaTime();
		const float drag = std::exp(-DirectionConfig::kDustDrag * deltaTime);
		for (Puffball& puff : puffs_)
		{
			if (puff.life <= 0.0f)
			{
				continue;
			}
			puff.life -= deltaTime;
			puff.velocity = puff.velocity * drag;
			puff.position = puff.position + puff.velocity * deltaTime;
			puff.spin += 360.0f * deltaTime;

			//出た瞬間にぽふっとふくらんでだんだん小さくなる
			const float rate = std::clamp(puff.life / DirectionConfig::kDustLife, 0.0f, 1.0f);
			const float grow = std::min((1.0f - rate) * 6.0f, 1.0f);
			const float scale = puff.size * rate * grow / GameConfig::kCubeModelSize;
			puff.model->SetScale({ scale, scale, scale });
			puff.model->SetRotate({ puff.spin, puff.spin * 0.7f, 0.0f });
			puff.model->SetTranslate(puff.position);
			puff.model->TransferMatrix();
		}
	}

	//出ているほこりだけ描く
	void PlayerDust::Draw()
	{
		for (Puffball& puff : puffs_)
		{
			if (puff.life > 0.0f)
			{
				puff.model->Draw();
			}
		}
	}
}
