#pragma once
#include "BossContact.h"
#include "PlayerTurn.h"
#include "../../Object/Boss/BossTuning.h"
#include "../../Stage/Grid/Cell.h"

namespace App
{
	class Floor;
	class Player;
	class Boss;
	class PlayerHealth;

	//戦闘中のボスとのやりとりと交互の手番で、起きたことをGameに返す
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

		//配置し直したときに何も起きていない状態とプレイヤーの番に戻す
		void Reset();
		//交互の手番
		PlayerTurn& GetTurn() { return turn_; }
		const PlayerTurn& GetTurn() const { return turn_; }
		//戦闘開始待ちになったらたどり着けるか調べなくてよくする
		void ClearSeparationCheck() { separationNeedsCheck_ = false; }

		//ボスの足元が崩れて逃げられずに落ちたらBossFell
		Event CheckBossFooting(const Floor& floor, const Player& player, Boss& boss, const BossTuning& tuning, bool isFloorChanged, const Cell& bossCell);

		//戦闘中の1フレーム
		Event Update(Floor& floor, Player& player, Boss& boss, PlayerHealth& health, const BossTuning& tuning,
			bool isFloorChanged, const Cell& bossCell, bool isJumpImpact, float deltaTime);

		//ボスに触れたときのはじきだけ調べる
		void CheckContact(const Floor& floor, Player& player, Boss& boss, bool isJumpImpact);

	private:
		//おたがいにたどり着けなくなったか
		bool IsSeparated(const Floor& floor, const Player& player, const Boss& boss);

		BossContact contact_; //ボスに触れたときのはじき
		bool separationNeedsCheck_ = false; //床が変わったのでたどり着けるか調べる
		PlayerTurn turn_; //交互の手番
	};
}
