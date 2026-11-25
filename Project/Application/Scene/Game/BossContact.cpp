#include "BossContact.h"
#include "../../Config/GameConfig.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Player/Player.h"
#include "../../Object/Player/PlayerHealth.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//重なっていたら無敵でないときだけハートを減らす
	void BossContact::Check(const Player& player, const Boss& boss, PlayerHealth& health)
	{
		if (IsTouching(player, boss))
		{
			health.Damage(GameConfig::kContactDamage);
		}
	}

	//ボスの向きに合わせて箱と円柱が重なったか調べる
	bool BossContact::IsTouching(const Player& player, const Boss& boss)
	{
		const Vector3 local = boss.ToLocal(player.GetPosition());
		const float gapX = std::max(0.0f, std::abs(local.x) - GameConfig::kBossContactSizeX * 0.5f);
		const float gapZ = std::max(0.0f, std::abs(local.z) - GameConfig::kBossContactSizeZ * 0.5f);
		const float radius = GameConfig::kPlayerContactRadius;
		if (local.y > GameConfig::kBossContactSizeY || local.y + GameConfig::kPlayerContactHeight < 0.0f)
		{
			return false;
		}
		return gapX * gapX + gapZ * gapZ <= radius * radius;
	}
}
