#pragma once
#include "../../Object/Boss/BossTuning.h"
#include "../../Stage/Grid/Cell.h"

namespace App
{
	class Floor;
	class Player;
	class Boss;
	class PlayerHealth;

	//プレイヤーと同時に動くボスとのやりとりで、起きたことをGameに返す
	class BossBattle
	{
	public:
		//このフレームで起きたこと
		enum class Event
		{
			None, //何もなく戦闘を続ける
			BossIsolated, //ボスの島が切り離された
			BossFell, //ボスが穴に落ちた
			Separated, //おたがいにたどり着けなくなった
			PlayerDefeated //プレイヤーのHPが0になった
		};

		//配置し直したときに何も起きていない状態に戻す
		void Reset() { separationNeedsCheck_ = false; }
		//戦闘を始めるときにたどり着けるか調べなくてよくする
		void ClearSeparationCheck() { separationNeedsCheck_ = false; }

		//ボスの足元が崩れて逃げられずに落ちたらBossFell
		Event CheckBossFooting(const Floor& floor, Boss& boss, const BossTuning& tuning, bool isFloorChanged, const Cell& bossCell);

		//戦闘中の1フレーム
		Event Update(Floor& floor, Player& player, Boss& boss, PlayerHealth& health, const BossTuning& tuning,
			bool isFloorChanged, const Cell& bossCell, float deltaTime);

	private:
		//おたがいにたどり着けなくなったか
		bool IsSeparated(const Floor& floor, const Player& player, const Boss& boss, const BossTuning& tuning);

		bool separationNeedsCheck_ = false; //床が変わったのでたどり着けるか調べる
	};
}
