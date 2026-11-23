#include "PlayerTurn.h"
#include "../../Input/GameInput.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Player/Player.h"
#include "../../Object/Player/Shape/ShapeController.h"
#include "../../Stage/Floor/Floor.h"
#include "../../Stage/Grid/GridDirection.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	//ボスの番か
	bool PlayerTurn::IsBossTurn(const Boss& boss) const
	{
		return pendingBossTurns_ > 0 || boss.IsBusy();
	}

	//戦闘中はボスの番でないときだけプレイヤーが動ける
	bool PlayerTurn::CanPlayerAct(GamePhase phase, const Boss& boss) const
	{
		if (phase == GamePhase::Battle)
		{
			return !IsBossTurn(boss);
		}
		return true;
	}

	//ボスが新しい行動を始めてよいか
	bool PlayerTurn::CanBossAct(const Player& player) const
	{
		return pendingBossTurns_ > 0 && !player.IsJumping();
	}

	//ボスが1回動いた
	void PlayerTurn::OnBossAction()
	{
		pendingBossTurns_ = std::max(0, pendingBossTurns_ - 1);
	}

	//プレイヤーが1回動いた
	void PlayerTurn::OnPlayerAction(GamePhase phase)
	{
		if (phase == GamePhase::Battle)
		{
			pendingBossTurns_ = 1;
		}
	}

	//プレイヤーの番で跳んでいないときだけ向きを変える
	void PlayerTurn::ApplyLook(const PlayerCommand& command, GamePhase phase, Player& player, const Boss& boss) const
	{
		if (command.hasLook && CanPlayerAct(phase, boss) && !player.IsJumping())
		{
			player.SetFacing(GridDirection::FromInput(command.look));
		}
	}

	//床を崩すか移動する
	bool PlayerTurn::ApplyAction(const PlayerCommand& command, GamePhase phase, Player& player, ShapeController& shapes, Floor& floor, const Boss& boss, float deltaTime)
	{
		const bool canAct = CanPlayerAct(phase, boss);
		shapes.SetReady(canAct);

		//床の上に立っているときに崩せたら1回動いた
		const Cell playerCell = floor.GetCellAt(player.GetPosition());
		const bool isStanding = player.IsGrounded() && !player.IsJumping() && !player.IsKnockedBack() && floor.IsSolid(playerCell);
		if (command.isBreak && canAct && isStanding)
		{
			if (shapes.TryBreak(floor, playerCell, player.GetFacing()))
			{
				OnPlayerAction(phase);
			}
		}

		//プレイヤーの番に跳び始めたら1回動いた
		const bool wasJumping = player.IsJumping();
		const bool hasMovement = command.hasMovement && CanPlayerAct(phase, boss); //床を崩したらもうボスの番
		const bool hasFallen = player.Advance(command.move, hasMovement, floor, deltaTime);
		if (!wasJumping && player.IsJumping())
		{
			OnPlayerAction(phase);
		}
		return hasFallen;
	}
}
