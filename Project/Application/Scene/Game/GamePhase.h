#pragma once
#include <cstdint>

namespace App
{
	//ゲームシーンの今の場面
	enum class GamePhase : int32_t
	{
		Battle, //戦闘中
		Escape, //ボスの島が切り離されて揺れている
		Descent, //ボスが落ちてプレイヤーも下の層へ
		LandingImpact, //ボスが着地した直後の待ち
		TileBarrage, //上の層の足場がボスへ飛んでいく
		PlayerFall, //プレイヤーが穴に落ちて下の層へ
		BossWake, //チュートリアルのあと寝ていたボスが起きる
	};
}
