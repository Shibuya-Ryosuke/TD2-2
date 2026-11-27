#include "PlayerTumble.h"
#include "../../Config/DirectionConfig.h"
#include "../../Direction/GameDirection.h"
#include "../../Object/Player/Player.h"
#include "../../Stage/Floor/Floor.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	//足をばたばたさせて落ち始める
	void PlayerTumble::Begin(const Vector3& from, const Vector3& landing, Player& player)
	{
		from_ = from;
		landing_ = landing;
		timer_ = 0.0f;
		stage_ = Stage::Fall;
		player.SetPose(PlayerPose::Kind::Flail);
	}

	//だんだん速く落ちて、着いたら尻もちで少し座る
	bool PlayerTumble::Update(float deltaTime, Player& player, Floor& landingFloor, GameDirection& direction)
	{
		if (stage_ == Stage::Idle)
		{
			return true;
		}
		timer_ += deltaTime;
		if (stage_ == Stage::Fall)
		{
			//横はなめらかに、下へはだんだん速く
			const float t = std::clamp(timer_ / DirectionConfig::kTumbleTime, 0.0f, 1.0f);
			const float smooth = t * t * (3.0f - 2.0f * t);
			Vector3 position = Floor::AlongGrid(from_, landing_, smooth);
			position.y = from_.y + (landing_.y - from_.y) * t * t;
			player.SetPosition(position);
			if (t < 1.0f)
			{
				return false;
			}

			//尻もちで足場が揺れてピンクの光が弾ける
			player.Place(landing_);
			player.SetPose(PlayerPose::Kind::Sit);
			landingFloor.Jiggle(landingFloor.GetCellAt(landing_), DirectionConfig::kFallLandJiggle);
			player.PlayLandingParticles();
			direction.Shake(DirectionConfig::kFallLandShake);
			stage_ = Stage::Sit;
			timer_ = 0.0f;
			return false;
		}

		//少し座ってから立ち上がる
		if (timer_ < DirectionConfig::kTumbleSitTime)
		{
			return false;
		}
		player.SetPose(PlayerPose::Kind::Normal);
		stage_ = Stage::Idle;
		return true;
	}
}
