#include "BossBattle.h"
#include "../../Config/GameConfig.h"
#include "../../Object/Player/PlayerHealth.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Player/Player.h"
#include "../../Stage/Floor/Floor.h"
#include "../../Stage/Grid/FloorPathFinder.h"

using namespace RyoEngine;

namespace App
{
	//何も起きていない状態に戻す
	void BossBattle::Reset()
	{
		contact_.Reset();
		separationNeedsCheck_ = false;
		turn_.Reset();
	}

	//ボスの足元が崩れたか調べる
	BossBattle::Event BossBattle::CheckBossFooting(const Floor& floor, const Player& player, Boss& boss, const BossTuning& tuning, bool isFloorChanged, const Cell& bossCell)
	{
		//攻撃中に足元が崩れそうならやめて逃げることにする
		boss.TryInterruptUnsafeFooting(floor, tuning);
		if (!isFloorChanged || boss.IsJumping() || !floor.IsHole(bossCell))
		{
			return Event::None;
		}
		//ボスの番に跳んで逃げられれば1回動いたことにして、だめなら落ちる
		if (boss.TryEvadeCollapse(floor, tuning, turn_.CanBossAct(player)))
		{
			turn_.OnBossAction();
			return Event::None;
		}
		return Event::BossFell;
	}

	//戦闘中の1フレーム
	BossBattle::Event BossBattle::Update(Floor& floor, Player& player, Boss& boss, PlayerHealth& health, const BossTuning& tuning,
		bool isFloorChanged, const Cell& bossCell, bool isJumpImpact, float deltaTime)
	{
		//床が変わったらあとでたどり着けるか調べ直す
		if (isFloorChanged)
		{
			separationNeedsCheck_ = true;
		}

		//ボスのいる島を切り離した
		if (isFloorChanged && !boss.IsJumping() && floor.TryIsolate(bossCell))
		{
			return Event::BossIsolated;
		}

		//床が減るほど速くなるボスを動かして、攻撃が当たったらダメージ
		const float bossSpeed = tuning.moveSpeed * (1.0f + GameConfig::kMaxBossSpeedBonus * floor.GetCollapsedFraction());
		const bool wasVulnerable = !health.IsInvincible();
		health.Damage(boss.Tick(floor, player.GetPosition(), tuning, bossSpeed, deltaTime, turn_.CanBossAct(player)));
		if (boss.ActionStartedThisTick())
		{
			turn_.OnBossAction();
		}
		if (health.IsDead())
		{
			return Event::PlayerDefeated;
		}

		//パンチが当たったらはじかれる
		if (wasVulnerable && boss.PunchHitThisTick())
		{
			player.ApplyKnockback(floor, boss.GetPunchPushDirection(), tuning.punchPushSpeed, tuning.punchPushDuration, tuning.punchLiftSpeed, false);
		}

		//ボスの着地した所が穴か切り離された島なら落ちる
		if (boss.LandedThisTick())
		{
			const Cell landedCell = floor.GetCellAt(boss.GetPosition());
			if (floor.IsHole(landedCell))
			{
				return Event::BossFell;
			}
			if (floor.TryIsolate(landedCell))
			{
				return Event::BossIsolated;
			}
		}

		//体当たりを調べてから分断を調べる
		CheckContact(floor, player, boss, isJumpImpact);
		if (IsSeparated(floor, player, boss))
		{
			return Event::Separated;
		}
		return Event::None;
	}

	//ボスに触れたときのはじき
	void BossBattle::CheckContact(const Floor& floor, Player& player, Boss& boss, bool isJumpImpact)
	{
		//体当たりでボスもはじいたらたどり着けるか調べ直す
		if (contact_.Check(player, boss, floor, isJumpImpact))
		{
			separationNeedsCheck_ = true;
		}
	}

	//おたがいにたどり着けなくなったか
	bool BossBattle::IsSeparated(const Floor& floor, const Player& player, const Boss& boss)
	{
		if (!separationNeedsCheck_ || !player.IsGrounded() || boss.IsJumping())
		{
			return false;
		}

		//2人とも床の上に立っているときだけ調べる
		const Cell playerCell = floor.GetCellAt(player.GetPosition());
		const Cell bossCell = floor.GetCellAt(boss.GetPosition());
		if (!floor.IsSolid(playerCell) || !floor.IsSolid(bossCell))
		{
			return false;
		}
		separationNeedsCheck_ = false;

		//プレイヤーは穴を1つ飛び越えられるので飛び越えありで調べる
		return !FloorPathFinder::CanReach(floor, bossCell, playerCell, true);
	}
}
