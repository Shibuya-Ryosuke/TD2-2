#pragma once
#include <RyoEngine.h>
#include <array>
#include <cstdint>
#include <memory>
#include <random>
#include "../../Config/DirectionConfig.h"

namespace App
{
	//床が砕けたときに飛び散るピンクの欠片を使い回す
	class FloorShards
	{
	public:
		FloorShards() = default;
		~FloorShards() = default;

		FloorShards(const FloorShards&) = delete;
		FloorShards& operator=(const FloorShards&) = delete;

		void Initialize(); //モデルを作る

		//タイルの真ん中から1マス分の欠片を飛ばす
		void Burst(const RyoEngine::Vector3& center, float tileSize);

		void Update(); //欠片を飛ばして小さくする
		void Draw();

	private:
		//欠片1個
		struct Shard
		{
			//unique_ptrを持つのでコピーを禁止する
			Shard() = default;
			~Shard() = default;
			Shard(const Shard&) = delete;
			Shard& operator=(const Shard&) = delete;

			std::unique_ptr<RyoEngine::Model> model;
			RyoEngine::Vector3 position{};
			RyoEngine::Vector3 velocity{};
			RyoEngine::Vector3 rotate{};
			RyoEngine::Vector3 spin{}; //1秒に回る角度
			float size = 0.0f;
			float life = 0.0f; //残りの時間で0以下は使っていない
		};

		std::array<Shard, DirectionConfig::kShardPoolSize> shards_;
		size_t next_ = 0; //次に使う欠片
		std::mt19937 random_{ std::random_device{}() };
	};
}
