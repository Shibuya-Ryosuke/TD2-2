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
		Deferred, //落ちたあとで下の層へ行ってから決める
	};

	//先読みと残像に使うボスの1回の行動
	struct BossPlannedAction
	{
		BossActionKind kind = BossActionKind::Move;
		RyoEngine::Vector3 from{}; //始まりの位置
		RyoEngine::Vector3 to{}; //終わりの位置
		RyoEngine::Vector3 target{}; //狙う位置
		float duration = 0.4f; //かかる秒数
		float windup = 0.0f; //予告の秒数
		bool isBlocked = false; //道がなくて動けなかった
		bool isEvading = false; //崩れる床から逃げている

		//攻撃か
		bool IsAttack() const { return kind == BossActionKind::Melee || kind == BossActionKind::Punch; }
	};
}
