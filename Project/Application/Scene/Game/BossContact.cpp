#include "BossContact.h"
#include "../../Config/GameConfig.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Player/Player.h"
#include "../../Stage/Floor/Floor.h"
#include "../../Stage/Grid/GridDirection.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//触れたか調べてはじく
	bool BossContact::Check(Player& player, Boss& boss, const Floor& floor, bool isJumpImpact)
	{
		//ボスの向きに合わせて箱と円柱が重なったか調べる
		const Vector3 local = boss.ToLocal(player.GetPosition());
		const float gapX = std::max(0.0f, std::abs(local.x) - GameConfig::kBossContactSizeX * 0.5f);
		const float gapZ = std::max(0.0f, std::abs(local.z) - GameConfig::kBossContactSizeZ * 0.5f);
		const float radius = GameConfig::kPlayerContactRadius;
		bool isTouching = false;
		if (local.y <= GameConfig::kBossContactSizeY && local.y + GameConfig::kPlayerContactHeight >= 0.0f && gapX * gapX + gapZ * gapZ <= radius * radius)
		{
			isTouching = true;
		}

		//触れた瞬間だけダメージなしではじく
		const bool isNewTouch = isTouching && !isTouching_;
		isTouching_ = isTouching;
		if (!isNewTouch || player.IsKnockedBack())
		{
			return false;
		}

		//ボスからプレイヤーへの向き
		Vector3 away = player.GetPosition() - boss.GetPosition();
		away.y = 0.0f;
		const bool isTooClose = away.x * away.x + away.z * away.z < 0.0001f;

		//跳んでぶつかったらおたがい反対へはじかれる
		if (isJumpImpact)
		{
			if (isTooClose)
			{
				away = player.GetJumpOrigin() - floor.GetCellCenter(player.GetJumpDestination());
			}
			const Cell direction = GridDirection::FromVector(away);
			KnockPlayer(player, floor, away);
			boss.KnockBack(floor, -direction, GameConfig::kContactPushDuration);
			return true;
		}

		//立っていて触れたらプレイヤーだけはじかれる
		if (isTooClose)
		{
			away = boss.GetForward();
		}
		KnockPlayer(player, floor, away);
		return false;
	}

	//プレイヤーを床のふちで止めずに2マスはじく
	void BossContact::KnockPlayer(Player& player, const Floor& floor, const Vector3& away)
	{
		//決めた秒数で決めたマス数を進む速さ
		const Vector3 offset = floor.StepOffset(GridDirection::FromVector(away));
		const float speed = Length(offset) * GameConfig::kContactKnockbackCells / GameConfig::kContactPushDuration;
		player.ApplyKnockback(floor, Normalize(offset), speed, GameConfig::kContactPushDuration, GameConfig::kContactLiftSpeed, true);
	}
}
