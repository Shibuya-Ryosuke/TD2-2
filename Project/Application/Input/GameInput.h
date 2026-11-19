#pragma once
#include <RyoEngine.h>

namespace App
{
	//1フレームぶんのプレイヤーの入力 (GameInput::ReadCommand で読む)
	struct PlayerCommand
	{
		RyoEngine::Vector2 move{};  //移動 (長さ 1 まで)
		bool hasMovement = false;   //移動の入力があるか
		RyoEngine::Vector2 look{};  //向き
		bool hasLook = false;       //向きの入力があるか
		bool isBreak = false;       //床を崩すボタンを押した瞬間か
		bool isBomb = false;        //十字爆弾のボタンを押した瞬間か

		//戦闘を始める入力があるか (移動・床を崩す・十字爆弾)
		bool HasAction() const { return hasMovement || isBreak || isBomb; }
	};

	//ゲームで使う入力をまとめたもの (キーボードとコントローラーの両方)
	class GameInput
	{
	public:
		//ゲーム中のプレイヤーの入力をまとめて読む
		static PlayerCommand ReadCommand();

		//移動 (WASD / 左スティック)。x: 右が + / y: 奥が +。長さは1まで
		static RyoEngine::Vector2 GetMove();

		//向き = 床を崩す形の向き (矢印キー / 右スティック / 十字キー)。x: 右が + / y: 奥が +）
		static RyoEngine::Vector2 GetLook();

		//床を崩すを押した瞬間か (Space / Aボタン)
		static bool IsBreakTriggered();

		//十字爆弾を使うボタンを押した瞬間か (E / Bボタン)
		static bool IsBombTriggered();

		//ポーズを押した瞬間か (Esc / STARTボタン)
		static bool IsPauseTriggered();

		//決定を押した瞬間か (Enter / Aボタン)。シーンを進めるときに使う
		static bool IsEnterTriggered();

	private:
		//長さを 1 までにする (斜めでも大きくならないように)
		static RyoEngine::Vector2 ClampLength(const RyoEngine::Vector2& value);
	};
}
