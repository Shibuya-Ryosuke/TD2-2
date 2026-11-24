#pragma once
#include <RyoEngine.h>

namespace App
{
	//1フレームぶんのプレイヤーの入力
	struct PlayerCommand
	{
		RyoEngine::Vector2 move{}; //長さ1までの移動
		bool hasMovement = false; //移動の入力があるか
		RyoEngine::Vector2 look{}; //向き
		bool hasLook = false; //向きの入力があるか
		bool isBreak = false; //床を崩すボタンを押した瞬間か
		bool isPreviousShape = false; //形を前へ切り替える瞬間か
		bool isNextShape = false; //形を次へ切り替える瞬間か
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// bool isBomb = false;        //十字爆弾のボタンを押した瞬間か

		//移動か床を崩す入力で戦闘を始める
		bool HasAction() const { return hasMovement || isBreak; }
	};

	//キーボードとコントローラーの入力をまとめる
	class GameInput
	{
	public:
		//ゲーム中のプレイヤーの入力をまとめて読む
		static PlayerCommand ReadCommand();

		//右と奥がプラスで長さ1までの移動
		static RyoEngine::Vector2 GetMove();

		//右と奥がプラスの床を崩す形の向き
		static RyoEngine::Vector2 GetLook();

		//床を崩すを押した瞬間か
		static bool IsBreakTriggered();

		//形を前へ切り替えるボタンを押した瞬間か
		static bool IsPreviousShapeTriggered();

		//形を次へ切り替えるボタンを押した瞬間か
		static bool IsNextShapeTriggered();

		//[アイテム] Eは形の切り替えに使っているのでコメントアウト
		// //EかBボタンの十字爆弾を押した瞬間か
		// static bool IsBombTriggered();

		//ポーズを押した瞬間か
		static bool IsPauseTriggered();

		//シーンを進める決定を押した瞬間か
		static bool IsEnterTriggered();

	private:
		//斜めでも長さを1までにする
		static RyoEngine::Vector2 ClampLength(const RyoEngine::Vector2& value);
	};
}
