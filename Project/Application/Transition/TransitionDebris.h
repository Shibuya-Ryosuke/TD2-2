#pragma once
#include <RyoEngine.h>
#include <array>
#include <cstdint>
#include <random>
#include "TransitionPiece.h"
#include "../Config/GameConfig.h"

namespace App
{
	struct TransitionView;

	// 最初に落ちてくる がれき = カラフルな紙吹雪 (小さいブロック 20 個。ひらひら回りながら、画面の下へ消えていく)
	class TransitionDebris
	{
	public:
		TransitionDebris() = default;
		~TransitionDebris() = default;

		TransitionDebris(const TransitionDebris&) = delete;
		TransitionDebris& operator=(const TransitionDebris&) = delete;

		void Initialize(RyoEngine::WorldTransform* frame, uint32_t shader); // ブロックを作る

		// 落ち方を1個ずつ決める
		void Begin(std::mt19937& random, const TransitionView& view);
		// 落とす (time: 落ち始めてからの時間)
		void Update(float time, const TransitionView& view);
		void Hide(); // 全部消す
		void Draw();

	private:
		// 1個ずつの落ち方
		struct FallMotion
		{
			float delay = 0.0f;                    // 落ち始めるまで
			RyoEngine::Vector3 start{};            // 落ち始める位置 (画面の上の外)
			float travel = 0.0f;                   // 下へ動く長さ (画面の下の外まで)
			RyoEngine::Vector3 spin{};             // 落ちきるまでに回る角度 (度)
			float swayWidth = 0.0f;                // 左右にひらひらする幅
			float swayPhase = 0.0f;                // ひらひらのずれ
		};

		std::array<TransitionPiece, GameConfig::kTransitionDebrisCount> pieces_;
		std::array<FallMotion, GameConfig::kTransitionDebrisCount> motions_{};
	};
}
