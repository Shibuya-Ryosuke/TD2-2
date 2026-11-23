#pragma once
#include <cstdint>
#include "GamePhase.h"

namespace App
{
	class PlayerHealth;
	class LayerFloors;
	class BossFallFlow;

	//デバッグ表示に出すもの
	struct GameHudInfo
	{
		int32_t hearts = 0; //プレイヤーのHP
		int32_t layer = 0; //0から数えた層
		int32_t bossHealth = 0; //ボスのHP
		GamePhase phase = GamePhase::Waiting;
		bool isPaused = false;
		float escapeTimer = 0.0f; //切り離された床が落ちるまで
		int32_t barrageTiles = 0; //飛ばす足場の数
		int32_t barrageDamage = 0; //足場のダメージ
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// bool hasBomb = false;        //十字爆弾を持っているか
		// bool hasJumpBoost = false;   //2マス乗り越えを持っているか
	};

	//Debugのときだけ出るゲームシーンのデバッグ表示
	class GameHud
	{
	public:
		//ゲームシーンの状態から出すものを集める
		static GameHudInfo MakeInfo(const PlayerHealth& health, const LayerFloors& floors, int32_t bossHealth,
			GamePhase phase, bool isPaused, const BossFallFlow& bossFall);

		static void Draw(const GameHudInfo& info);
	};
}
