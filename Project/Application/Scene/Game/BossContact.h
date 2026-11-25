#pragma once

namespace App
{
	class Player;
	class Boss;
	class PlayerHealth;

	//ボスに触れている間ハートを減らし、はじきはしないボスとの当たり
	class BossContact
	{
	public:
		//ボスの体とプレイヤーの体が重なっていたらハートを減らす
		static void Check(const Player& player, const Boss& boss, PlayerHealth& health);

	private:
		//ボスの向きに合わせた箱とプレイヤーの円柱が重なっているか
		static bool IsTouching(const Player& player, const Boss& boss);
	};
}
