#pragma once
#include <RyoEngine.h>
#include <array>
#include <cstdint>
#include <memory>
#include <random>
#include "../../Config/DirectionConfig.h"

namespace App
{
	//跳んだときと着地で足場からカラフルなハートがぶわっとはじけて上へ舞う
	class HeartBurst
	{
	public:
		HeartBurst() = default;
		~HeartBurst() = default;

		HeartBurst(const HeartBurst&) = delete;
		HeartBurst& operator=(const HeartBurst&) = delete;

		void Initialize(); //ハートのモデルを作る

		//足元から強さpowerに合わせた数のハートをはじけさせる
		void Burst(const RyoEngine::Vector3& feet, float power);

		void Update(); //ハートを舞い上がらせて消す
		void Draw();

	private:
		//ハート1個
		struct Heart
		{
			//unique_ptrを持つのでコピーを禁止する
			Heart() = default;
			~Heart() = default;
			Heart(const Heart&) = delete;
			Heart& operator=(const Heart&) = delete;

			std::unique_ptr<RyoEngine::Model> model;
			RyoEngine::Vector3 position{};
			RyoEngine::Vector3 velocity{};
			float roll = 0.0f; //面の中で回る角度
			float rollSpeed = 0.0f; //回る速さ
			float sway = 0.0f; //左右にゆれるタイミング
			float size = 0.0f;
			float life = 0.0f; //残りの時間で0以下は使っていない
			float maxLife = 0.0f; //出たときの時間
		};

		std::array<Heart, DirectionConfig::kHeartPoolSize> hearts_;
		size_t next_ = 0; //次に使うハート
		std::mt19937 random_{ std::random_device{}() };
	};
}
