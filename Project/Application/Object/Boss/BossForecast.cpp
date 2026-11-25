#include "Boss.h"
#include "BossPlanner.h"
#include <algorithm>

using namespace RyoEngine;

//やりかけの行動のあとをBossPlannerで先読みするボスの先読み
namespace App
{
	//次のcount回の行動を先読みする
	void Boss::BuildForecast(const Floor& floor, const Vector3& player, const BossTuning& tuning, float speed, float stepTime,
		std::vector<BossPlannedAction>& result, int32_t count) const
	{
		count = std::clamp(count, 1, 10);
		result.clear();
		Vector3 from = position_;
		float elapsed = 0.0f;

		//やりかけの行動が1つ目で、次はstepTime秒待ったあと
		BossPlannedAction active{};
		if (DescribeActiveAction(floor, tuning, active))
		{
			result.push_back(active);
			from = active.to;
			elapsed = active.duration + stepTime;
		}
		BossPlanner::Forecast(result, floor, from, player, tuning, speed, nextPatternTurn_, count - static_cast<int32_t>(result.size()),
			stepTime, elapsed, isEvacuating_, floor.GetCellCenter(evacuationDestination_));
	}

	//やりかけの行動を先読みの1つ目にする
	bool Boss::DescribeActiveAction(const Floor& floor, const BossTuning& tuning, BossPlannedAction& action) const
	{
		if (!IsBusy())
		{
			return false;
		}
		action = BossPlannedAction{};
		action.from = position_;
		action.to = position_;
		action.target = position_ + GetForward();

		//パンチの残りの秒数
		if (punchPhase_ != PunchPhase::None)
		{
			action.kind = BossActionKind::Punch;
			action.windup = tuning.punchWindup;
			action.target = position_ + punchDirection_ * tuning.punchRange;
			if (punchPhase_ == PunchPhase::Windup)
			{
				action.duration = std::max(0.0f, tuning.punchWindup - punchTimer_) + tuning.punchStrikeDuration + tuning.punchRecovery;
			}
			else if (punchPhase_ == PunchPhase::Strike)
			{
				action.duration = std::max(0.0f, tuning.punchStrikeDuration - punchTimer_) + tuning.punchRecovery;
			}
			else
			{
				action.duration = std::max(0.0f, tuning.punchRecovery - punchTimer_);
			}
			return true;
		}

		//近接攻撃の予告中
		if (isWindingUp_)
		{
			action.kind = BossActionKind::Melee;
			action.duration = std::max(0.0f, windup_);
			return true;
		}

		//跳んでいる途中は着地先へ
		action.kind = BossActionKind::Move;
		action.isEvading = isEvacuating_;
		action.to = floor.GetCellCenter(next_);
		action.duration = std::max(0.05f, jumpDuration_ - jumpElapsed_);
		return true;
	}
}
