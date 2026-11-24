#pragma once
#include <cstdint>
#include "../../Config/GameConfig.h"

namespace App
{
	//層ごとのボスの強さ
	struct BossTuning
	{
		//移動
		float moveSpeed = 3.0f;

		//足元の範囲への近接攻撃
		float meleeRange = 2.0f; //届く距離
		float meleeInterval = 2.0f; //交互の手番になったので今は使っていない間隔
		float meleeWindup = 0.55f; //予告してから当たるまでの秒数
		float meleeHeight = 2.0f; //上下にどこまで届くか

		int32_t damage = 1; //当たったときに減るハート

		//いつも1マスずつ跳び、canJumpなら穴を1つ飛び越えて崩れる床から逃げる
		bool canJump = false;
		float jumpHeight = 1.25f; //跳ぶ高さ
		float jumpDuration = 0.7f; //崩れる床から逃げるときに跳ぶ秒数
		float landingSafetyMargin = 0.0f; //着地してから崩れるまでに欲しい余裕の秒数

		//パンチ
		bool punch = false;
		float punchRange = 4.0f; //前にどこまで届くか
		float punchWidth = 2.5f; //横の幅
		float punchWindup = 0.475f; //予告の秒数
		float punchStrikeDuration = 0.16f; //拳が伸びる秒数
		float punchRecovery = 0.65f; //拳が戻る秒数
		float punchInterval = 5.0f; //今は使っていない間隔
		float punchPushSpeed = 14.0f; //はじかれる速さ
		float punchPushDuration = 0.55f; //はじかれている秒数
		float punchLiftSpeed = 4.0f; //上へ浮く速さ
	};

	//何層目のボスの強さを長さの倍率をかける前の数字で作る
	inline BossTuning MakeBossTuning(int32_t layerIndex)
	{
		BossTuning tuning{};
		int32_t index = layerIndex % 5;
		if (index < 0)
		{
			index = 0;
		}

		//1層目は近接攻撃だけ
		if (index == 0)
		{
			tuning.moveSpeed = 3.0f;
			return tuning;
		}
		//2層目は少し速い
		if (index == 1)
		{
			tuning.moveSpeed = 3.25f;
			return tuning;
		}
		//3層目は穴を飛び越える
		if (index == 2)
		{
			tuning.moveSpeed = 3.5f;
			tuning.canJump = true;
			return tuning;
		}
		//4層目はパンチが出て攻撃の間隔が短い
		if (index == 3)
		{
			tuning.moveSpeed = 3.75f;
			tuning.meleeInterval = 1.6f;
			tuning.canJump = true;
			tuning.punch = true;
			return tuning;
		}

		//5層目はいちばん速い
		tuning.moveSpeed = 4.0f;
		tuning.meleeInterval = 1.35f;
		tuning.meleeWindup = 0.45f;
		tuning.canJump = true;
		tuning.punch = true;
		return tuning;
	}

	//床の大きさに合わせて速さと届く距離を長さの倍率で大きくした何層目のボスの強さ
	inline BossTuning GetBossTuning(int32_t layerIndex)
	{
		BossTuning tuning = MakeBossTuning(layerIndex);
		const float scale = GameConfig::kWorldScale;
		tuning.moveSpeed *= scale;
		tuning.meleeRange *= scale;
		tuning.jumpHeight *= scale;
		tuning.punchRange *= scale;
		tuning.punchWidth *= scale;
		tuning.punchPushSpeed *= scale;
		return tuning;
	}
}
