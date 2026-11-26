#include "ClearPose.h"
#include "../../Camera/StageCamera.h"
#include "../../Config/DirectionConfig.h"
#include "../../Object/Player/Player.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		constexpr float kRadToDeg = 180.0f / 3.14159265f; //ラジアンから度へ

		//ゆっくり始まってゆっくり止まる進み具合
		float SmoothStep(float t)
		{
			return t * t * (3.0f - 2.0f * t);
		}
	}

	//カメラの方を向かせて寄せ始める
	void ClearPose::Start(StageCamera& camera, Player& player)
	{
		elapsed_ = 0.0f;
		startOrthoSize_ = camera.GetOrthoSize();
		isPosing_ = false;
		holdElapsed_ = 0.0f;

		//カメラの向きと反対の水平な向きがカメラの方
		const Vector3 forward = camera.GetForward();
		const float yawDeg = std::atan2(-forward.x, -forward.z) * kRadToDeg;
		player.SetVisible(true);
		player.TurnForPose(yawDeg);
	}

	//寄せてからポーズをして少し待ち、終わったらtrue
	bool ClearPose::Update(float deltaTime, StageCamera& camera, Player& player)
	{
		elapsed_ += deltaTime;
		const float t = SmoothStep(std::clamp(elapsed_ / DirectionConfig::kClearZoomTime, 0.0f, 1.0f));

		//プレイヤーの胸が画面の真ん中に来るように横と縦だけずらす
		const Vector3& feet = player.GetPosition();
		const Vector3 target = { feet.x, feet.y + DirectionConfig::kClearFocusHeight, feet.z };
		camera.SetFocusOffset(camera.GetCenteringOffset(target) * t);
		camera.SetOrthoSize(startOrthoSize_ + (DirectionConfig::kClearZoomOrthoSize - startOrthoSize_) * t);

		//寄せ終わったらポーズを1回する
		if (elapsed_ < DirectionConfig::kClearZoomTime)
		{
			return false;
		}
		if (!isPosing_)
		{
			isPosing_ = true;
			player.StartPose();
			return false;
		}

		//ポーズが終わったら少し待つ
		if (!player.IsPoseFinished())
		{
			return false;
		}
		holdElapsed_ += deltaTime;
		return holdElapsed_ >= DirectionConfig::kClearHoldTime;
	}
}
