#pragma once
#include <cstdint>
#include "Screen/DamageFlash.h"
#include "Screen/FallLines.h"
#include "Boss/BossGhost.h"
#include "../Scene/Game/GamePhase.h"

namespace App
{
	class PlayerHealth;
	class Player;
	class Boss;
	class Floor;

	//ゲームシーンの画面の演出とボスの残像をまとめる
	class GameDirection
	{
	public:
		GameDirection() = default;
		~GameDirection() = default;

		GameDirection(const GameDirection&) = delete;
		GameDirection& operator=(const GameDirection&) = delete;

		void Initialize(); //ポストエフェクトと残像を用意する
		//HPと場面と着地を見て演出を進める
		void Update(const PlayerHealth& health, GamePhase phase, const Player& player, const Boss& boss, Floor& floor, const BossTuning& tuning);
		//キャラを描いたあとにボスの残像を描く
		void Draw();

	private:
		DamageFlash damage_;
		FallLines fallLines_;
		BossGhost ghost_;
		int32_t lastHearts_ = -1; //前のフレームのHP
		bool wasBossJumping_ = false; //前のフレームでボスが跳んでいたか
	};
}
