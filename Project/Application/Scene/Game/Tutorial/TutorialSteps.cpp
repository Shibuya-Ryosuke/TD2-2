#include "TutorialFlow.h"
#include "../PlayerAction.h"
#include "../../../Config/GameConfig.h"
#include "../../../Config/TutorialConfig.h"
#include "../../../Direction/GameDirection.h"
#include "../../../Object/Player/Player.h"
#include "../../../Stage/Floor/LayerFloors.h"
#include <algorithm>

using namespace RyoEngine;

//TutorialFlowのうち段階ごとの進み具合を見るところ
namespace App
{
	//段階ごとの進み具合を見る
	void TutorialFlow::UpdateStep(const Context& context, bool isFloorChanged, float deltaTime)
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
			UpdateReshape(context, deltaTime);
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

	//足場のボン！で最初の位置へ飛ばしてから、切り離しやすく床を崩す
	void TutorialFlow::UpdateReshape(const Context& context, float deltaTime)
	{
		Floor& floor = context.floors.GetCurrent();
		Player& player = context.player;
		if (!hasLaunched_)
		{
			hasLaunched_ = true;
			floor.Jiggle(floor.GetCellAt(player.GetPosition()), TutorialConfig::kLaunchJiggle);
			player.BurstHearts(1.2f);
			context.direction.Shake(0.3f);
			launch_.Begin(player.GetPosition(), GetStartPosition(floor));
			return;
		}
		if (launch_.IsActive())
		{
			if (launch_.Update(deltaTime, player))
			{
				player.SetPose(PlayerPose::Kind::Normal);
				floor.Jiggle(floor.GetCellAt(player.GetPosition()), TutorialConfig::kLaunchJiggle * 0.5f);
				stepTimer_ = 0.0f;
			}
			return;
		}

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
		if (stepTimer_ < GameConfig::kEscapeDelay)
		{
			floor.Shake(stepTimer_, GameConfig::kShakeDistance, GameConfig::kShakeFrequency);
			return;
		}
		if (!isReleased_)
		{
			isReleased_ = true;
			floor.Release();
		}
		if (!isDropFinished_)
		{
			const float drop = std::min((stepTimer_ - GameConfig::kEscapeDelay) / GameConfig::kBossDropDuration, 1.0f);
			floor.AnimateDrop(GameConfig::kFloorSpacing * drop * drop);
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
}
