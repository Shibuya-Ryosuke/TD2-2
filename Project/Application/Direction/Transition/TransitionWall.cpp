#include "TransitionWall.h"
#include "TransitionView.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		//0から1におさめる
		float Clamp01(float value)
		{
			return std::clamp(value, 0.0f, 1.0f);
		}
	}

	//ブロックを作る
	void TransitionWall::Initialize(WorldTransform* frame, uint32_t shader)
	{
		for (int32_t i = 0; i < kBlockCount; ++i)
		{
			TransitionPiece& piece = pieces_[static_cast<size_t>(i)];
			piece.Initialize(frame, shader);
			//ドン！の縁の光の色をパステルの斜めのしま模様に並べる
			const int32_t column = i % DirectionConfig::kTransitionColumns;
			const int32_t row = i / DirectionConfig::kTransitionColumns;
			const size_t colorIndex = static_cast<size_t>(column + row * 2) % DirectionConfig::kTransitionPopColors.size();
			const Vector4& color = DirectionConfig::kTransitionPopColors[colorIndex];
			piece.shaderParams1 = { color.x, color.y, color.z, 0.0f }; //縁の光の色
		}
		Hide();
	}

	//全部消す
	void TransitionWall::Hide()
	{
		for (TransitionPiece& piece : pieces_)
		{
			piece.isVisible = false;
		}
	}

	//画面を5x5に分けたマスの真ん中の置き場所
	Vector3 TransitionWall::GetHomePosition(int32_t index, const TransitionView& view) const
	{
		const int32_t column = index % DirectionConfig::kTransitionColumns;
		const int32_t row = index / DirectionConfig::kTransitionColumns;
		const float cellWidth = view.GetHalfWidth() * 2.0f / static_cast<float>(DirectionConfig::kTransitionColumns);
		const float cellHeight = view.halfHeight * 2.0f / static_cast<float>(DirectionConfig::kTransitionRows);
		return Vector3{
			-view.GetHalfWidth() + (static_cast<float>(column) + 0.5f) * cellWidth,
			-view.halfHeight + (static_cast<float>(row) + 0.5f) * cellHeight,
			view.refDepth };
	}

	//横長で奥行きは高さと同じブロックの大きさ
	Vector3 TransitionWall::GetBlockSize(const TransitionView& view) const
	{
		const float fill = 1.0f - DirectionConfig::kTransitionBlockGap;
		const float cellWidth = view.GetHalfWidth() * 2.0f / static_cast<float>(DirectionConfig::kTransitionColumns);
		const float cellHeight = view.halfHeight * 2.0f / static_cast<float>(DirectionConfig::kTransitionRows);
		return Vector3{ cellWidth * fill, cellHeight * fill, cellHeight };
	}

	//積み上げる
	void TransitionWall::UpdateStack(float time, const TransitionView& view)
	{
		//最後の1個がちょうど積み上がる時間で着くように少しずつずらして落とす
		const float interval = (DirectionConfig::kTransitionStackTime - DirectionConfig::kTransitionBlockFallTime) / static_cast<float>(kBlockCount - 1);
		const Vector3 blockSize = GetBlockSize(view);
		const float startY = view.halfHeight + blockSize.y * 2.0f; //画面の上の外から落ちてくる

		for (int32_t i = 0; i < kBlockCount; ++i)
		{
			TransitionPiece& piece = pieces_[static_cast<size_t>(i)];
			const Vector3 home = GetHomePosition(i, view);
			const float localTime = time - interval * static_cast<float>(i);
			piece.isVisible = localTime >= 0.0f;
			piece.rotate = { 0.0f, 0.0f, 0.0f };
			piece.size = blockSize;
			piece.position = home;
			SetGlow(i, 0.0f);
			if (!piece.isVisible)
			{
				continue;
			}

			//落ちてくる途中はだんだん速く縦にのびる
			const float fall = localTime / DirectionConfig::kTransitionBlockFallTime;
			if (fall < 1.0f)
			{
				const float stretch = fall * DirectionConfig::kTransitionFallStretch;
				piece.size.y = blockSize.y * (1.0f + stretch);
				piece.size.x = blockSize.x * (1.0f - stretch * DirectionConfig::kTransitionSquashWiden);
				piece.position.y = startY + (home.y - startY) * fall * fall;
				continue;
			}

			//着いたらドン！で縁が光ってぽよよんと弾む
			const float landed = localTime - DirectionConfig::kTransitionBlockFallTime;
			SetGlow(i, Clamp01(1.0f - landed / DirectionConfig::kTransitionGlowTime));
			const float bounce = DirectionConfig::kTransitionSquash * std::exp(-DirectionConfig::kTransitionSquashDamping * landed)
				* std::cos(DirectionConfig::kTransitionSquashSpeed * landed);
			piece.size.y = blockSize.y * (1.0f - bounce);
			piece.size.x = blockSize.x * (1.0f + bounce * DirectionConfig::kTransitionSquashWiden);
			piece.position.y = home.y - blockSize.y * bounce * 0.5f;
		}
		ApplyAll(view);
	}

	//全部置いた状態にする
	void TransitionWall::SetCovered(const TransitionView& view)
	{
		const Vector3 blockSize = GetBlockSize(view);
		for (int32_t i = 0; i < kBlockCount; ++i)
		{
			TransitionPiece& piece = pieces_[static_cast<size_t>(i)];
			piece.isVisible = true;
			piece.position = GetHomePosition(i, view);
			piece.rotate = { 0.0f, 0.0f, 0.0f };
			piece.size = blockSize;
			SetGlow(i, 0.0f);
		}
		ApplyAll(view);
	}

	//シェーダーに縁の光を渡す
	void TransitionWall::SetGlow(int32_t index, float glow)
	{
		pieces_[static_cast<size_t>(index)].shaderParams0 = { glow, 0.0f, 0.0f, 0.0f };
	}

	//ブロックごとの飛ばされ方を決めて埋まった状態から始める
	void TransitionWall::BeginBlow(std::mt19937& random, const TransitionView& view)
	{
		std::uniform_real_distribution<float> unit(0.0f, 1.0f);
		for (int32_t i = 0; i < kBlockCount; ++i)
		{
			const int32_t column = i % DirectionConfig::kTransitionColumns;
			const Vector3 home = GetHomePosition(i, view);
			BlowMotion& motion = motions_[static_cast<size_t>(i)];

			//風は左からなので左の列から少しばらつかせて飛ばす
			motion.delay = static_cast<float>(column) * DirectionConfig::kTransitionBlowColumnDelay
				+ unit(random) * DirectionConfig::kTransitionBlowRandomDelay;
			//どのブロックも右の画面の外まで出る長さ
			motion.travel = (view.GetHalfWidth() - home.x) + view.GetHalfWidth() * DirectionConfig::kTransitionBlowDistance * (0.8f + unit(random) * 0.4f);
			motion.lift = view.halfHeight * DirectionConfig::kTransitionBlowLift * (unit(random) * 1.4f - 0.2f);
			motion.towardCamera = view.refDepth * unit(random) * 0.15f;
			motion.spinZ = -(240.0f + unit(random) * 360.0f); //右へ転がる向き
			motion.spinX = (unit(random) - 0.5f) * 360.0f;
			motion.spinY = (unit(random) - 0.5f) * 240.0f;
		}
		SetCovered(view);
	}

	//横に吹き飛ばす
	void TransitionWall::UpdateBlow(float time, const TransitionView& view)
	{
		const Vector3 blockSize = GetBlockSize(view);
		const float pi = std::numbers::pi_v<float>;
		for (int32_t i = 0; i < kBlockCount; ++i)
		{
			TransitionPiece& piece = pieces_[static_cast<size_t>(i)];
			const BlowMotion& motion = motions_[static_cast<size_t>(i)];
			const Vector3 home = GetHomePosition(i, view);
			piece.size = blockSize;

			//まだ風が来ていないブロックは置いたまま
			if (time < motion.delay)
			{
				piece.position = home;
				piece.rotate = { 0.0f, 0.0f, 0.0f };
				continue;
			}

			//だんだん速く右へ、ふわっと上下に流れてくるくる転がる
			const float flightTime = std::max(0.01f, DirectionConfig::kTransitionBlowTime - motion.delay);
			const float t = Clamp01((time - motion.delay) / flightTime);
			const float ease = t * t;
			piece.position.x = home.x + motion.travel * ease;
			piece.position.y = home.y + motion.lift * std::sin(pi * 0.5f * t);
			piece.position.z = home.z - motion.towardCamera * t;
			const float spin = std::pow(t, 1.3f);
			piece.rotate = { motion.spinX * spin, motion.spinY * spin, motion.spinZ * spin };

			if (t >= 1.0f)
			{
				piece.isVisible = false;
			}
		}
		ApplyAll(view);
	}

	//行列を確定する
	void TransitionWall::ApplyAll(const TransitionView& view)
	{
		for (TransitionPiece& piece : pieces_)
		{
			piece.Apply(view);
		}
	}

	//見えているブロックを描く
	void TransitionWall::Draw()
	{
		for (TransitionPiece& piece : pieces_)
		{
			piece.Draw();
		}
	}
}
