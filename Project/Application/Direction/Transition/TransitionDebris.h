#pragma once
#include <RyoEngine.h>
#include <array>
#include <cstdint>
#include <random>
#include "TransitionPiece.h"
#include "../../Config/GameConfig.h"
#include "../../Config/DirectionConfig.h"

namespace App
{
	struct TransitionView;

	//最初にひらひら回りながら落ちる紙吹雪
	class TransitionDebris
	{
	public:
		TransitionDebris() = default;
		~TransitionDebris() = default;

		TransitionDebris(const TransitionDebris&) = delete;
		TransitionDebris& operator=(const TransitionDebris&) = delete;

		void Initialize(RyoEngine::WorldTransform* frame, uint32_t shader); //ブロックを作る

		//1個ずつ落ち方を決める
		void Begin(std::mt19937& random, const TransitionView& view);
		//落ち始めてからの時間で落とす
		void Update(float time, const TransitionView& view);
		void Hide(); //全部消す
		void Draw();

	private:
		//1個ずつの落ち方
		struct FallMotion
		{
			float delay = 0.0f; //落ち始めるまでの時間
			RyoEngine::Vector3 start{}; //画面の上の外の落ち始める位置
			float travel = 0.0f; //画面の下の外まで動く長さ
			RyoEngine::Vector3 spin{}; //落ちきるまでに回る角度
			float swayWidth = 0.0f; //左右にひらひらする幅
			float swayPhase = 0.0f; //ひらひらのずれ
		};

		std::array<TransitionPiece, DirectionConfig::kTransitionDebrisCount> pieces_;
		std::array<FallMotion, DirectionConfig::kTransitionDebrisCount> motions_{};
	};
}
