#include "PlayerAction.h"
#include "../../Config/DirectionConfig.h"
#include "../../Config/FloorConfig.h"
#include "../../Input/GameInput.h"
#include "../../Object/Player/Player.h"
#include "../../Stage/Floor/Floor.h"
#include "../../Stage/Grid/GridDirection.h"

using namespace RyoEngine;

namespace App
{
	//向きの入力があればいちばん近い上下左右へ、なければ跳ぶ上下左右の向きをまっすぐ向く
	void PlayerAction::ApplyLook(const PlayerCommand& command, Player& player) const
	{
		if (command.hasLook)
		{
			const Cell look = GridDirection::FromInput(command.look);
			player.SetFacingDirection({ static_cast<float>(look.x), 0.0f, static_cast<float>(look.z) });
			return;
		}
		if (command.hasMovement)
		{
			const Cell step = GridDirection::FromInput(command.move);
			player.SetFacingDirection({ static_cast<float>(step.x), 0.0f, static_cast<float>(step.z) });
		}
	}

	//床を崩して移動する
	bool PlayerAction::Apply(const PlayerCommand& command, Player& player, Floor& floor, float deltaTime)
	{
		//床の上か跳んでいる途中なら、今いるマスの正面を崩せる
		const Cell playerCell = floor.GetCellAt(player.GetPosition());
		bool canBreak = player.IsGrounded() || player.IsJumping();
		if (player.IsKnockedBack() || !floor.IsSolid(playerCell) || !isBreakEnabled_)
		{
			canBreak = false;
		}
		wasBreakSucceeded_ = false;
		wasBreakRejected_ = false;
		UpdateReject(floor, deltaTime);
		UpdateImpact(player, floor);
		const Cell front = playerCell + player.GetFacing();
		if (command.isBreak && canBreak && isLimited_ && !IsTarget(front))
		{
			//壊せないマスはハンマーがポヨンとはじかれて床がぷるるんと揺れるだけ
			if (rejectTimer_ < 0.0f && floor.IsSolid(front))
			{
				player.StartSwing();
				rejectTimer_ = DirectionConfig::kSwingImpactDelay;
				rejectCell_ = front;
				impactCell_ = front;
				doesImpactBreak_ = false;
				isWaitingImpact_ = true;
				wasBreakRejected_ = true;
			}
		}
		else if (command.isBreak && canBreak && frontBreak_.TryBreak(floor, playerCell, player.GetFacing()))
		{
			//ハンマーを振り下ろして、当たった瞬間から床が崩れ始める
			player.StartSwing();
			wasBreakSucceeded_ = true;
			impactCell_ = front;
			doesImpactBreak_ = true;
			isWaitingImpact_ = true;
		}

		//はじかれていなければ押している間となりのマスへ跳び続ける
		return player.Advance(command.move, command.hasMovement, floor, deltaTime);
	}

	//壊せるマスをtargetsだけにする
	void PlayerAction::LimitBreakTargets(const std::vector<Cell>& targets)
	{
		targets_ = targets;
		isLimited_ = true;
	}

	//どのマスでも壊せるようにもどす
	void PlayerAction::ClearBreakTargets()
	{
		targets_.clear();
		isLimited_ = false;
	}

	//壊してよいマスか
	bool PlayerAction::IsTarget(const Cell& cell) const
	{
		for (const Cell& target : targets_)
		{
			if (target == cell)
			{
				return true;
			}
		}
		return false;
	}

	//はじかれたマスにハンマーが当たったらぷるるんと揺らす
	void PlayerAction::UpdateReject(Floor& floor, float deltaTime)
	{
		if (rejectTimer_ < 0.0f)
		{
			return;
		}
		rejectTimer_ -= deltaTime;
		if (rejectTimer_ < 0.0f)
		{
			floor.Jiggle(rejectCell_, DirectionConfig::kBreakRejectJiggle);
		}
	}

	//ハンマーの打撃に合わせて足場からハートを出して崩れ始めをそろえる
	void PlayerAction::UpdateImpact(Player& player, Floor& floor)
	{
		if (!isWaitingImpact_ || !player.WasHammerImpact())
		{
			return;
		}
		isWaitingImpact_ = false;
		if (floor.IsSolid(impactCell_))
		{
			player.BurstHammerHearts(floor.GetCellCenter(impactCell_));
		}
		if (doesImpactBreak_)
		{
			floor.SyncCollapse(impactCell_, FloorConfig::kCollapseDelay);
		}
	}

	//床に正面の1マスの予告を出す
	void PlayerAction::UpdatePreview(Floor& floor, const Player& player, bool isVisible)
	{
		frontBreak_.UpdatePreview(floor, floor.GetCellAt(player.GetPosition()), player.GetFacing(), isVisible);
	}
}
