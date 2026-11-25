#include "PlayerAction.h"
#include "../../Input/GameInput.h"
#include "../../Object/Player/Player.h"
#include "../../Stage/Floor/Floor.h"
#include "../../Stage/Grid/GridDirection.h"

using namespace RyoEngine;

namespace App
{
	//向きの入力があればそれを、なければ移動の入力の向きを向く
	void PlayerAction::ApplyLook(const PlayerCommand& command, Player& player) const
	{
		if (command.hasLook)
		{
			player.SetFacingDirection(GridDirection::ToWorld(command.look));
			return;
		}
		if (command.hasMovement)
		{
			player.SetFacingDirection(GridDirection::ToWorld(command.move));
		}
	}

	//床を崩して移動する
	bool PlayerAction::Apply(const PlayerCommand& command, Player& player, Floor& floor, float deltaTime)
	{
		//床の上か跳んでいる途中なら、今いるマスの正面を崩せる
		const Cell playerCell = floor.GetCellAt(player.GetPosition());
		bool canBreak = player.IsGrounded() || player.IsJumping();
		if (player.IsKnockedBack() || !floor.IsSolid(playerCell))
		{
			canBreak = false;
		}
		if (command.isBreak && canBreak && frontBreak_.TryBreak(floor, playerCell, player.GetFacing()))
		{
			//ハンマーを振り下ろして、当たった瞬間から床が崩れ始める
			player.StartSwing();
		}

		//はじかれていなければ押している間となりのマスへ跳び続ける
		return player.Advance(command.move, command.hasMovement, floor, deltaTime);
	}

	//床に正面の1マスの予告を出す
	void PlayerAction::UpdatePreview(Floor& floor, const Player& player, bool isVisible)
	{
		frontBreak_.UpdatePreview(floor, floor.GetCellAt(player.GetPosition()), player.GetFacing(), isVisible);
	}
}
