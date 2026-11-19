#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <vector>
#include "../../Stage/Grid/Cell.h"

namespace App
{
	class Floor;

	// 上の層に残った足場を、1枚ずつボスへ飛ばす流れ (Game・Floor から分けたもの)
	// 届いた足場の分だけボスにダメージ (穴のとなりの足場は 4 倍。上限あり)
	class TileBarrage
	{
	public:
		// 飛ばし始める (upper が nullptr なら 0 枚)。飛ばす枚数を返す
		int32_t Begin(const Floor* upper);

		// 1フレーム進める。このフレームで増えたダメージを返す
		// target: 飛んでいく先 (ボス)
		int32_t Advance(Floor& upper, const RyoEngine::Vector3& target, float deltaTime);

		// 全部届いたか
		bool IsFinished() const { return arrived_ >= static_cast<int32_t>(tiles_.size()); }
		int32_t GetTileCount() const { return static_cast<int32_t>(tiles_.size()); }
		int32_t GetDamageApplied() const { return damageApplied_; }

	private:
		struct BarrageTile
		{
			Cell cell{};
			int32_t multiplier = 1; // ダメージの倍率 (穴のとなりは 4)
		};

		std::vector<BarrageTile> tiles_; // 飛ばす足場
		float timer_ = 0.0f;             // 飛ばし始めてからの時間
		int32_t arrived_ = 0;            // 届いた枚数
		int32_t damageApplied_ = 0;      // もう入れたダメージ
	};
}
