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
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// bool isBomb = false;        //十字爆弾のボタンを押した瞬間か
	};

	//キーボードとコントローラーの入力をまとめる
	class GameInput
	{
	public:
		//ゲーム中のプレイヤーの入力をまとめて読む
		static PlayerCommand ReadCommand();

		//右と奥がプラスで長さ1までの移動
		static RyoEngine::Vector2 GetMove();

		//右と奥がプラスの向き
		static RyoEngine::Vector2 GetLook();

		//床を崩すを押した瞬間か
		static bool IsBreakTriggered();

		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// //EかBボタンの十字爆弾を押した瞬間か
		// static bool IsBombTriggered();

		//ポーズを押した瞬間か
		static bool IsPauseTriggered();

		//シーンを進める決定とメニューの決定を押した瞬間か
		static bool IsEnterTriggered();

		//メニューの上を押した瞬間か
		static bool IsMenuUpTriggered();
		//メニューの下を押した瞬間か
		static bool IsMenuDownTriggered();
		//メニューを選ぶ右スティックの上下
		static float GetMenuStickY();

	private:
		//斜めでも長さを1までにする
		static RyoEngine::Vector2 ClampLength(const RyoEngine::Vector2& value);
		//RTを半分より押しこんだ瞬間か
		static bool IsRightTriggerPressed();

		static constexpr BYTE kRightTriggerThreshold = 128; //0から255のRTの押しこみでこれ以上を押したとする
		static inline bool wasRightTriggerDown_ = false; //前に読んだときRTを押しこんでいたか
	};
}
