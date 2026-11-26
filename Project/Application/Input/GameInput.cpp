#include "GameInput.h"
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//斜めでも長さを1までにする
	Vector2 GameInput::ClampLength(const Vector2& value)
	{
		Vector2 result = value;
		const float length = std::sqrt(result.x * result.x + result.y * result.y);
		if (length > 1.0f)
		{
			result.x /= length;
			result.y /= length;
		}
		return result;
	}

	//移動はWASDか左スティック
	Vector2 GameInput::GetMove()
	{
		Vector2 move = { Input::GetLeftStickX(), Input::GetLeftStickY() };
		if (Input::PushKey(DIK_A))
		{
			move.x -= 1.0f;
		}
		if (Input::PushKey(DIK_D))
		{
			move.x += 1.0f;
		}
		if (Input::PushKey(DIK_W))
		{
			move.y += 1.0f;
		}
		if (Input::PushKey(DIK_S))
		{
			move.y -= 1.0f;
		}
		return ClampLength(move);
	}

	//向きは矢印キーか右スティックか十字キー
	Vector2 GameInput::GetLook()
	{
		Vector2 look = { Input::GetRightStickX(), Input::GetRightStickY() };
		if (Input::PushKey(DIK_LEFT) || Input::GetJoystickButton(XINPUT_GAMEPAD_DPAD_LEFT))
		{
			look.x -= 1.0f;
		}
		if (Input::PushKey(DIK_RIGHT) || Input::GetJoystickButton(XINPUT_GAMEPAD_DPAD_RIGHT))
		{
			look.x += 1.0f;
		}
		if (Input::PushKey(DIK_UP) || Input::GetJoystickButton(XINPUT_GAMEPAD_DPAD_UP))
		{
			look.y += 1.0f;
		}
		if (Input::PushKey(DIK_DOWN) || Input::GetJoystickButton(XINPUT_GAMEPAD_DPAD_DOWN))
		{
			look.y -= 1.0f;
		}
		return ClampLength(look);
	}

	//ゲーム中のプレイヤーの入力をまとめて読む
	PlayerCommand GameInput::ReadCommand()
	{
		PlayerCommand command{};
		command.move = GetMove();
		command.hasMovement = command.move.x * command.move.x + command.move.y * command.move.y > 0.0001f;
		command.look = GetLook();
		command.hasLook = command.look.x * command.look.x + command.look.y * command.look.y > 0.0001f;
		command.isBreak = IsBreakTriggered();
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// command.isBomb = IsBombTriggered();
		return command;
	}

	//床を崩すのはSpaceかAボタンかRTを押しこんだ瞬間
	bool GameInput::IsBreakTriggered()
	{
		const bool isRightTriggerPressed = IsRightTriggerPressed();
		if (Input::TriggerKey(DIK_SPACE))
		{
			return true;
		}
		if (Input::GetJoystickTrigger(XINPUT_GAMEPAD_A))
		{
			return true;
		}
		return isRightTriggerPressed;
	}

	//エンジンにRTを読む関数がないのでXInputから直接読み、半分より押しこんだ瞬間だけtrue
	bool GameInput::IsRightTriggerPressed()
	{
		XINPUT_STATE state{};
		bool isDown = false;
		if (XInputGetState(0, &state) == ERROR_SUCCESS)
		{
			isDown = state.Gamepad.bRightTrigger >= kRightTriggerThreshold;
		}
		const bool isPressed = isDown && !wasRightTriggerDown_;
		wasRightTriggerDown_ = isDown;
		return isPressed;
	}

	//[アイテム] アイテムは一旦使わないのでコメントアウト
	// //EかBボタンの十字爆弾
	// bool GameInput::IsBombTriggered()
	// {
		// if (Input::TriggerKey(DIK_E))
		// {
			// return true;
		// }
		// if (Input::GetJoystickTrigger(XINPUT_GAMEPAD_B))
		// {
			// return true;
		// }
		// return false;
	// }

	//ポーズはTABかメニューボタン
	bool GameInput::IsPauseTriggered()
	{
		if (Input::TriggerKey(DIK_TAB))
		{
			return true;
		}
		if (Input::GetJoystickTrigger(XINPUT_GAMEPAD_START))
		{
			return true;
		}
		return false;
	}

	//決定はEnterかAボタン
	bool GameInput::IsEnterTriggered()
	{
		if (Input::TriggerKey(DIK_RETURN))
		{
			return true;
		}
		if (Input::GetJoystickTrigger(XINPUT_GAMEPAD_A))
		{
			return true;
		}

		return false;
	}

	//メニューの上はWか十字の上
	bool GameInput::IsMenuUpTriggered()
	{
		if (Input::TriggerKey(DIK_W))
		{
			return true;
		}
		if (Input::GetJoystickTrigger(XINPUT_GAMEPAD_DPAD_UP))
		{
			return true;
		}
		return false;
	}

	//メニューの下はSか十字の下
	bool GameInput::IsMenuDownTriggered()
	{
		if (Input::TriggerKey(DIK_S))
		{
			return true;
		}
		if (Input::GetJoystickTrigger(XINPUT_GAMEPAD_DPAD_DOWN))
		{
			return true;
		}
		return false;
	}

	//メニューを選ぶ右スティックの上下で上がプラス
	float GameInput::GetMenuStickY()
	{
		return Input::GetRightStickY();
	}
}
