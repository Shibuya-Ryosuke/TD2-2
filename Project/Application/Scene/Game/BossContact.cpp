#include "BossContact.h"
#include "../../Config/BossConfig.h"
#include "../../Config/PlayerConfig.h"
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
			health.Damage(BossConfig::kContactDamage);
		}
	}

	//ボスの向きに合わせて箱と円柱が重なったか調べる
	bool BossContact::IsTouching(const Player& player, const Boss& boss)
	{
		const Vector3 local = boss.ToLocal(player.GetPosition());
		const float gapX = std::max(0.0f, std::abs(local.x) - BossConfig::kBossContactSizeX * 0.5f);
		const float gapZ = std::max(0.0f, std::abs(local.z) - BossConfig::kBossContactSizeZ * 0.5f);
		const float radius = PlayerConfig::kPlayerContactRadius;
		if (local.y > BossConfig::kBossContactSizeY || local.y + PlayerConfig::kPlayerContactHeight < 0.0f)
		{
			return false;
		}
		return gapX * gapX + gapZ * gapZ <= radius * radius;
	}
}
