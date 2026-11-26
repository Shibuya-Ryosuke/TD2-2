#pragma once
#include "../../Object/Player/FrontBreak.h"
#include <vector>

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

		//falseなら床を崩せない
		void SetBreakEnabled(bool isEnabled) { isBreakEnabled_ = isEnabled; }
		//このフレームで床を崩せたか
		bool WasBreakSucceeded() const { return wasBreakSucceeded_; }
		//壊せるマスをtargetsだけにして、ほかを叩くとはじかれる
		void LimitBreakTargets(const std::vector<Cell>& targets);
		//どのマスでも壊せるようにもどす
		void ClearBreakTargets();
		//このフレームで壊せないマスを叩いてはじかれたか
		bool WasBreakRejected() const { return wasBreakRejected_; }

	private:
		bool IsTarget(const Cell& cell) const; //壊してよいマスか
		void UpdateReject(Floor& floor, float deltaTime); //はじかれたマスにハンマーが当たったら揺らす
		void UpdateImpact(Player& player, Floor& floor); //ハンマーが当たった瞬間に足場のハートと崩れ始めをそろえる

		FrontBreak frontBreak_; //正面の1マスを崩す
		bool isBreakEnabled_ = true; //床を崩せるか
		bool wasBreakSucceeded_ = false; //このフレームで崩せたか
		std::vector<Cell> targets_; //壊せるマスを決めているときの壊せるマス
		bool isLimited_ = false; //壊せるマスを決めているか
		bool wasBreakRejected_ = false; //このフレームではじかれたか
		float rejectTimer_ = -1.0f; //はじかれたマスにハンマーが当たるまでの残りで、マイナスならなし
		Cell rejectCell_{}; //はじかれたマス
		Cell breakCell_{}; //崩してハンマーが当たるのを待っているマス
		bool isWaitingImpact_ = false; //ハンマーが当たるのを待っているか
	};
}
