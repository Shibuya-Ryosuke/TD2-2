#pragma once
#include <RyoEngine.h>
#include <memory>

namespace App
{
	struct TransitionView;

	// 場面切り替えの演出のブロック1個 (積み上がるブロック・がれき どちらにも使う)
	// 見た目は cube.obj (仮)。TransitionBlock シェーダーで、縁の光と風の揺れを出す
	class TransitionPiece
	{
	public:
		TransitionPiece() = default;
		~TransitionPiece() = default;

		TransitionPiece(const TransitionPiece&) = delete;
		TransitionPiece& operator=(const TransitionPiece&) = delete;

		// モデルを作る。frame: カメラの位置・向き (親にする) / shader: TransitionBlock シェーダー
		void Initialize(RyoEngine::WorldTransform* frame, uint32_t shader);

		// 行列を確定する (位置・大きさを view に合わせて直す)
		void Apply(const TransitionView& view);
		void Draw();
		// モデルの色 (紙吹雪の色など)
		void SetColor(const RyoEngine::Vector4& color);

		// --- 動かす値 (カメラから見た空間。TransitionView を参照) ---
		RyoEngine::Vector3 position{};                 // 位置 (z: カメラからの奥行き)
		RyoEngine::Vector3 rotate{};                   // 回転 (度)
		RyoEngine::Vector3 size = { 1.0f, 1.0f, 1.0f }; // 大きさ (箱の 幅・高さ・奥行き)
		float glow = 0.0f;                             // 縁の光 (0 〜 1)
		RyoEngine::Vector3 glowColor = { 1.0f, 1.0f, 1.0f }; // 縁の光の色
		float flatColor = 0.0f;                        // 1 にすると木目を消して、SetColor の色だけで塗る (紙吹雪用)
		float wind = 0.0f;                             // 風の揺れ (0 〜 1)
		float seed = 0.0f;                             // 揺れ方をずらす値 (ブロックごと)
		bool isVisible = false;                        // 描くか

	private:
		std::unique_ptr<RyoEngine::Model> model_{};
	};
}
