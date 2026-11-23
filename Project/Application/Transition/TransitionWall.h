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

	// 画面を埋める 5 x 5 の積み木 (場面切り替えの演出)
	// ・積み上がる: 下の段の左から1個ずつ、上から落ちてきてドン！(縁がカラフルに光って、ぽよよんと弾む)
	// ・倒れる: 風に押されて、上の段から順にぴょんと跳ねて、くるくる回りながら手前 (カメラの方) へ落ちていく
	class TransitionWall
	{
	public:
		static constexpr int32_t kBlockCount = GameConfig::kTransitionColumns * GameConfig::kTransitionRows;

		TransitionWall() = default;
		~TransitionWall() = default;

		TransitionWall(const TransitionWall&) = delete;
		TransitionWall& operator=(const TransitionWall&) = delete;

		void Initialize(RyoEngine::WorldTransform* frame, uint32_t shader); // ブロックを作る

		void Hide(); // 全部消す

		// 積み上がる (time: 積み上がり始めてからの時間)
		void UpdateStack(float time, const TransitionView& view);
		// 全部置いた状態にする (画面が埋まっている)
		void SetCovered(const TransitionView& view);

		// 倒れ方をブロックごとに決める
		void BeginTopple(std::mt19937& random, const TransitionView& view);
		// 倒れる (time: 倒れ始めてからの時間)
		void UpdateTopple(float time, const TransitionView& view);

		void Draw();

	private:
		// ブロックごとの倒れ方
		struct ToppleMotion
		{
			float delay = 0.0f;      // 倒れ始めるまで (上の段ほど早い)
			float hop = 0.0f;        // ぴょんと跳ねる高さ
			float endDepth = 4.0f;   // 最後にどこまで手前に来るか
			float drop = 0.0f;       // どれだけ下に落ちるか
			float driftX = 0.0f;     // 横にずれる量
			float tilt = 270.0f;     // 手前に回る角度 (度)
			float spinY = 0.0f;      // 横に回る角度 (度)
			float spinZ = 0.0f;      // ひねる角度 (度)
		};

		// 置き場所 (index 番目のブロック。下の段の左から)
		RyoEngine::Vector3 GetHomePosition(int32_t index, const TransitionView& view) const;
		// ブロックの大きさ (すき間の分だけ小さく)
		RyoEngine::Vector3 GetBlockSize(const TransitionView& view) const;
		// 全部の行列を確定する
		void ApplyAll(const TransitionView& view);

		std::array<TransitionPiece, kBlockCount> pieces_;
		std::array<ToppleMotion, kBlockCount> motions_{};
	};
}
