#include "TransitionWall.h"
#include "TransitionView.h"
#include <algorithm>
#include <cmath>
#include <numbers>
#include <random>

using namespace RyoEngine;

namespace App
{
	void TransitionWall::Initialize(WorldTransform* frame, uint32_t shader)
	{
		for (TransitionPiece& piece : pieces_)
		{
			piece.Initialize(frame, shader, true);
		}
		Hide();
	}

	//飛ぶ向きと速さと置かれる角度を積み木ごとにずらす
	void TransitionWall::Begin(uint32_t seed)
	{
		std::mt19937 random{ seed };
		std::uniform_real_distribution<float> unit(0.0f, 1.0f);
		const auto signedUnit = [&]() { return unit(random) * 2.0f - 1.0f; };

		for (size_t i = 0; i < pieces_.size(); ++i)
		{
			Motion& motion = motions_[i];
			const float angle = unit(random) * (2.0f * std::numbers::pi_v<float>);
			motion.offset = { signedUnit(), signedUnit(), signedUnit() };
			motion.launch = { std::cos(angle), std::sin(angle), 0.48f + 1.08f * unit(random) };
			motion.settleRotate = { 13.0f * signedUnit(), 15.0f * signedUnit(), 27.0f * signedUnit() };
			motion.flightSpin = { 250.0f * signedUnit(), 290.0f * signedUnit(), 205.0f * signedUnit() };
			motion.dropSpin = { 73.0f * signedUnit(), 86.0f * signedUnit(), 210.0f * signedUnit() };
			motion.size = 1.73f + 0.23f * unit(random);
			motion.flightTime = 0.37f + 0.25f * unit(random);
			motion.arrivalJitter = 0.19f * signedUnit();
			motion.dropJitter = 0.13f * unit(random);

			const size_t colorIndex = static_cast<size_t>(random()) % DirectionConfig::kTransitionPopColors.size();
			const Vector4& color = DirectionConfig::kTransitionPopColors[colorIndex];
			TransitionPiece& piece = pieces_[i];
			piece.SetColor({ 0.72f + color.x * 0.28f, 0.72f + color.y * 0.28f, 0.72f + color.z * 0.28f, 1.0f });
			piece.shaderParams1 = { color.x, color.y, color.z, 0.0f };
		}
		Hide();
	}

	void TransitionWall::Hide()
	{
		for (TransitionPiece& piece : pieces_)
		{
			piece.isVisible = false;
		}
	}

	//中央の前に最後の積み木を置けるように列数を奇数にする
	int32_t TransitionWall::GetUsedColumns(const TransitionView& view) const
	{
		const int32_t count = static_cast<int32_t>(std::ceil(view.aspectRatio * static_cast<float>(kRows)));
		return std::clamp(count | 1, 3, kColumns);
	}

