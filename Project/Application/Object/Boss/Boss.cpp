#include "Boss.h"
#include "../../Config/GameConfig.h"
#include "../../Stage/Grid/FloorPathFinder.h"
#include "BossPlanner.h"
#include "BossDetail.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace RyoEngine;
using namespace App::BossDetail;

namespace App
{
	void Boss::Initialize()
	{
		//体と拳と攻撃の予告のモデルはBossViewが作る
		view_.Initialize();
		fistLocal_ = kFistRest;
	}

	//攻撃を全部やめて床の上に置く
	void Boss::Place(const Floor& floor, const Vector3& position)
	{
		Stop();
		cell_ = floor.GetCellAt(position);
		next_ = cell_;
		position_ = position;
		yawDeg_ = 0.0f;
		nextPatternTurn_ = 0;
	}

	//攻撃とジャンプをやめる
	void Boss::Stop()
	{
		isEvacuating_ = false;
		isWindingUp_ = false;
		isJumping_ = false;
		CancelPunch();
		isMeleeTellVisible_ = false;
	}

	//足元が崩れたらとなりの床へ跳んで逃げる
	bool Boss::TryEvadeCollapse(const Floor& floor, const BossTuning& tuning)
	{
		if (isEvacuating_ || !tuning.canJump || isJumping_)
		{
			return false;
		}
		Cell landing{};
		if (!FloorPathFinder::FindLandingNeighbor(floor, position_, tuning.jumpDuration + tuning.landingSafetyMargin, landing))
		{
			return false;
		}
		StartJump(floor, landing, tuning.jumpDuration);
		++nextPatternTurn_;
		return true;
	}

	//攻撃中に足元が崩れそうなら攻撃をやめて逃げることにする
	bool Boss::TryInterruptUnsafeFooting(const Floor& floor, const BossTuning& tuning)
	{
		if (isJumping_ || !IsBusy())
		{
			return false;
		}
		Vector3 destination{};
		if (!BossPlanner::TryFindSafetyStep(floor, position_, tuning.moveSpeed, 0.0f, tuning.landingSafetyMargin, destination))
		{
			return false;
		}
		Stop();
		isEvacuating_ = true;
		evacuationDestination_ = floor.GetCellAt(destination);
		return true;
	}

	//跳んでいるか攻撃中か
	bool Boss::IsBusy() const
	{
		return isJumping_ || isWindingUp_ || punchPhase_ != PunchPhase::None;
	}

	int32_t Boss::Tick(const Floor& floor, const Vector3& target, const BossTuning& tuning, float moveSpeed, float deltaTime)
	{
		//このフレームの結果を消す
		landedThisTick_ = false;
		punchHitThisTick_ = false;

		//攻撃中に足元が崩れそうならやめて逃げることにする
		TryInterruptUnsafeFooting(floor, tuning);

		//やりかけのパンチとジャンプはそれだけ進める
		if (punchPhase_ != PunchPhase::None)
		{
			return AdvancePunch(floor, target, tuning, deltaTime);
		}
		if (isJumping_)
		{
			AdvanceJump(floor, tuning, deltaTime);
			return 0;
		}

		//近接攻撃の予告が終わったら当てる
		if (isWindingUp_)
		{
			windup_ -= deltaTime;
			if (windup_ > 0.0f)
			{
				return 0;
			}
			isWindingUp_ = false;
			isMeleeTellVisible_ = false;
			Vector3 toTarget = target - position_;
			toTarget.y = 0.0f;
			if (std::sqrt(LengthSquared(toTarget)) <= tuning.meleeRange && std::abs(target.y - floor.GetHeight()) <= tuning.meleeHeight)
			{
				return tuning.damage;
			}
			return 0;
		}

		//逃げ先に着いた
		if (isEvacuating_ && floor.GetCellAt(position_) == evacuationDestination_)
		{
			isEvacuating_ = false;
		}

		//BossPlannerで何をするか決める
		const BossPlannedAction action = BossPlanner::Choose(floor, position_, target, tuning, moveSpeed, nextPatternTurn_, 0.0f,
			isEvacuating_, floor.GetCellCenter(evacuationDestination_));
		if (action.kind == BossActionKind::Fall)
		{
			return 0; //落ちるのはGameが見る
		}
		if (action.kind == BossActionKind::Move)
		{
			//1マス跳び、逃げるときは逃げ先を覚えておく
			if (!action.isBlocked)
			{
				if (action.isEvading)
				{
					isEvacuating_ = true;
					evacuationDestination_ = floor.GetCellAt(action.to);
				}
				StartJump(floor, floor.GetCellAt(action.to), action.duration, isEvacuating_);
				AdvanceJump(floor, tuning, deltaTime);
			}
		}
		else if (!StartAttack(floor, target, tuning, action.kind))
		{
			return 0;
		}

		//1回動いた
		++nextPatternTurn_;
		return 0;
	}

