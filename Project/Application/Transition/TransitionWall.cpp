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
		// 0 〜 1 の割合 (範囲の外は 0 か 1)
		float Clamp01(float value)
		{
			return std::clamp(value, 0.0f, 1.0f);
		}
	}

	// ブロックを作る
	void TransitionWall::Initialize(WorldTransform* frame, uint32_t shader)
	{
		for (int32_t i = 0; i < kBlockCount; ++i)
		{
			TransitionPiece& piece = pieces_[static_cast<size_t>(i)];
			piece.Initialize(frame, shader);
			piece.seed = static_cast<float>(i) * 1.37f;
			// ドン！の縁の光の色 (パステルを斜めのしま模様に並べる)
			const int32_t column = i % GameConfig::kTransitionColumns;
			const int32_t row = i / GameConfig::kTransitionColumns;
			const size_t colorIndex = static_cast<size_t>(column + row * 2) % GameConfig::kTransitionPopColors.size();
			const Vector4& color = GameConfig::kTransitionPopColors[colorIndex];
			piece.glowColor = { color.x, color.y, color.z };
		}
		Hide();
	}

	// 全部消す
	void TransitionWall::Hide()
	{
		for (TransitionPiece& piece : pieces_)
		{
			piece.isVisible = false;
		}
	}

	// 置き場所 (画面を 5 x 5 に分けたマスの真ん中。奥行きは壁の距離)
	Vector3 TransitionWall::GetHomePosition(int32_t index, const TransitionView& view) const
	{
		const int32_t column = index % GameConfig::kTransitionColumns;
		const int32_t row = index / GameConfig::kTransitionColumns;
		const float cellWidth = view.GetHalfWidth() * 2.0f / static_cast<float>(GameConfig::kTransitionColumns);
		const float cellHeight = view.halfHeight * 2.0f / static_cast<float>(GameConfig::kTransitionRows);
		return Vector3{
			-view.GetHalfWidth() + (static_cast<float>(column) + 0.5f) * cellWidth,
			-view.halfHeight + (static_cast<float>(row) + 0.5f) * cellHeight,
			view.refDepth };
	}

	// ブロックの大きさ (横長の積み木。奥行きは高さと同じ)
	Vector3 TransitionWall::GetBlockSize(const TransitionView& view) const
	{
		const float fill = 1.0f - GameConfig::kTransitionBlockGap;
		const float cellWidth = view.GetHalfWidth() * 2.0f / static_cast<float>(GameConfig::kTransitionColumns);
		const float cellHeight = view.halfHeight * 2.0f / static_cast<float>(GameConfig::kTransitionRows);
		return Vector3{ cellWidth * fill, cellHeight * fill, cellHeight };
	}

	// 積み上がる
	void TransitionWall::UpdateStack(float time, const TransitionView& view)
	{
		// 1個ずつ少しずらして落とす (最後の1個がちょうど kTransitionStackTime で着く)
		const float interval = (GameConfig::kTransitionStackTime - GameConfig::kTransitionBlockFallTime) / static_cast<float>(kBlockCount - 1);
		const Vector3 blockSize = GetBlockSize(view);
		const float startY = view.halfHeight + blockSize.y * 2.0f; // 画面の上の外から落ちてくる

		for (int32_t i = 0; i < kBlockCount; ++i)
		{
			TransitionPiece& piece = pieces_[static_cast<size_t>(i)];
			const Vector3 home = GetHomePosition(i, view);
			const float localTime = time - interval * static_cast<float>(i);
			piece.isVisible = localTime >= 0.0f;
			piece.rotate = { 0.0f, 0.0f, 0.0f };
			piece.size = blockSize;
			piece.position = home;
			piece.wind = 0.0f;
			piece.glow = 0.0f;
			if (!piece.isVisible)
			{
				continue;
			}

			// 落ちてくる途中 (だんだん速く。速いほど縦にびよーんとのびる)
			const float fall = localTime / GameConfig::kTransitionBlockFallTime;
			if (fall < 1.0f)
			{
				const float stretch = fall * GameConfig::kTransitionFallStretch;
				piece.size.y = blockSize.y * (1.0f + stretch);
				piece.size.x = blockSize.x * (1.0f - stretch * GameConfig::kTransitionSquashWiden);
				piece.position.y = startY + (home.y - startY) * fall * fall;
				continue;
			}

			// 着いた: ドン！で縁がカラフルに光って、ぽよよんと弾む (下の面はそのまま)
			const float landed = localTime - GameConfig::kTransitionBlockFallTime;
			piece.glow = Clamp01(1.0f - landed / GameConfig::kTransitionGlowTime);
			const float bounce = GameConfig::kTransitionSquash * std::exp(-GameConfig::kTransitionSquashDamping * landed)
				* std::cos(GameConfig::kTransitionSquashSpeed * landed);
			piece.size.y = blockSize.y * (1.0f - bounce);
			piece.size.x = blockSize.x * (1.0f + bounce * GameConfig::kTransitionSquashWiden);
			piece.position.y = home.y - blockSize.y * bounce * 0.5f;
		}
		ApplyAll(view);
	}

	// 全部置いた状態にする
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
			piece.glow = 0.0f;
			piece.wind = 0.0f;
		}
		ApplyAll(view);
	}

	// 倒れ方をブロックごとに決める
	void TransitionWall::BeginTopple(std::mt19937& random, const TransitionView& view)
	{
		std::uniform_real_distribution<float> unit(0.0f, 1.0f);
		for (int32_t i = 0; i < kBlockCount; ++i)
		{
			const int32_t row = i / GameConfig::kTransitionColumns;
			const Vector3 home = GetHomePosition(i, view);
			ToppleMotion& motion = motions_[static_cast<size_t>(i)];

			// 上の段から倒れ始める (同じ段でも少しばらつかせる)
			motion.delay = static_cast<float>(GameConfig::kTransitionRows - 1 - row) * GameConfig::kTransitionRowDelay
				+ unit(random) * GameConfig::kTransitionRandomDelay;

			// ぴょんと跳ねてから、くるくる回って手前に来ながら下へ落ちて、画面の外へ出ていく
			motion.hop = view.halfHeight * GameConfig::kTransitionHopHeight * (0.7f + unit(random) * 0.6f);
			motion.endDepth = view.refDepth * (0.35f + unit(random) * 0.1f);
			motion.drop = home.y + view.halfHeight * (0.9f + unit(random) * 0.6f);
			motion.driftX = home.x * (0.2f + unit(random) * 0.3f);
			motion.tilt = 200.0f + unit(random) * 160.0f;
			motion.spinY = (unit(random) - 0.5f) * 180.0f;
			motion.spinZ = (unit(random) - 0.5f) * 240.0f;
		}
		SetCovered(view);
	}

	// 倒れる
	void TransitionWall::UpdateTopple(float time, const TransitionView& view)
	{
		const Vector3 blockSize = GetBlockSize(view);
		for (int32_t i = 0; i < kBlockCount; ++i)
		{
			TransitionPiece& piece = pieces_[static_cast<size_t>(i)];
			const ToppleMotion& motion = motions_[static_cast<size_t>(i)];
			const Vector3 home = GetHomePosition(i, view);
			piece.size = blockSize;
			piece.glow = 0.0f;

			// 倒れる前: 風を受けてグラグラ揺れる (揺れはシェーダー)
			piece.wind = Clamp01(time / GameConfig::kTransitionWindRiseTime);
			const float flightTime = std::max(0.01f, GameConfig::kTransitionToppleTime - motion.delay);
			const float t = Clamp01((time - motion.delay) / flightTime);
			if (time < motion.delay)
			{
				piece.position = home;
				piece.rotate = { 0.0f, 0.0f, 0.0f };
				continue;
			}

			// ぐっ: 跳ねる前にかがむ (つぶれる) → ぴょん: 縦にのびながら上へ跳ねる
			const float hopRate = Clamp01(t / GameConfig::kTransitionHopTime);
			const float crouch = Clamp01(hopRate / GameConfig::kTransitionHopCrouch);
			const float jump = Clamp01((hopRate - GameConfig::kTransitionHopCrouch) / (1.0f - GameConfig::kTransitionHopCrouch));
			const float hop = std::sin(std::numbers::pi_v<float> * 0.5f * jump) * motion.hop;
			const float stretch = (std::sin(std::numbers::pi_v<float> * jump) - std::sin(std::numbers::pi_v<float> * crouch)) * GameConfig::kTransitionHopSquash;
			piece.size.y = blockSize.y * (1.0f + stretch);
			piece.size.x = blockSize.x * (1.0f - stretch * GameConfig::kTransitionSquashWiden);
			const float floorOffset = blockSize.y * stretch * 0.5f; // 下の面を床につけたまま

			// 一番上まで来たら、くるくる回りながら手前に来て下へ落ちる (だんだん速く)
			const float fall = Clamp01((t - GameConfig::kTransitionHopTime) / (1.0f - GameConfig::kTransitionHopTime));
			const float ease = t * t;
			piece.position.x = home.x + motion.driftX * ease;
			piece.position.y = home.y + floorOffset + hop - (motion.drop + motion.hop) * fall * fall;
			piece.position.z = home.z + (motion.endDepth - home.z) * ease;
			const float spin = std::pow(t, 1.5f);
			piece.rotate = { -motion.tilt * spin, motion.spinY * spin, motion.spinZ * spin };

			// 倒れ終わったら消す
			if (t >= 1.0f)
			{
				piece.isVisible = false;
			}
		}
		ApplyAll(view);
	}

	// 行列を確定する
	void TransitionWall::ApplyAll(const TransitionView& view)
	{
		for (TransitionPiece& piece : pieces_)
		{
			piece.Apply(view);
		}
	}

	// 見えているブロックを描く
	void TransitionWall::Draw()
	{
		for (TransitionPiece& piece : pieces_)
		{
			piece.Draw();
		}
	}
}
