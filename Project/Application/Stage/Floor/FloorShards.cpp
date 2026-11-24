#include "FloorShards.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	// モデルを作る (木目を消したピンク。TransitionBlock シェーダーの「単色」を使う)
	void FloorShards::Initialize()
	{
		const uint32_t shader = ShaderManager::Load(GameConfig::kTransitionBlockShader);
		for (Shard& shard : shards_)
		{
			shard.model = Model::Create(GameConfig::kCubeModel);
			shard.model->SetShader(shader);
			shard.model->SetShaderParams(0, { 0.0f, 0.0f, 0.0f, 1.0f }); // 3番 = 1: 木目を消す
			shard.model->SetColor(GameConfig::kShardColor);
			shard.model->SetEmissive({ GameConfig::kHeartColor.x, GameConfig::kHeartColor.y, GameConfig::kHeartColor.z }, GameConfig::kShardGlow);
			shard.model->SetCastShadow(false);
			shard.life = 0.0f;
		}
	}

	// 1マス分の欠片を飛ばす
	void FloorShards::Burst(const Vector3& center, float tileSize)
	{
		std::uniform_real_distribution<float> unit(0.0f, 1.0f);
		for (int32_t i = 0; i < GameConfig::kShardsPerTile; ++i)
		{
			Shard& shard = shards_[next_];
			next_ = (next_ + 1) % shards_.size();

			// タイルの上の面のどこかから、外へ・上へはじける
			const float offsetX = (unit(random_) - 0.5f) * tileSize * 0.8f;
			const float offsetZ = (unit(random_) - 0.5f) * tileSize * 0.8f;
			shard.position = { center.x + offsetX, center.y + GameConfig::kTileThickness * 0.5f, center.z + offsetZ };
			const float speed = GameConfig::kShardSpeed * (0.6f + unit(random_) * 0.8f);
			const float length = std::max(0.01f, std::sqrt(offsetX * offsetX + offsetZ * offsetZ));
			shard.velocity = { offsetX / length * speed, GameConfig::kShardJump * (0.7f + unit(random_) * 0.6f), offsetZ / length * speed };
			shard.rotate = { unit(random_) * 360.0f, unit(random_) * 360.0f, unit(random_) * 360.0f };
			shard.spin = { (unit(random_) - 0.5f) * 1080.0f, (unit(random_) - 0.5f) * 1080.0f, (unit(random_) - 0.5f) * 1080.0f };
			shard.size = tileSize * GameConfig::kShardSize * (0.6f + unit(random_) * 0.8f);
			shard.life = GameConfig::kShardLife;
		}
	}

	// 飛ばす・回す・だんだん小さくして消す
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
			shard.velocity.y -= GameConfig::kShardGravity * deltaTime;
			shard.position = shard.position + shard.velocity * deltaTime;
			shard.rotate = shard.rotate + shard.spin * deltaTime;

			// 残りの時間に合わせて小さくなる (cube.obj は1辺 2 なので半分にする)
			const float rate = std::clamp(shard.life / GameConfig::kShardLife, 0.0f, 1.0f);
			const float scale = shard.size * rate / GameConfig::kCubeModelSize;
			shard.model->SetScale({ scale, scale, scale });
			shard.model->SetRotate(shard.rotate);
			shard.model->SetTranslate(shard.position);
			shard.model->TransferMatrix();
		}
	}

	// 飛んでいる欠片だけ描く
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
