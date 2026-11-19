#pragma once
#include <cstdint>

namespace App
{
	// 層ごとのボスの強さ
	struct BossTuning
	{
		//移動
		float moveSpeed = 3.0f;

		//近接攻撃 (足元の範囲)
		float meleeRange = 2.0f;       // 届く距離
		float meleeInterval = 2.0f;    // 次に使えるまで(秒)
		float meleeWindup = 0.55f;     // 予告してから当たるまで(秒)
		float meleeHeight = 2.0f;      // 上下にどこまで届くか

		int32_t damage = 1;            // 当たったときに減るハート

		// 跳ぶ (移動はいつも1マスずつ跳ぶ。canJumpなら穴を1つ飛び越える・崩れる床から逃げる)
		bool canJump = false;
		float jumpHeight = 1.25f;      // 跳ぶ高さ
		float jumpDuration = 0.7f;     // 崩れる床から逃げるときに跳ぶ時間(秒)

		//パンチ
		bool punch = false;
		float punchRange = 4.0f;           // 前にどこまで届くか
		float punchWidth = 2.5f;           // 横の幅
		float punchWindup = 0.475f;        // 予告(秒)
		float punchStrikeDuration = 0.16f; // 拳が伸びる時間(秒)
		float punchRecovery = 0.65f;       // 拳が戻る時間(秒)
		float punchInterval = 5.0f;        // 次に使えるまで(秒)
		float punchPushSpeed = 14.0f;      // はじかれる速さ
		float punchPushDuration = 0.55f;   // はじかれている時間(秒)
		float punchLiftSpeed = 4.0f;       // 上へ浮く速さ
	};

	// 何層目のボスの強さ。6層目からは1層目に戻る（ループさせてる！！）
	inline BossTuning GetBossTuning(int32_t layerIndex)
	{
		BossTuning tuning{};
		int32_t index = layerIndex % 5;
		if (index < 0)
		{
			index = 0;
		}

		// 1層目: 近接攻撃だけ
		if (index == 0)
		{
			tuning.moveSpeed = 3.0f;
			return tuning;
		}
		// 2層目: 少し速い
		if (index == 1)
		{
			tuning.moveSpeed = 3.25f;
			return tuning;
		}
		// 3層目: 穴を飛び越える
		if (index == 2)
		{
			tuning.moveSpeed = 3.5f;
			tuning.canJump = true;
			return tuning;
		}
		// 4層目: パンチ。攻撃の間隔が短くなる
		if (index == 3)
		{
			tuning.moveSpeed = 3.75f;
			tuning.meleeInterval = 1.6f;
			tuning.canJump = true;
			tuning.punch = true;
			return tuning;
		}

		// 5層目: いちばん速い
		tuning.moveSpeed = 4.0f;
		tuning.meleeInterval = 1.35f;
		tuning.meleeWindup = 0.45f;
		tuning.canJump = true;
		tuning.punch = true;
		return tuning;
	}
}
