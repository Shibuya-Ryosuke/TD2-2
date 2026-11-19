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
	// 触れたか調べて、はじく
	bool BossContact::Check(Player& player, Boss& boss, const Floor& floor, bool isJumpImpact)
	{
		// ボスの体 (箱) とプレイヤーの体 (円柱) が重なったか。ボスの向きに合わせて調べる
		const Vector3 local = boss.ToLocal(player.GetPosition());
		const float gapX = std::max(0.0f, std::abs(local.x) - GameConfig::kBossContactSizeX * 0.5f);
		const float gapZ = std::max(0.0f, std::abs(local.z) - GameConfig::kBossContactSizeZ * 0.5f);
		const float radius = GameConfig::kPlayerContactRadius;
		bool isTouching = false;
		if (local.y <= GameConfig::kBossContactSizeY && local.y + GameConfig::kPlayerContactHeight >= 0.0f && gapX * gapX + gapZ * gapZ <= radius * radius)
		{
			isTouching = true;
		}

		// 触れた瞬間だけはじく (ダメージはない)
		const bool isNewTouch = isTouching && !isTouching_;
		isTouching_ = isTouching;
		if (!isNewTouch || player.IsKnockedBack())
		{
			return false;
		}

		// ボスからプレイヤーへの向き (重なりすぎて向きが取れないときは、あとで別の向きを使う)
		Vector3 away = player.GetPosition() - boss.GetPosition();
		away.y = 0.0f;
		const bool isTooClose = away.x * away.x + away.z * away.z < 0.0001f;

		// 跳んでぶつかった: プレイヤーもボスも、おたがい反対へはじかれる
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

		// 歩いて (立っていて) 触れた: プレイヤーだけはじかれる
		if (isTooClose)
		{
			away = boss.GetForward();
		}
		KnockPlayer(player, floor, away);
		return false;
	}

	// プレイヤーを away の向きへはじく (2マス分。床のふちで止めない)
	void BossContact::KnockPlayer(Player& player, const Floor& floor, const Vector3& away)
	{
		// kContactKnockbackCells マス分を、kContactPushDuration 秒で進む速さ
		const Vector3 offset = floor.StepOffset(GridDirection::FromVector(away));
		const float speed = Length(offset) * GameConfig::kContactKnockbackCells / GameConfig::kContactPushDuration;
		player.ApplyKnockback(floor, Normalize(offset), speed, GameConfig::kContactPushDuration, GameConfig::kContactLiftSpeed, true);
	}
}
