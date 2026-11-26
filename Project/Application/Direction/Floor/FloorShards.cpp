#include "FloorShards.h"
#include "../../Config/FloorConfig.h"
#include "../../Config/ModelConfig.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//木目を消したピンクの欠片を作る
	void FloorShards::Initialize()
	{
		const uint32_t shader = ShaderManager::Load(DirectionConfig::kTransitionBlockShader);
		for (Shard& shard : shards_)
		{
			shard.model = Model::Create(ModelConfig::kCubeModel);
			shard.model->SetShader(shader);
			shard.model->SetShaderParams(0, { 0.0f, 0.0f, 0.0f, 1.0f }); //木目を消す
			shard.model->SetColor(DirectionConfig::kShardColor);
			shard.model->SetEmissive({ DirectionConfig::kHeartColor.x, DirectionConfig::kHeartColor.y, DirectionConfig::kHeartColor.z }, DirectionConfig::kShardGlow);
			shard.model->SetCastShadow(false);
			shard.life = 0.0f;
		}
	}

	//1マス分の欠片を飛ばす
	void FloorShards::Burst(const Vector3& center, float tileSize)
	{
		std::uniform_real_distribution<float> unit(0.0f, 1.0f);
		for (int32_t i = 0; i < DirectionConfig::kShardsPerTile; ++i)
		{
			Shard& shard = shards_[next_];
			next_ = (next_ + 1) % shards_.size();

			//タイルの上の面から外と上へはじける
			const float offsetX = (unit(random_) - 0.5f) * tileSize * 0.8f;
			const float offsetZ = (unit(random_) - 0.5f) * tileSize * 0.8f;
			shard.position = { center.x + offsetX, center.y + FloorConfig::kTileThickness * 0.5f, center.z + offsetZ };
			const float speed = DirectionConfig::kShardSpeed * (0.6f + unit(random_) * 0.8f);
			const float length = std::max(0.01f, std::sqrt(offsetX * offsetX + offsetZ * offsetZ));
			shard.velocity = { offsetX / length * speed, DirectionConfig::kShardJump * (0.7f + unit(random_) * 0.6f), offsetZ / length * speed };
			shard.rotate = { unit(random_) * 360.0f, unit(random_) * 360.0f, unit(random_) * 360.0f };
			shard.spin = { (unit(random_) - 0.5f) * 1080.0f, (unit(random_) - 0.5f) * 1080.0f, (unit(random_) - 0.5f) * 1080.0f };
			shard.size = tileSize * DirectionConfig::kShardSize * (0.6f + unit(random_) * 0.8f);
			shard.life = DirectionConfig::kShardLife;
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
			const float rate = std::clamp(shard.life / DirectionConfig::kShardLife, 0.0f, 1.0f);
			const float scale = shard.size * rate / ModelConfig::kCubeModelSize;
			shard.model->SetScale({ scale, scale, scale });
			shard.model->SetRotate(shard.rotate);
			shard.model->SetTranslate(shard.position);
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
