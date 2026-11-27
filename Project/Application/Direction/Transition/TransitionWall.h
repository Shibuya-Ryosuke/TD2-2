#pragma once
#include <RyoEngine.h>
#include <array>
#include <cstdint>
#include "TransitionPiece.h"
#include "../../Config/DirectionConfig.h"

namespace App
{
	struct TransitionView;

	//飛び込んだ積み木で視界を埋め、切り替え先で同じ積み木を落とす
	class TransitionWall
	{
	public:
		TransitionWall() = default;
		~TransitionWall() = default;

		TransitionWall(const TransitionWall&) = delete;
		TransitionWall& operator=(const TransitionWall&) = delete;

		void Initialize(RyoEngine::WorldTransform* frame, uint32_t shader);
		void Begin(uint32_t seed);
		void Hide();
		void UpdateCover(float time, const TransitionView& view);
		void UpdateDrop(float time, const TransitionView& view);
		void Draw(const TransitionView& view);

	private:
		static constexpr int32_t kColumns = DirectionConfig::kTransitionColumns;
		static constexpr int32_t kRows = DirectionConfig::kTransitionRows;
		static constexpr int32_t kBlockCount = kColumns * kRows;

		struct Motion
		{
			RyoEngine::Vector3 offset{};
			RyoEngine::Vector3 launch{};
			RyoEngine::Vector3 settleRotate{};
			RyoEngine::Vector3 flightSpin{};
			RyoEngine::Vector3 dropSpin{};
			float size = 1.0f;
			float flightTime = 0.0f;
			float arrivalJitter = 0.0f;
			float dropJitter = 0.0f;
		};

		int32_t GetUsedColumns(const TransitionView& view) const;

		std::array<TransitionPiece, kBlockCount> pieces_;
		std::array<Motion, kBlockCount> motions_{};
	};
}
