#include "TutorialFlow.h"
#include "../PlayerAction.h"
#include "../SpawnPlanner.h"
#include "../../../Camera/StageCamera.h"
#include "../../../Config/GameConfig.h"
#include "../../../Config/TutorialConfig.h"
#include "../../../Direction/GameDirection.h"
#include "../../../Input/GameInput.h"
#include "../../../Object/Boss/Boss.h"
#include "../../../Object/Player/Player.h"
#include "../../../Stage/Floor/LayerFloors.h"
#include <algorithm>
#include <limits>

using namespace RyoEngine;

namespace App
{
	//csvを読んで積み木を作る
	void TutorialFlow::Initialize()
	{
		map_.Load();
		block_.Initialize();
	}

	//プレイヤーを最初の位置に、ボスを下の層に寝かせて始める
	void TutorialFlow::Begin(const Context& context)
	{
		Floor& floor = context.floors.GetCurrent();
		context.floors.EnsureBelow();
		context.player.Place(GetStartPosition(floor));

		//ボスは下の層のいつもの最初のマスで横になって寝ている
		const Floor& below = *context.floors.GetBelow();
		const Vector3 bossPosition = LayerFloors::GetSpawnPosition(below, GameConfig::kBossSpawnX, GameConfig::kBossSpawnZ);
		context.boss.SetActive(true);
		context.boss.Place(below, bossPosition);
		context.boss.Stop();
		context.boss.SetSleep(1.0f);

		isRescuing_ = false;
		clearTimer_ = 0.0f;
		ChangeStep(context, Step::Move);
	}

	//1フレーム進める
	TutorialFlow::Result TutorialFlow::Update(const Context& context, const PlayerCommand& command, float deltaTime)
	{
		Result result{};
		if (step_ == Step::Finale)
		{
			const TutorialFinale::Result finale = finale_.Update(deltaTime, context.floors, context.player, context.boss, context.camera, context.direction);
			result.bossDamage = finale.bossDamage;
			result.isFinished = finale.isFinished;
			return result;
		}

		//CLEAR!を出し終わったら次のやることへ
		stepTimer_ += deltaTime;
		clearTimer_ -= deltaTime;
		if (hasPendingStep_)
		{
			pendingTimer_ -= deltaTime;
			if (pendingTimer_ <= 0.0f)
			{
				ChangeStep(context, pendingStep_);
			}
		}

		//床の崩れと積み木を進めて、プレイヤーを動かす
		Floor& floor = context.floors.GetCurrent();
		const bool isFloorChanged = floor.Tick(deltaTime);
		block_.Update(deltaTime, floor, context.camera, context.direction);
		UpdateControl(context, command, deltaTime);
		if (step_ != Step::Finale)
		{
			UpdateStep(context, isFloorChanged, deltaTime);
		}
		return result;
	}

	//段階を変えて、その段階の始めにやることをする
	void TutorialFlow::ChangeStep(const Context& context, Step step)
	{
		step_ = step;
		stepTimer_ = 0.0f;
		hasPendingStep_ = false;
		if (step == Step::Move)
		{
			jumpStart_ = context.player.GetJumpCount();
			movedCount_ = 0;
		}
		else if (step == Step::Break)
		{
			breakCount_ = 0;
		}
		else if (step == Step::BlockOneFall)
		{
			DropBlock(context, TutorialConfig::kCodeFirstBlock);
		}
		else if (step == Step::Reshape)
		{
			hasLaunched_ = false;
			isWallScheduled_ = false;
		}
		else if (step == Step::BlockTwoFall)
		{
			map_.RestoreIsland(context.floors.GetCurrent());
			DropBlock(context, TutorialConfig::kCodeSecondBlock);
		}
		else if (step == Step::IslandFall)
		{
			isReleased_ = false;
			isDropFinished_ = false;
		}
		else if (step == Step::GoDown)
		{
			context.player.SetCanLeaveFloor(true);
		}
	}

	//CLEAR!を出してハートをはじけさせ、少し待ってから次へ
	void TutorialFlow::Clear(const Context& context, Step next)
	{
		clearTimer_ = TutorialConfig::kClearShowTime;
		hasPendingStep_ = true;
		pendingStep_ = next;
		pendingTimer_ = TutorialConfig::kNextStepDelay;
		context.player.BurstHearts(1.5f);
	}

