#include "FloorShards.h"
#include "../../Config/FloorConfig.h"
#include "../../Object/Tsumiki/Tsumiki.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//既存の積み木モデルとシェーダーでピンクの木片を作る
	void FloorShards::Initialize()
	{
		const uint32_t shader = ShaderManager::Load(DirectionConfig::kTransitionBlockShader);
		for (Shard& shard : shards_)
		{
			shard.model = Tsumiki::Create();
			shard.model->SetShader(shader);
			shard.model->SetShaderParam(3, 0.90f); //木目をうっすら残してピンクを強く出す
			shard.model->SetCastShadow(false);
			shard.life = 0.0f;
		}
	}

	//1マス分の欠片を飛ばす
	void FloorShards::Burst(const Vector3& center, float tileSize)
	{
		std::uniform_real_distribution<float> unit(0.0f, 1.0f);
		for (int32_t i = 0; i < DirectionConfig::kWoodShardsPerTile; ++i)
		{
			Shard& shard = shards_[next_];
			next_ = (next_ + 1) % shards_.size();

			//タイルの上の面から外と上へはじける
			const float offsetX = (unit(random_) - 0.5f) * tileSize * 0.8f;
			const float offsetZ = (unit(random_) - 0.5f) * tileSize * 0.8f;
			shard.position = { center.x + offsetX, center.y + FloorConfig::kTileThickness * 0.5f, center.z + offsetZ };
			const float speed = DirectionConfig::kWoodShardSpeed * (0.6f + unit(random_) * 0.8f);
			const float length = std::max(0.01f, std::sqrt(offsetX * offsetX + offsetZ * offsetZ));
			shard.velocity = { offsetX / length * speed, DirectionConfig::kWoodShardJump * (0.7f + unit(random_) * 0.6f), offsetZ / length * speed };
			shard.rotate = { unit(random_) * 360.0f, unit(random_) * 360.0f, unit(random_) * 360.0f };
			shard.spin = { (unit(random_) - 0.5f) * 1080.0f, (unit(random_) - 0.5f) * 1080.0f, (unit(random_) - 0.5f) * 1080.0f };
			const float size = tileSize * DirectionConfig::kWoodShardSize * (0.6f + unit(random_) * 0.8f);
			shard.size = { size * (1.3f + unit(random_)), size * (0.16f + unit(random_) * 0.25f), size * (0.3f + unit(random_) * 0.4f) };
			const float tint = unit(random_);
			shard.model->SetColor({ 1.0f, 0.41f + tint * 0.24f, 0.70f + tint * 0.17f, 1.0f });
			Tsumiki::Place(*shard.model, shard.position, shard.size, shard.rotate);
			shard.model->TransferMatrix();
			shard.life = DirectionConfig::kWoodShardLife;
		}
	}

	//飛ばして回してだんだん小さくして消す
	void FloorShards::Update()
	{
		const float deltaTime = GetDeltaTime();
		for (Shard& shard : shards_)
		{
			if (shard.life <= 0.0f)
			{
				continue;
			}
			shard.life -= deltaTime;
			shard.velocity.y -= DirectionConfig::kShardGravity * deltaTime;
			shard.position = shard.position + shard.velocity * deltaTime;
			shard.rotate = shard.rotate + shard.spin * deltaTime;

			//残りの時間に合わせて小さくする
			const float rate = std::clamp(shard.life / DirectionConfig::kWoodShardLife, 0.0f, 1.0f);
			Tsumiki::Place(*shard.model, shard.position, shard.size * rate, shard.rotate);
			shard.model->TransferMatrix();
		}
	}

	//飛んでいる欠片だけ描く
	void FloorShards::Draw()
	{
		for (Shard& shard : shards_)
		{
			if (shard.life > 0.0f)
			{
				shard.model->Draw();
			}
		}
	}
}
