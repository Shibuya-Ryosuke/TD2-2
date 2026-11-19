#pragma once
#include <RyoEngine.h>

namespace App
{
	
	//ゲームで使う入力をまとめたもの (キーボードとコントローラーの両方)
	class GameInput 
	{
	public:
		//移動 (WASD / 左スティック)。x: 右が + / y: 奥が +。長さは1まで
		static RyoEngine::Vector2 GetMove();

		//向き = 床を崩す形の向き (矢印キー / 右スティック / 十字キー)。x: 右が + / y: 奥が +）
		static RyoEngine::Vector2 GetLook();

		//床を崩すを押した瞬間か (Space / Aボタン)
		static bool IsBreakTriggered();

		//ポーズを押した瞬間か (Esc / STARTボタン)
		static bool IsPauseTriggered();

		//決定を押した瞬間か (Enter / Aボタン)。シーンを進めるときに使う
		static bool IsEnterTriggered();

	private:
		//長さを 1 までにする (斜めでも大きくならないように)
		static RyoEngine::Vector2 ClampLength(const RyoEngine::Vector2& value);
	};
}
