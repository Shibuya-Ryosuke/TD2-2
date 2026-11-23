#include "Boss.h"
#include "BossDetail.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;
using namespace App::BossDetail;

//ボスの攻撃を始めてパンチを進める
namespace App
{
	//近接かパンチの攻撃を始める
	bool Boss::StartAttack(const Floor& floor, const Vector3& target, const BossTuning& tuning, BossActionKind kind)
	{
		const Cell cell = floor.GetCellAt(position_);
		if (!floor.IsSolid(cell) || floor.IsIsolated(cell))
		{
			return false;
		}
		Stop();
		cell_ = cell;
		next_ = cell;

		//プレイヤーの方を向く
		Vector3 forward = target - position_;
		forward.y = 0.0f;
		if (LengthSquared(forward) < 0.0001f)
		{
			forward = { 0.0f, 0.0f, 1.0f };
		}
		LookAt(forward);

		//パンチは前方に予告を出して拳を引く
		if (kind == BossActionKind::Punch)
		{
			punchDirection_ = Normalize(forward);
			punchTimer_ = 0.0f;
			punchPhase_ = PunchPhase::Windup;
			punchTellRange_ = tuning.punchRange;
			punchTellWidth_ = tuning.punchWidth;
			isPunchTellVisible_ = true;
			return true;
		}

		//近接攻撃は足元に予告を出す
		isWindingUp_ = true;
		windup_ = tuning.meleeWindup;
		meleeTellSize_ = tuning.meleeRange * 2.0f;
		isMeleeTellVisible_ = true;
		return true;
	}

	int32_t Boss::AdvancePunch(const Floor& floor, const Vector3& target, const BossTuning& tuning, float deltaTime)
	{
		//拳を引いた所と伸ばしきった所
		punchTimer_ += deltaTime;
		const Vector3 pulledBack = kFistRest + Vector3{ 0.0f, 0.0f, -kFistPullBack };
		const Vector3 extended = { 0.0f, kFistRest.y, tuning.punchRange };

		//予告で拳を後ろに引く
		if (punchPhase_ == PunchPhase::Windup)
		{
			const float t = std::clamp(punchTimer_ / tuning.punchWindup, 0.0f, 1.0f);
			fistLocal_ = kFistRest + Vector3{ 0.0f, 0.0f, -kFistPullBack * t };
			if (t < 1.0f)
			{
				return 0;
			}
			punchPhase_ = PunchPhase::Strike;
			punchTimer_ = 0.0f;
			isPunchResolved_ = false;
			isPunchTellVisible_ = false;
			return 0;
		}

		//拳が伸びて、半分まで伸びたら当たり判定
		if (punchPhase_ == PunchPhase::Strike)
		{
			const float s = std::clamp(punchTimer_ / tuning.punchStrikeDuration, 0.0f, 1.0f);
			fistLocal_ = Lerp(pulledBack, extended, s);
			if (s >= 1.0f)
			{
				punchPhase_ = PunchPhase::Recovery;
				punchTimer_ = 0.0f;
			}
			if (isPunchResolved_ || s < 0.5f)
			{
				return 0;
			}
			isPunchResolved_ = true;

			//前方の長方形の中にいるか
			const Vector3 relative = target - position_;
			const float forwardDistance = Dot(relative, punchDirection_);
			const float sideDistance = Dot(relative, Cross(Vector3{ 0.0f, 1.0f, 0.0f }, punchDirection_));
			if (forwardDistance < 0.0f || forwardDistance > tuning.punchRange)
			{
				return 0;
			}
			if (std::abs(sideDistance) > tuning.punchWidth * 0.5f)
			{
				return 0;
			}
			if (std::abs(target.y - floor.GetHeight()) > tuning.meleeHeight)
			{
				return 0;
			}

			//床の真ん中からプレイヤーへの向きにはじく
			Vector3 push = target - Vector3{ 0.0f, floor.GetHeight(), 0.0f };
			push.y = 0.0f;
			if (LengthSquared(push) > 0.001f)
			{
				punchPushDirection_ = Normalize(push);
			}
			else
			{
				punchPushDirection_ = punchDirection_;
			}
			punchHitThisTick_ = true;
			return tuning.damage;
		}

		//拳が戻る
		if (punchPhase_ == PunchPhase::Recovery)
		{
			const float r = std::clamp(punchTimer_ / tuning.punchRecovery, 0.0f, 1.0f);
			fistLocal_ = Lerp(extended, kFistRest, r);
			if (r < 1.0f)
			{
				return 0;
			}
			CancelPunch();
		}
		return 0;
	}

	//パンチをやめて拳を戻す
	void Boss::CancelPunch()
	{
		punchPhase_ = PunchPhase::None;
		punchTimer_ = 0.0f;
		isPunchTellVisible_ = false;
		fistLocal_ = kFistRest;
	}
}
