#pragma once
#include <RyoEngine.h>

namespace App
{
	//ボスの1回の行動の種類
	enum class BossActionKind
	{
		Move, //逃げるか近づくために1マス跳ぶ
		Melee, //近接攻撃
		Punch, //吹き飛ばしパンチ
		Fall, //足場が崩れて落ちる
	};

	//BossPlannerが決めるボスの1回の行動
	struct BossPlannedAction
	{
		BossActionKind kind = BossActionKind::Move;
		RyoEngine::Vector3 to{}; //終わりの位置
		float duration = 0.4f; //かかる秒数
		bool isBlocked = false; //道がなくて動けなかった
		bool isEvading = false; //崩れる床から逃げている
	};
}
