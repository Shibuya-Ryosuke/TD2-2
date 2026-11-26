#include "TutorialFlow.h"
#include "../Game/PlayerAction.h"
#include "../../Config/FloorConfig.h"
#include "../../Config/LayerConfig.h"
#include "../../Config/TutorialConfig.h"
#include "../../Direction/GameDirection.h"
#include "../../Object/Player/Player.h"
#include "../../Stage/Floor/LayerFloors.h"
#include <algorithm>

using namespace RyoEngine;

//TutorialFlowのうち段階ごとの進み具合を見るところ
namespace App
{
	//段階ごとの進み具合を見る
	void TutorialFlow::UpdateStep(const Context& context, bool isFloorChanged)
	{
		if (hasPendingStep_)
		{
			return;
		}
		if (step_ == Step::Move)
		{
			movedCount_ = context.player.GetJumpCount() - jumpStart_;
			if (movedCount_ >= TutorialConfig::kMoveCount)
			{
				Clear(context, Step::Break);
			}
		}
		else if (step_ == Step::Break)
		{
			if (context.action.WasBreakSucceeded())
			{
				++breakCount_;
			}
			if (breakCount_ >= TutorialConfig::kBreakCount)
			{
				Clear(context, Step::BlockOneFall);
			}
		}
		else if (step_ == Step::BlockOneFall || step_ == Step::BlockTwoFall)
		{
			//降りた所が崩れて積み木が消えたら降らせ直す
			if (block_.IsGone())
			{
				DropBlock(context, ToBlockCode(step_));
				return;
			}
			//積み木が降りてカメラが戻ったら落とすか切り離すやることへ
			if (block_.IsResting() && !block_.IsCinematic())
			{
				if (step_ == Step::BlockOneFall)
				{
					ChangeStep(context, Step::BlockOneDrop);
					return;
				}
				ChangeStep(context, Step::BlockTwoCut);
			}
		}
		else if (step_ == Step::BlockOneDrop)
		{
			if (block_.IsSinking())
			{
				Clear(context, Step::Reshape);
			}
		}
		else if (step_ == Step::Reshape)
		{
			UpdateReshape(context);
		}
		else if (step_ == Step::BlockTwoCut)
		{
			//積み木のマスを直接壊したら、島を戻して降らせ直す
			Floor& floor = context.floors.GetCurrent();
			if (block_.IsGone())
			{
				ChangeStep(context, Step::BlockTwoFall);
				return;
			}
			if (isFloorChanged && !block_.IsSinking() && floor.TryIsolate(block_.GetCell()))
			{
				ChangeStep(context, Step::IslandFall);
			}
		}
		else if (step_ == Step::IslandFall)
		{
			UpdateIslandFall(context);
		}
	}

	//最初の位置に戻ったあと少し待って、切り離しやすく床を崩す
	void TutorialFlow::UpdateReshape(const Context& context)
	{
		Floor& floor = context.floors.GetCurrent();

		//少し待ってから、島をつなぐマス以外のまわりを崩す
		if (!isWallScheduled_)
		{
			if (stepTimer_ >= TutorialConfig::kReshapeDelay)
			{
				map_.RestoreIsland(floor);
				floor.ScheduleCellsCollapse(map_.Find(TutorialConfig::kCodeWall), TutorialConfig::kWallCollapseDelay);
				isWallScheduled_ = true;
				stepTimer_ = 0.0f;
			}
			return;
		}

		//崩れ終わったら2つ目の積み木
		if (stepTimer_ >= TutorialConfig::kWallCollapseDelay + 0.4f)
		{
			ChangeStep(context, Step::BlockTwoFall);
		}
	}

	//切り離した島をボスのときと同じように揺らしてから落とす
	void TutorialFlow::UpdateIslandFall(const Context& context)
	{
		Floor& floor = context.floors.GetCurrent();
		if (stepTimer_ < FloorConfig::kEscapeDelay)
		{
			floor.Shake(stepTimer_, FloorConfig::kShakeDistance, FloorConfig::kShakeFrequency);
			return;
		}
		if (!isReleased_)
		{
			isReleased_ = true;
			floor.Release();
		}
		if (!isDropFinished_)
		{
			const float drop = std::min((stepTimer_ - FloorConfig::kEscapeDelay) / LayerConfig::kBossDropDuration, 1.0f);
			floor.AnimateDrop(LayerConfig::kFloorSpacing * drop * drop);
			if (drop >= 1.0f)
			{
				floor.FinishDrop();
				isDropFinished_ = true;
			}
			return;
		}
		if (block_.IsGone())
		{
			Clear(context, Step::GoDown);
		}
	}

	//最後の切り離しのあとは床を残して、それ以外はやることごとに飛ばして床を直す
	bool TutorialFlow::NeedsRepair(Step next)
	{
		return next != Step::GoDown && next != Step::Finale;
	}

	//足場のばねで最初の位置へ吹っ飛ばし始める
	void TutorialFlow::StartRepair(const Context& context)
	{
		isRepairing_ = true;
		hasRepaired_ = false;
		repairTimer_ = 0.0f;
		launch_.Begin(context.player.GetPosition(), GetStartPosition(context.floors.GetCurrent()));
	}

	//飛んでいる間に崩れる予定をやめて穴を全部戻し、尻もちから立ち上がったら次へ
	void TutorialFlow::UpdateRepair(const Context& context, float deltaTime)
	{
		Floor& floor = context.floors.GetCurrent();
		repairTimer_ += deltaTime;
		if (!hasRepaired_ && repairTimer_ >= TutorialConfig::kRepairAt && !block_.IsSinking())
		{
			hasRepaired_ = true;
			floor.CancelCollapses();
			map_.RestoreAll(floor);
		}
		if (!launch_.Update(deltaTime, context.player, floor, context.direction))
		{
			return;
		}
		isRepairing_ = false;
		if (!hasRepaired_)
		{
			floor.CancelCollapses();
			map_.RestoreAll(floor);
		}
		ChangeStep(context, pendingStep_);
	}
}
