#pragma once
#include <cstdint>
#include "Screen/DamageFlash.h"
#include "Screen/FallLines.h"
#include "../Scene/Game/GamePhase.h"

namespace App
{
	class PlayerHealth;
	class Player;
	class Boss;
	class Floor;

	//ゲームシーンの画面の演出と着地のぷるるんをまとめる
	class GameDirection
	{
	public:
		GameDirection() = default;
		~GameDirection() = default;

		GameDirection(const GameDirection&) = delete;
		GameDirection& operator=(const GameDirection&) = delete;

		void Initialize(); //ポストエフェクトを用意する
		//HPと場面と着地を見て演出を進める
		void Update(const PlayerHealth& health, GamePhase phase, const Player& player, const Boss& boss, Floor& floor);

	private:
		DamageFlash damage_;
		FallLines fallLines_;
		int32_t lastHearts_ = -1; //前のフレームのHP
		bool wasBossJumping_ = false; //前のフレームでボスが跳んでいたか
	};
}
