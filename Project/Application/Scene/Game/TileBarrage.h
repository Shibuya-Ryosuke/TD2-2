#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <vector>
#include "../../Stage/Grid/Cell.h"

namespace App
{
	class Floor;

	//上の層に残ってためた足場を、穴のとなりから先に1枚ずつボスへ飛ばしてダメージを入れる
	class TileBarrage
	{
	public:
		//飛ばし始めて飛ばす枚数を返す
		int32_t Begin(Floor* upper);

		//1フレーム進めて、増えたダメージを返す
		int32_t Advance(Floor& upper, const RyoEngine::Vector3& target, float deltaTime);
		//このフレームで新しく届いた枚数
		int32_t GetArrivedNow() const { return arrivedNow_; }

		//全部届いたか
		bool IsFinished() const { return arrived_ >= static_cast<int32_t>(tiles_.size()); }
		int32_t GetTileCount() const { return static_cast<int32_t>(tiles_.size()); }
		int32_t GetDamageApplied() const { return damageApplied_; }
		//そのマスの足場が飛び立ったか、飛ばす足場がないか
		bool IsLaunched(const Cell& cell) const;

	private:
		struct BarrageTile
		{
			Cell cell{};
			int32_t damage = 0; //届いたときのボスへのダメージ
			RyoEngine::Vector3 start{}; //飛び立つときの位置で、ためて浮いた高さから飛ぶ
		};

		//i枚目が飛び立つ秒数
		float GetLaunchTime(int32_t index) const;

		std::vector<BarrageTile> tiles_; //穴のとなりの足場が先に並ぶ飛ばす足場
		int32_t adjacentCount_ = 0; //先に飛ばす穴のとなりの足場の数
		float timer_ = 0.0f; //飛ばし始めてからの時間
		int32_t arrived_ = 0; //届いた枚数
		int32_t arrivedNow_ = 0; //このフレームで新しく届いた枚数
		int32_t damageApplied_ = 0; //もう入れたダメージ
	};
}
