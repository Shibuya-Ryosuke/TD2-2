#pragma once
#include "BossContact.h"
#include "BossTuning.h"
#include "../Stage/Cell.h"

namespace App
{
	class Floor;
	class Player;
	class Boss;
	class PlayerHealth;

	// 戦闘中のボスとのやりとり (Game から分けたもの)
	// ・ボスの足元が崩れた → 跳んで逃げる / 逃げられなければ落ちる
	// ・ボスの島を切り離した → 揺れ始める
	// ・ボスを動かす (攻撃が当たったらダメージ・パンチではじかれる)
	// ・ボスに触れたときのはじき (BossContact)
	// ・おたがいにたどり着けなくなった → 床を元に戻す
	// 場面を変えることは Game がするので、ここでは「何が起きたか」を返すだけ
	class BossBattle
	{
	public:
		// このフレームで起きたこと
		enum class Event
		{
			None,          // 何もない (戦闘を続ける)
			BossIsolated,  // ボスの島が切り離された (揺れ始める)
			BossFell,      // ボスが穴に落ちた (下の層へ)
			Separated,     // おたがいにたどり着けなくなった (床を元に戻す)
			PlayerDefeated // プレイヤーの HP が 0 になった
		};

		// 何も起きていない状態に戻す (配置し直したとき)
		void Reset();
		// 「たどり着けるか」を調べなくてよくする (戦闘開始待ちになったとき)
		void ClearSeparationCheck() { separationNeedsCheck_ = false; }

		// ボスの足元が崩れたか調べる。逃げられずに落ちたら BossFell
		Event CheckBossFooting(const Floor& floor, Boss& boss, const BossTuning& tuning, bool isFloorChanged, const Cell& bossCell);

		// 戦闘中の1フレーム (ボス・当たり・切り離し・分断)
		// isJumpImpact: プレイヤーが跳んでいる途中・跳び終わった瞬間か (体当たりになる)
		Event Update(Floor& floor, Player& player, Boss& boss, PlayerHealth& health, const BossTuning& tuning,
			bool isFloorChanged, const Cell& bossCell, bool isJumpImpact, float deltaTime);

		// ボスに触れたときのはじきだけ調べる (揺れている間は isJumpImpact = false)
		void CheckContact(const Floor& floor, Player& player, Boss& boss, bool isJumpImpact);

	private:
		// おたがいにたどり着けなくなったか (2人とも床の上に立っているときだけ調べる)
		bool IsSeparated(const Floor& floor, const Player& player, const Boss& boss);

		BossContact contact_;               // ボスに触れたときのはじき
		bool separationNeedsCheck_ = false; // 床が変わったので「おたがいにたどり着けるか」を調べる
	};
}