	void TransitionWall::UpdateCover(float time, const TransitionView& view)
	{
		const int32_t columns = GetUsedColumns(view);
		const float halfWidth = view.GetHalfWidth();
		const float cellWidth = 2.0f * halfWidth / static_cast<float>(columns);
		const float cellHeight = 2.0f * view.halfHeight / static_cast<float>(kRows);
		const float tilt = std::min(1.0f, view.refDepth / std::max(cellWidth, cellHeight));

		for (int32_t i = 0; i < kBlockCount; ++i)
		{
			TransitionPiece& piece = pieces_[static_cast<size_t>(i)];
			const Motion& motion = motions_[static_cast<size_t>(i)];
			const int32_t column = i % kColumns;
			const int32_t row = i / kColumns;
			piece.isVisible = false;
			if (column >= columns)
			{
				continue;
			}

			const int32_t x = column - columns / 2;
			const int32_t y = row - kRows / 2;
			const bool isCenter = x == 0 && y == 0;
			const float screenX = static_cast<float>(x) * cellWidth;
			const float screenY = static_cast<float>(y) * cellHeight;
			const float radius = std::max(std::abs(screenX) / halfWidth, std::abs(screenY) / view.halfHeight);
			float arrival = std::clamp(1.98f - radius * 1.50f + motion.arrivalJitter, 0.32f, 1.99f);
			if (isCenter)
			{
				arrival = DirectionConfig::kTransitionImpactTime;
			}
			const float local = time - arrival;
			if (local < -motion.flightTime)
			{
				continue;
			}

			//奥行きが違っても狙った画面位置で積み木どうしを重ねる
			float spread = 1.0f;
			float depth = 1.0f + motion.offset.z * 0.18f;
			float size = motion.size + 0.10f * radius;
			if (std::abs(x) + std::abs(y) == 1)
			{
				spread = 1.33f;
			}
			if (isCenter)
			{
				depth = 0.82f;
				size = 1.87f;
			}
			piece.isVisible = true;
			piece.position = {
				(screenX * spread + cellWidth * motion.offset.x * 0.14f) * depth,
				(screenY * spread + cellHeight * motion.offset.y * 0.14f) * depth,
				view.refDepth * depth
			};
			piece.size = { cellWidth * size * depth, cellHeight * size * depth, view.refDepth * depth * 0.29f };
			piece.rotate = { motion.settleRotate.x * tilt, motion.settleRotate.y * tilt, motion.settleRotate.z };
			float glow = 0.0f;

			if (local < 0.0f)
			{
				const float t = 1.0f + local / motion.flightTime;
				const float remaining = 1.0f - t * t;
				//横と縦と奥から別々の軌道で飛び込みながら三軸で回る
				const float spreadInAir = 1.0f - 0.78f * static_cast<float>(isCenter);
				piece.position.x += (motion.launch.x * halfWidth * 2.8f - screenX) * remaining * spreadInAir;
				piece.position.y += (motion.launch.y * view.halfHeight * 2.65f - screenY) * remaining * spreadInAir;
				piece.position.z += view.refDepth * motion.launch.z * remaining;
				piece.rotate = piece.rotate + motion.flightSpin * remaining;
			}
			else
			{
				const float bounce = std::sin(27.0f * local) * std::exp(-16.0f * local);
				float bounceDepth = 0.055f;
				if (isCenter)
				{
					bounceDepth = 0.13f;
				}
				piece.position.z += view.refDepth * bounceDepth * bounce;
				glow = 0.58f * std::exp(-11.0f * local);
				const float waveTime = time - DirectionConfig::kTransitionImpactTime - radius * 0.10f;
				if (isCenter)
				{
					glow = 1.8f * std::exp(-11.0f * local);
				}
				else if (waveTime >= 0.0f)
				{
					glow += 0.66f * std::exp(-17.0f * waveTime);
				}
			}

			piece.shaderParams0 = { glow, 0.0f, 1.0f, 0.0f };
		}
	}

	void TransitionWall::UpdateDrop(float time, const TransitionView& view)
	{
		//切り替え前の積み木を同じ向きから落とす
		UpdateCover(DirectionConfig::kTransitionCoverTime, view);
		for (int32_t i = 0; i < kBlockCount; ++i)
		{
			TransitionPiece& piece = pieces_[static_cast<size_t>(i)];
			if (!piece.isVisible)
			{
				continue;
			}

			const Motion& motion = motions_[static_cast<size_t>(i)];
			const float height = std::clamp(piece.position.y / view.halfHeight, -1.0f, 1.0f);
			const float delay = 0.04f + (height + 1.0f) * 0.19f + motion.dropJitter;
			const float progress = std::clamp((time - delay) / (DirectionConfig::kTransitionDropTime - delay), 0.0f, 1.0f);
			if (progress >= 1.0f)
			{
				piece.isVisible = false;
				continue;
			}

			const float gravity = progress * progress;
			const float travel = piece.position.y + view.halfHeight * 1.9f + std::max(piece.size.x, piece.size.y);
			piece.position.y -= travel * gravity;
			piece.position.x += motion.launch.x * piece.size.x * gravity * 0.7f;
			piece.rotate = piece.rotate + motion.dropSpin * gravity;
			piece.shaderParams0.x *= std::exp(-8.0f * time);
		}
	}

	void TransitionWall::Draw(const TransitionView& view)
	{
		for (TransitionPiece& piece : pieces_)
		{
			piece.Apply(view);
			piece.Draw();
		}
	}
}
