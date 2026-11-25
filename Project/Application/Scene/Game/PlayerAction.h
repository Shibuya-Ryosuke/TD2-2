#pragma once
#include "../../Object/Player/FrontBreak.h"

namespace App
{
	struct PlayerCommand;
	class Player;
	class Floor;

	//ボスと同時に動くプレイヤーの向きと床を崩すと移動
	class PlayerAction
	{
	public:
		//向きの入力か移動の入力の向きを向く
		void ApplyLook(const PlayerCommand& command, Player& player) const;

		//床を崩して移動し、落ちたらtrue
		bool Apply(const PlayerCommand& command, Player& player, Floor& floor, float deltaTime);

		//床に正面の1マスの予告を出す
		void UpdatePreview(Floor& floor, const Player& player, bool isVisible);

	private:
		FrontBreak frontBreak_; //正面の1マスを崩す
	};
}
