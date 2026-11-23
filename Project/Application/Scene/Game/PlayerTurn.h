#pragma once
#include <cstdint>
#include "GamePhase.h"

namespace App
{
	struct PlayerCommand;
	class Player;
	class Boss;
	class Floor;
	class ShapeController;

	//プレイヤーが1回動くとボスが1回動く交互の手番
	class PlayerTurn
	{
	public:
		//配置し直したときにボスの番を消す
		void Reset() { pendingBossTurns_ = 0; }

		//ボスがまだ動いていないか行動の途中ならボスの番
		bool IsBossTurn(const Boss& boss) const;
		//プレイヤーが動けるか
		bool CanPlayerAct(GamePhase phase, const Boss& boss) const;
		//ボスの番でプレイヤーが跳び終わっていればボスが動ける
		bool CanBossAct(const Player& player) const;

		//ボスが1回動いた
		void OnBossAction();

		//プレイヤーの番だけ向きを変える
		void ApplyLook(const PlayerCommand& command, GamePhase phase, Player& player, const Boss& boss) const;
		//プレイヤーの番だけ床を崩すか移動して、落ちたらtrue
		bool ApplyAction(const PlayerCommand& command, GamePhase phase, Player& player, ShapeController& shapes, Floor& floor, const Boss& boss, float deltaTime);

	private:
		//戦闘中ならボスの番にする
		void OnPlayerAction(GamePhase phase);

		int32_t pendingBossTurns_ = 0; //ボスがあと何回動くか
	};
}
