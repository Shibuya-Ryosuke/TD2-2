#pragma once
#include <RyoEngine.h>

namespace App
{
	class Player;
	class Boss;
	class Floor;

	//ボスに触れた瞬間にダメージなしで2マスはじく
	class BossContact
	{
	public:
		//配置し直したときに触れていない状態に戻す
		void Reset() { isTouching_ = false; }

		//触れたか調べてはじき、体当たりでボスもはじいたらtrue
		bool Check(Player& player, Boss& boss, const Floor& floor, bool isJumpImpact);

	private:
		//プレイヤーをawayの向きへはじく
		static void KnockPlayer(Player& player, const Floor& floor, const RyoEngine::Vector3& away);

		bool isTouching_ = false; //前のフレームで触れていたか
	};
}
