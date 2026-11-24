#pragma once
#include <RyoEngine.h>
#include <array>
#include <cstdint>
#include <memory>
#include <random>
#include "../../Config/DirectionConfig.h"

namespace App
{
	//着地で足元から広がるパステルのほこりを使い回す
	class PlayerDust
	{
	public:
		PlayerDust() = default;
		~PlayerDust() = default;

		PlayerDust(const PlayerDust&) = delete;
		PlayerDust& operator=(const PlayerDust&) = delete;

		void Initialize(); //モデルを作る

		//着地の強さに合わせて足元からほこりを出す
		void Puff(const RyoEngine::Vector3& feet, float power);

		void Update(); //ほこりを広げて小さくする
		void Draw();

	private:
		//ほこり1個
		struct Puffball
		{
			//unique_ptrを持つのでコピーを禁止する
			Puffball() = default;
			~Puffball() = default;
			Puffball(const Puffball&) = delete;
			Puffball& operator=(const Puffball&) = delete;

			std::unique_ptr<RyoEngine::Model> model;
			RyoEngine::Vector3 position{};
			RyoEngine::Vector3 velocity{};
			float spin = 0.0f; //回っている角度
			float size = 0.0f;
			float life = 0.0f; //残りの時間で0以下は使っていない
		};

		std::array<Puffball, DirectionConfig::kDustPoolSize> puffs_;
		size_t next_ = 0; //次に使うほこり
		std::mt19937 random_{ std::random_device{}() };
	};
}
