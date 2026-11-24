#pragma once
#include <RyoEngine.h>
#include <array>
#include <cstdint>
#include <memory>
#include <random>
#include "../../Config/GameConfig.h"

namespace App
{
	// 床がガラスみたいに砕けたときに飛び散る、ピンクの欠片 (小さい cube)
	// 決まった数のモデルを使い回す (足りなくなったら、いちばん古い欠片から使う)
	class FloorShards
	{
	public:
		FloorShards() = default;
		~FloorShards() = default;

		FloorShards(const FloorShards&) = delete;
		FloorShards& operator=(const FloorShards&) = delete;

		void Initialize(); // モデルを作る

		// 1マス分の欠片を飛ばす (center: タイルの真ん中 / tileSize: タイルの幅)
		void Burst(const RyoEngine::Vector3& center, float tileSize);

		void Update(); // 飛ばす・小さくする
		void Draw();

	private:
		// 欠片1個
		struct Shard
		{
			// unique_ptr を持つので、コピーはできない (警告 C4625 / C4626 が出ないように、はっきり書く)
			Shard() = default;
			~Shard() = default;
			Shard(const Shard&) = delete;
			Shard& operator=(const Shard&) = delete;

			std::unique_ptr<RyoEngine::Model> model;
			RyoEngine::Vector3 position{};
			RyoEngine::Vector3 velocity{};
			RyoEngine::Vector3 rotate{};
			RyoEngine::Vector3 spin{};      // 1秒に回る角度 (度)
			float size = 0.0f;
			float life = 0.0f;              // 残りの時間 (0 以下 = 使っていない)
		};

		std::array<Shard, GameConfig::kShardPoolSize> shards_;
		size_t next_ = 0; // 次に使う欠片
		std::mt19937 random_{ std::random_device{}() };
	};
}
