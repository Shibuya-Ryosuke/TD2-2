#pragma once
#include <cstdint>
#include "../Game/GamePhase.h"

namespace App
{
	// ゲームシーンのデバッグ表示に出すもの
	struct GameHudInfo
	{
		int32_t hearts = 0;          // プレイヤーの HP
		int32_t layer = 0;           // 何層目か (0 から)
		int32_t bossHealth = 0;      // ボスの HP
		GamePhase phase = GamePhase::Waiting;
		bool isPaused = false;
		float escapeTimer = 0.0f;    // 切り離された床が落ちるまで
		int32_t barrageTiles = 0;    // 飛ばす足場の数
		int32_t barrageDamage = 0;   // 足場のダメージ
		bool hasBomb = false;        // 十字爆弾を持っているか
		bool hasJumpBoost = false;   // 2マス乗り越えを持っているか
	};

	// ゲームシーンのデバッグ表示 (HP・アイテム・お知らせ・操作。Game から分けたもの)
	// 英数字だけ・Debug のときだけ出る
	class GameHud
	{
	public:
		static void Draw(const GameHudInfo& info);
	};
}
