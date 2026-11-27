#include "PlayerLaunch.h"
#include "../../Config/TutorialConfig.h"
#include "../../Direction/GameDirection.h"
#include "../../Object/Player/Player.h"
#include "../../Stage/Floor/Floor.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//足場がぐっと沈むところから始める
	void PlayerLaunch::Begin(const Vector3& from, const Vector3& target)
	{
		from_ = from;
		target_ = target;
		timer_ = 0.0f;
		stage_ = Stage::Windup;
	}

	//穴の下までじたばた落ちるところから始める
	void PlayerLaunch::BeginFromHole(const Vector3& from, const Vector3& target)
	{
		from_ = from;
		target_ = target;
		timer_ = 0.0f;
		stage_ = Stage::Drop;
	}

	//ためて、くるくる飛んで、尻もちで着く
	bool PlayerLaunch::Update(float deltaTime, Player& player, Floor& floor, GameDirection& direction)
	{
		if (stage_ == Stage::Idle)
		{
			return true;
		}
		timer_ += deltaTime;

		//だんだん速く穴の下まで落ちて、下でボヨン！とはね返される
		if (stage_ == Stage::Drop)
		{
			const float t = std::clamp(timer_ / TutorialConfig::kRescueDropTime, 0.0f, 1.0f);
			player.SetPose(PlayerPose::Kind::Flail);
			player.SetPosition(from_ + Vector3{ 0.0f, -TutorialConfig::kRescueDropDepth * t * t, 0.0f });
			if (t < 1.0f)
			{
				return false;
			}
			from_ = player.GetPosition();
			direction.Shake(TutorialConfig::kLaunchShake);
			stage_ = Stage::Fly;
			timer_ = 0.0f;
			return false;
		}

		//足場と一緒にぐっと沈んでじたばた
		if (stage_ == Stage::Windup)
		{
			const float t = std::clamp(timer_ / TutorialConfig::kLaunchWindupTime, 0.0f, 1.0f);
			player.SetPose(PlayerPose::Kind::Flail);
			player.SetPosition(from_ + Vector3{ 0.0f, -TutorialConfig::kLaunchWindupSink * t * t, 0.0f });
			if (t < 1.0f)
			{
				return false;
			}

			//ボヨン！と足場がはねてハートが散る
			const Cell from = floor.GetCellAt(from_);
			if (floor.Contains(from))
			{
				floor.Jiggle(from, TutorialConfig::kLaunchJiggle);
			}
			direction.Shake(TutorialConfig::kLaunchShake);
			stage_ = Stage::Fly;
			timer_ = 0.0f;
			return false;
		}

		//横はまっすぐ、縦は高く上がって、くるくる回りながらじたばた
		if (stage_ == Stage::Fly)
		{
			const float t = std::clamp(timer_ / TutorialConfig::kLaunchTime, 0.0f, 1.0f);
			Vector3 position = from_ + (target_ - from_) * t;
			position.y += 4.0f * TutorialConfig::kLaunchHeight * t * (1.0f - t);
			player.SetPosition(position);
			const float spin = timer_ * TutorialConfig::kLaunchSpinSpeed;
			player.SetFacingDirection({ std::sin(spin), 0.0f, std::cos(spin) });
			if (t < 1.0f)
			{
				return false;
			}

			//どすんと尻もち
			player.Place(target_);
			player.SetPose(PlayerPose::Kind::Sit);
			floor.Jiggle(floor.GetCellAt(target_), TutorialConfig::kLaunchLandJiggle);
			player.PlayLandingParticles();
			direction.Shake(TutorialConfig::kLaunchShake);
			stage_ = Stage::Land;
			timer_ = 0.0f;
			return false;
		}

		//少し座ってから立ち上がる
		if (timer_ < TutorialConfig::kLaunchSitTime)
		{
			return false;
		}
		player.SetPose(PlayerPose::Kind::Normal);
		stage_ = Stage::Idle;
		return true;
	}
}