	//その数字のマスか、崩れるか穴ならいちばん近い崩れない床へ積み木を降らせる
	void TutorialFlow::DropBlock(const Context& context, int32_t code)
	{
		const Floor& floor = context.floors.GetCurrent();
		const Cell target = map_.FindFirst(code);
		Cell best = floor.GetCellAt(SpawnPlanner::FindNearestStand(floor, floor.GetCellCenter(target)));
		int32_t bestDistance = std::numeric_limits<int32_t>::max();
		for (int32_t z = 0; z < floor.GetDepth(); ++z)
		{
			for (int32_t x = 0; x < floor.GetWidth(); ++x)
			{
				const Cell cell = { x, z };
				if (!CanHoldBlock(floor, cell))
				{
					continue;
				}
				const int32_t distance = (cell.x - target.x) * (cell.x - target.x) + (cell.z - target.z) * (cell.z - target.z);
				if (distance < bestDistance)
				{
					bestDistance = distance;
					best = cell;
				}
			}
		}
		block_.Drop(floor, best);
	}

	//積み木が降ってくる段階で使うcsvの数字
	int32_t TutorialFlow::ToBlockCode(Step step)
	{
		if (step == Step::BlockOneFall)
		{
			return TutorialConfig::kCodeFirstBlock;
		}
		return TutorialConfig::kCodeSecondBlock;
	}

	//積み木を置ける崩れる予定のない床か
	bool TutorialFlow::CanHoldBlock(const Floor& floor, const Cell& cell)
	{
		return floor.IsSolid(cell) && !floor.IsIsolated(cell) && !floor.IsCollapseScheduled(cell);
	}

	//落ちて戻している間と、積み木を追うカメラの間と、飛ばしている間とボスの所では動かせない
	bool TutorialFlow::CanControl() const
	{
		if (isRescuing_ || block_.IsCinematic())
		{
			return false;
		}
		return step_ != Step::Reshape && step_ != Step::Finale;
	}

	//プレイヤーを動かして、落ちたら最初の位置へ戻すか最後なら下の層へ
	void TutorialFlow::UpdateControl(const Context& context, const PlayerCommand& command, float deltaTime)
	{
		Floor& floor = context.floors.GetCurrent();
		if (isRescuing_)
		{
			//戻りきったらボン！と着地
			if (launch_.Update(deltaTime, context.player))
			{
				isRescuing_ = false;
				context.player.SetPose(PlayerPose::Kind::Normal);
				floor.Jiggle(floor.GetCellAt(context.player.GetPosition()), TutorialConfig::kLaunchJiggle * 0.5f);
			}
			return;
		}
		if (!CanControl())
		{
			return;
		}

		//5マス動くの間は崩せない
		context.action.ApplyLook(command, context.player);
		context.action.SetBreakEnabled(step_ != Step::Move);
		if (!context.action.Apply(command, context.player, floor, deltaTime))
		{
			return;
		}

		//最後のやることなら下の層へ、それ以外は最初の位置へ戻す
		if (step_ == Step::GoDown)
		{
			finale_.Begin(context.floors, context.player, context.boss);
			step_ = Step::Finale;
			return;
		}
		StartRescue(context);
	}

	//落ちたらトランポリンで最初の位置へ戻す
	void TutorialFlow::StartRescue(const Context& context)
	{
		isRescuing_ = true;
		launch_.Begin(context.player.GetPosition(), GetStartPosition(context.floors.GetCurrent()));
		context.direction.Shake(0.25f);
	}

	//最初の位置か、穴ならいちばん近い床
	Vector3 TutorialFlow::GetStartPosition(const Floor& floor) const
	{
		const Cell start = map_.FindFirst(TutorialConfig::kCodeStart);
		if (floor.IsSolid(start) && !floor.IsIsolated(start))
		{
			return floor.GetCellCenter(start);
		}
		return SpawnPlanner::FindNearestStand(floor, floor.GetCellCenter(start));
	}

	//落ちる線を出す間か
	bool TutorialFlow::IsFalling() const
	{
		return step_ == Step::Finale && finale_.IsFalling();
	}

	//崩せるときだけ正面の1マスの予告を出す
	bool TutorialFlow::IsPreviewVisible() const
	{
		return CanControl() && step_ != Step::Move;
	}

	//積み木を描く
	void TutorialFlow::Draw()
	{
		block_.Draw();
	}
}
