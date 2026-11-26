#include "TransitionDebris.h"
#include "TransitionView.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//ブロックを作る
	void TransitionDebris::Initialize(WorldTransform* frame, uint32_t shader)
	{
		for (size_t i = 0; i < pieces_.size(); ++i)
		{
			pieces_[i].Initialize(frame, shader, false);
		}
		Hide();
	}

	//1個ずつ落ち方を決める
	void TransitionDebris::Begin(std::mt19937& random, const TransitionView& view)
	{
		std::uniform_real_distribution<float> unit(0.0f, 1.0f);
		const float cellHeight = view.halfHeight * 2.0f / static_cast<float>(DirectionConfig::kTransitionRows);
		for (size_t i = 0; i < pieces_.size(); ++i)
		{
			TransitionPiece& piece = pieces_[i];
			FallMotion& motion = motions_[i];

			//積み木の2から4割くらいのばらばらの大きさ
			const float size = cellHeight * (DirectionConfig::kTransitionDebrisMinSize + unit(random) * DirectionConfig::kTransitionDebrisSizeRange);
			piece.size = { size, size, size };
			piece.isVisible = false;

			//パステルのどれかの色で木目を消して塗る
			std::uniform_int_distribution<size_t> pickColor(0, DirectionConfig::kTransitionPopColors.size() - 1);
			piece.SetColor(DirectionConfig::kTransitionPopColors[pickColor(random)]);
			piece.shaderParams0 = { 0.0f, 0.0f, 0.0f, 1.0f }; //木目を消す

			//画面の上の外から下の外まで落ちる
			const float depth = view.refDepth * (0.7f + unit(random) * 0.3f);
			const float halfHeight = view.halfHeight * depth / view.refDepth;
			const float halfWidth = view.GetHalfWidth() * depth / view.refDepth;
			motion.delay = unit(random) * DirectionConfig::kTransitionDebrisSpread;
			motion.start = { (unit(random) * 2.0f - 1.0f) * halfWidth, halfHeight + size * (1.0f + unit(random) * 2.0f), depth };
			motion.travel = motion.start.y + halfHeight + size * 2.0f;
			motion.spin = { (unit(random) - 0.5f) * 1440.0f, (unit(random) - 0.5f) * 1440.0f, (unit(random) - 0.5f) * 1440.0f };
			motion.swayWidth = cellHeight * DirectionConfig::kTransitionDebrisSway * (0.5f + unit(random) * 0.5f);
			motion.swayPhase = unit(random) * 6.28f;
		}
	}

	//回りながら左右にひらひらしてだんだん速く落とす
	void TransitionDebris::Update(float time, const TransitionView& view)
	{
		for (size_t i = 0; i < pieces_.size(); ++i)
		{
			TransitionPiece& piece = pieces_[i];
			const FallMotion& motion = motions_[i];
			const float fallTime = std::max(0.01f, DirectionConfig::kTransitionDebrisTime - motion.delay);
			const float t = (time - motion.delay) / fallTime;
			piece.isVisible = t >= 0.0f && t < 1.0f;
			if (!piece.isVisible)
			{
				continue;
			}
			const float sway = std::sin(time * DirectionConfig::kTransitionDebrisSwaySpeed + motion.swayPhase) * motion.swayWidth;
			piece.position = { motion.start.x + sway, motion.start.y - motion.travel * t * t, motion.start.z };
			piece.rotate = { motion.spin.x * t, motion.spin.y * t, motion.spin.z * t };
			piece.Apply(view);
		}
	}

	//全部消す
	void TransitionDebris::Hide()
	{
		for (TransitionPiece& piece : pieces_)
		{
			piece.isVisible = false;
		}
	}

	//見えているものだけ描く
	void TransitionDebris::Draw()
	{
		for (TransitionPiece& piece : pieces_)
		{
			piece.Draw();
		}
	}
}
