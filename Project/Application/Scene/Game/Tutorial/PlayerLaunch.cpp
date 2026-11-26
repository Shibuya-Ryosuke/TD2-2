#include "PlayerLaunch.h"
#include "../../../Config/TutorialConfig.h"
#include "../../../Object/Player/Player.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	//飛ばし始めて、空中ではじたばたさせる
	void PlayerLaunch::Begin(const Vector3& from, const Vector3& target)
	{
		from_ = from;
		target_ = target;
		timer_ = 0.0f;
		isActive_ = true;
	}

	//山なりに飛ばして、着いたら置き直す
	bool PlayerLaunch::Update(float deltaTime, Player& player)
	{
		if (!isActive_)
		{
			return true;
		}
		timer_ += deltaTime;
		const float t = std::clamp(timer_ / TutorialConfig::kLaunchTime, 0.0f, 1.0f);

		//横はまっすぐ、縦は高く上がって着地する所へ下りる
		Vector3 position = from_ + (target_ - from_) * t;
		position.y += 4.0f * TutorialConfig::kLaunchHeight * t * (1.0f - t);
		player.SetPosition(position);
		player.SetPose(PlayerPose::Kind::Flail);
		if (t < 1.0f)
		{
			return false;
		}
		player.Place(target_);
		isActive_ = false;
		return true;
	}
}