	//その向きを向く
	void Boss::LookAt(const Vector3& direction)
	{
		yawDeg_ = std::atan2(direction.x, direction.z) * 180.0f / std::numbers::pi_v<float>;
	}

	//前の向き
	Vector3 Boss::GetForward() const
	{
		return RotateY({ 0.0f, 0.0f, 1.0f }, yawDeg_);
	}

	//ボスから見た位置をワールドの位置にする
	Vector3 Boss::ToWorld(const Vector3& local) const
	{
		return position_ + RotateY(local, yawDeg_);
	}

	//ワールドの位置をボスから見た位置にする
	Vector3 Boss::ToLocal(const Vector3& world) const
	{
		return RotateY(world - position_, -yawDeg_);
	}

	void Boss::StartJump(const Floor& floor, const Cell& landing, float duration, bool isEvacuation)
	{
		//逃げるためのジャンプでなければ逃げるのはやめる
		if (!isEvacuation)
		{
			isEvacuating_ = false;
		}
		//攻撃をやめて行き先と時間と始まりの位置を決める
		CancelPunch();
		isWindingUp_ = false;
		isMeleeTellVisible_ = false;
		isJumping_ = true;
		next_ = landing;
		jumpElapsed_ = 0.0f;
		jumpDuration_ = duration;
		jumpOrigin_ = position_;
		//着地先の方を向く
		const Vector3 toLanding = floor.GetCellCenter(landing) - jumpOrigin_;
		if (LengthSquared(Vector3{ toLanding.x, 0.0f, toLanding.z }) > 0.0001f)
		{
			LookAt(toLanding);
		}
	}

	//跳んでいる途中を進める
	void Boss::AdvanceJump(const Floor& floor, const BossTuning& tuning, float deltaTime)
	{
		jumpElapsed_ += deltaTime;
		//0から1の進み具合
		float t = 1.0f;
		if (jumpDuration_ > 0.0f)
		{
			t = std::clamp(jumpElapsed_ / jumpDuration_, 0.0f, 1.0f);
		}
		//マスの線にそって山なりに進む
		Vector3 position = Floor::AlongGrid(jumpOrigin_, floor.GetCellCenter(next_), t);
		position.y += 4.0f * tuning.jumpHeight * t * (1.0f - t);
		position_ = position;
		//上下に動かず歩くときは穴の上に来たらそこで止まって落ちる
		if (tuning.jumpHeight <= 0.0f && !floor.IsSolid(floor.GetCellAt(position)))
		{
			next_ = floor.GetCellAt(position);
			cell_ = next_;
			isJumping_ = false;
			landedThisTick_ = true;
			return;
		}
		if (t < 1.0f)
		{
			return;
		}
		//着いた
		cell_ = next_;
		isJumping_ = false;
		landedThisTick_ = true;
	}

	void Boss::UpdateVisuals()
	{
		view_.Update(*this);
	}

	//体と拳と攻撃の予告を描く
	void Boss::Draw()
	{
		view_.Draw(*this);
	}
}
