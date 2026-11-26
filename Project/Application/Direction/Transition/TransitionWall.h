#pragma once
#include <RyoEngine.h>
#include <array>
#include <cstdint>
#include <random>
#include "TransitionPiece.h"
#include "../../Config/DirectionConfig.h"

namespace App
{
	struct TransitionView;

	//画面を埋める正方形の積み木で、ドン！と積み上がって横に吹き飛ぶ
	class TransitionWall
	{
	public:
		static constexpr int32_t kBlockCount = DirectionConfig::kTransitionColumns * DirectionConfig::kTransitionRows;

		TransitionWall() = default;
		~TransitionWall() = default;

		TransitionWall(const TransitionWall&) = delete;
		TransitionWall& operator=(const TransitionWall&) = delete;

		void Initialize(RyoEngine::WorldTransform* frame, uint32_t shader); //ブロックを作る

		void Hide(); //全部消す

		//積み上がり始めてからの時間で積み上げる
		void UpdateStack(float time, const TransitionView& view);
		//画面が埋まった状態にする
		void SetCovered(const TransitionView& view);

		//ブロックごとの飛ばされ方を決めて埋まった状態から始める
		void BeginBlow(std::mt19937& random, const TransitionView& view);
		//吹き始めてからの時間で横に吹き飛ばす
		void UpdateBlow(float time, const TransitionView& view);

		void Draw();

	private:
		//ブロックごとの飛ばされ方
		struct BlowMotion
		{
			float delay = 0.0f; //左の列ほど早い飛ばされ始めるまでの時間
			float travel = 0.0f; //画面の外まで右へ動く長さ
			float lift = 0.0f; //ふわっと上下に流れる量
			float towardCamera = 0.0f; //少し手前に来る量
			float spinX = 0.0f; //回る角度
			float spinY = 0.0f;
			float spinZ = 0.0f; //右へ転がる角度
		};

		//下の段の左から数えたブロックの置き場所
		RyoEngine::Vector3 GetHomePosition(int32_t index, const TransitionView& view) const;
		//すき間の分だけ小さい、元のモデルをそのまま大きくした正方形のブロックの大きさ
		RyoEngine::Vector3 GetBlockSize(const TransitionView& view) const;
		//画面の高さで割ったマスの1辺
		float GetCellSize(const TransitionView& view) const;
		//画面の横幅を埋めるのに使う列の数
		int32_t GetUsedColumns(const TransitionView& view) const;
		//使う列の中のブロックか
		bool IsUsed(int32_t index, const TransitionView& view) const;
		//下の段の左から数えた積む順番
		int32_t GetStackOrder(int32_t index, const TransitionView& view) const;
		//全部の行列を確定する
		void ApplyAll(const TransitionView& view);
		//シェーダーに縁の光を渡す
		void SetGlow(int32_t index, float glow);

		std::array<TransitionPiece, kBlockCount> pieces_;
		std::array<BlowMotion, kBlockCount> motions_{};
	};
}
