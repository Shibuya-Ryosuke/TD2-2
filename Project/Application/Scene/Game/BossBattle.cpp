#include "BossBattle.h"
#include "BossContact.h"
#include "../../Config/GameConfig.h"
#include "../../Object/Player/PlayerHealth.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Player/Player.h"
#include "../../Stage/Floor/Floor.h"
#include "../../Stage/Grid/FloorPathFinder.h"

using namespace RyoEngine;

namespace App
{
	//ボスの足元が崩れたか調べる
	BossBattle::Event BossBattle::CheckBossFooting(const Floor& floor, Boss& boss, const BossTuning& tuning, bool isFloorChanged, const Cell& bossCell)
	{
		//攻撃中に足元が崩れそうならやめて逃げることにする
		boss.TryInterruptUnsafeFooting(floor, tuning);
		if (!isFloorChanged || boss.IsJumping() || !floor.IsHole(bossCell))
		{
			return Event::None;
		}
		//となりの床へ跳んで逃げられなければ落ちる
		if (boss.TryEvadeCollapse(floor, tuning))
		{
			return Event::None;
		}
		return Event::BossFell;
	}

	//戦闘中の1フレーム
	BossBattle::Event BossBattle::Update(Floor& floor, Player& player, Boss& boss, PlayerHealth& health, const BossTuning& tuning,
		bool isFloorChanged, const Cell& bossCell, float deltaTime)
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

		//床が減るほど速くなるボスをプレイヤーと同時に動かして、攻撃が当たったらダメージ
		const float bossSpeed = tuning.moveSpeed * (1.0f + GameConfig::kMaxBossSpeedBonus * floor.GetCollapsedFraction());
		const bool wasVulnerable = !health.IsInvincible();
		health.Damage(boss.Tick(floor, player.GetPosition(), tuning, bossSpeed, deltaTime));
		if (health.IsDead())
		{
			return Event::PlayerDefeated;
		}

		//パンチが当たったらはじかれる
		if (wasVulnerable && boss.PunchHitThisTick())
		{
			player.ApplyKnockback(floor, boss.GetPunchPushDirection(), tuning.punchPushSpeed, tuning.punchPushDuration, tuning.punchLiftSpeed, false);
		}

		//地面パンチが地面を叩いたら、歩数だけ遅らせて盤面を跳ねさせる
		const GroundPunch& groundPunch = boss.GetGroundPunch();
		if (groundPunch.IsImpactThisTick()) {
			floor.Bounce(groundPunch.GetSteps(), groundPunch.GetBounceHeight(), groundPunch.GetWaveStepDelay(),
				groundPunch.GetBounceSpeed(), groundPunch.GetBounceDamping());
		}
		//地面パンチが当たったら、外へ向かってマスの数だけ吹っ飛ばす
		if (wasVulnerable && groundPunch.IsHitThisTick()) {
			const Cell& direction = groundPunch.GetKnockbackDirection();
			const Vector3 push = { static_cast<float>(direction.x), 0.0f, static_cast<float>(direction.z) };
			const float tileSize = (direction.x != 0) ? floor.GetTileSizeX() : floor.GetTileSizeZ();
			const float distance = tileSize * static_cast<float>(groundPunch.GetKnockbackCells());
			player.ApplyKnockback(floor, push, distance / tuning.punchPushDuration, tuning.punchPushDuration, tuning.punchLiftSpeed, false);
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

		//ボスに触れていたらハートが減る
		BossContact::Check(player, boss, health);
		if (health.IsDead())
		{
			return Event::PlayerDefeated;
		}
		if (IsSeparated(floor, player, boss, tuning))
		{
			return Event::Separated;
		}
		return Event::None;
	}

	//おたがいにたどり着けなくなったか
	bool BossBattle::IsSeparated(const Floor& floor, const Player& player, const Boss& boss, const BossTuning& tuning)
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

		//プレイヤーは穴を飛び越えられないので、ボスが穴を越えて来られるかで調べる
		bool canCrossGaps = false;
		if (tuning.canJump && tuning.moveSpeed > 0.0f)
		{
			canCrossGaps = true;
		}
		return !FloorPathFinder::CanReach(floor, bossCell, playerCell, canCrossGaps);
	}
}
