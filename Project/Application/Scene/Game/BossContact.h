#pragma once
#include <RyoEngine.h>

namespace App
{
	class Player;
	class Boss;
	class Floor;

	// プレイヤーがボスに触れたときのはじき (Game から分けたもの)
	// ・触れた瞬間だけはじく (ダメージはない)。2マス分、床のふちで止めない
	// ・跳んでぶつかった (体当たり): プレイヤーもボスも、おたがい反対へはじかれる
	// ・歩いて (立っていて) 触れた: プレイヤーだけはじかれる
	class BossContact
	{
	public:
		// 触れていない状態に戻す (配置し直したとき)
		void Reset() { isTouching_ = false; }

		// 触れたか調べて、はじく。体当たりでボスもはじいたら true (床の形が変わるので、たどり着けるかを調べ直す)
		bool Check(Player& player, Boss& boss, const Floor& floor, bool isJumpImpact);

	private:
		// プレイヤーを away の向きへはじく
		static void KnockPlayer(Player& player, const Floor& floor, const RyoEngine::Vector3& away);

		bool isTouching_ = false; // 前のフレームで触れていたか
	};
}
