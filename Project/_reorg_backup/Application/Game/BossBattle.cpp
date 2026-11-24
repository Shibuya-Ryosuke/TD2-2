#include "BossBattle.h"
#include "GameConfig.h"
#include "PlayerHealth.h"
#include "../Object/Boss.h"
#include "../Object/Player.h"
#include "../Stage/Floor.h"
#include "../Stage/FloorPathFinder.h"

using namespace RyoEngine;

namespace App
{
	// 何も起きていない状態に戻す
	void BossBattle::Reset()
	{
		contact_.Reset();
		separationNeedsCheck_ = false;
	}

	// ボスの足元が崩れたか調べる
	BossBattle::Event BossBattle::CheckBossFooting(const Floor& floor, Boss& boss, const BossTuning& tuning, bool isFloorChanged, const Cell& bossCell)
	{
		if (!isFloorChanged || boss.IsJumping() || !floor.IsHole(bossCell))
		{
			return Event::None;
		}
		// 跳んで逃げられなければ落ちる
		if (boss.TryEvadeCollapse(floor, tuning))
		{
			return Event::None;
		}
		return Event::BossFell;
	}

	// 戦闘中の1フレーム
	BossBattle::Event BossBattle::Update(Floor& floor, Player& player, Boss& boss, PlayerHealth& health, const BossTuning& tuning,
		bool isFloorChanged, const Cell& bossCell, bool isJumpImpact, float deltaTime)
	{
		// 床が変わったら、あとで「たどり着けるか」を調べ直す
		if (isFloorChanged)
		{
			separationNeedsCheck_ = true;
		}

		// ボスのいる島を切り離した
		if (isFloorChanged && !boss.IsJumping() && floor.TryIsolate(bossCell))
		{
			return Event::BossIsolated;
		}

		// ボス (床が減るほど速くなる)。攻撃が当たったらダメージ
		const float bossSpeed = tuning.moveSpeed * (1.0f + GameConfig::kMaxBossSpeedBonus * floor.GetCollapsedFraction());
		const bool wasVulnerable = !health.IsInvincible();
		health.Damage(boss.Tick(floor, player.GetPosition(), tuning, bossSpeed, deltaTime));
		if (health.IsDead())
		{
			return Event::PlayerDefeated;
		}

		// パンチが当たったら、はじかれる
		if (wasVulnerable && boss.PunchHitThisTick())
		{
			player.ApplyKnockback(floor, boss.GetPunchPushDirection(), tuning.punchPushSpeed, tuning.punchPushDuration, tuning.punchLiftSpeed, false);
		}

		// ボスがジャンプから着地した所が、穴・切り離された島なら落ちる
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

		// ボスとの体当たり → 分断のチェック
		CheckContact(floor, player, boss, isJumpImpact);
		if (IsSeparated(floor, player, boss))
		{
			return Event::Separated;
		}
		return Event::None;
	}

	// ボスに触れたときのはじき
	void BossBattle::CheckContact(const Floor& floor, Player& player, Boss& boss, bool isJumpImpact)
	{
		// 体当たりでボスもはじいたら、床の形が変わったのと同じなので、たどり着けるかを調べ直す
		if (contact_.Check(player, boss, floor, isJumpImpact))
		{
			separationNeedsCheck_ = true;
		}
	}

	// おたがいにたどり着けなくなったか
	bool BossBattle::IsSeparated(const Floor& floor, const Player& player, const Boss& boss)
	{
		if (!separationNeedsCheck_ || !player.IsGrounded() || boss.IsJumping())
		{
			return false;
		}

		// 2人とも床の上に立っているときだけ調べる
		const Cell playerCell = floor.GetCellAt(player.GetPosition());
		const Cell bossCell = floor.GetCellAt(boss.GetPosition());
		if (!floor.IsSolid(playerCell) || !floor.IsSolid(bossCell))
		{
			return false;
		}
		separationNeedsCheck_ = false;

		// プレイヤーは穴を1つ飛び越えられるので、飛び越えあり で調べる (2マス乗り越えのアイテムは入れない)
		return !FloorPathFinder::CanReach(floor, bossCell, playerCell, true);
	}
}
