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
		command.isPreviousShape = IsPreviousShapeTriggered();
		command.isNextShape = IsNextShapeTriggered();
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// command.isBomb = IsBombTriggered();
		return command;
	}

	//床を崩すのはSpaceかAボタン
	bool GameInput::IsBreakTriggered()
	{
		if (Input::TriggerKey(DIK_SPACE))
		{
			return true;
		}
		if (Input::GetJoystickTrigger(XINPUT_GAMEPAD_A))
		{
			return true;
		}
		return false;
	}

	//[アイテム] Eは形の切り替えに使っているのでコメントアウト
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

	//形を前へはQかLB
	bool GameInput::IsPreviousShapeTriggered()
	{
		if (Input::TriggerKey(DIK_Q))
		{
			return true;
		}
		if (Input::GetJoystickTrigger(XINPUT_GAMEPAD_LEFT_SHOULDER))
		{
			return true;
		}
		return false;
	}

	//形を次へはEかRB
	bool GameInput::IsNextShapeTriggered()
	{
		if (Input::TriggerKey(DIK_E))
		{
			return true;
		}
		if (Input::GetJoystickTrigger(XINPUT_GAMEPAD_RIGHT_SHOULDER))
		{
			return true;
		}
		return false;
	}

	//ポーズはEscかSTARTボタン
	bool GameInput::IsPauseTriggered()
	{
		if (Input::TriggerKey(DIK_ESCAPE))
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
}
