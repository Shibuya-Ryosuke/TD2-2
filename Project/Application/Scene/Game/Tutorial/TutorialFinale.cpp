#include "TutorialFinale.h"
#include "../SpawnPlanner.h"
#include "../../../Camera/StageCamera.h"
#include "../../../Config/DirectionConfig.h"
#include "../../../Config/TutorialConfig.h"
#include "../../../Direction/GameDirection.h"
#include "../../../Object/Boss/Boss.h"
#include "../../../Object/Player/Player.h"
#include "../../../Stage/Floor/LayerFloors.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		//ゆっくり始まってゆっくり止まる進み具合
		float SmoothStep(float t)
		{
			return t * t * (3.0f - 2.0f * t);
		}
	}

	//寝ているボスの反対側の床へ落ち始める
	void TutorialFinale::Begin(LayerFloors& floors, Player& player, const Boss& boss)
	{
		floors.EnsureBelow();
		const Floor& below = *floors.GetBelow();
		bossRest_ = boss.GetPosition();
		fallStart_ = player.GetPosition();
		landing_ = SpawnPlanner::FindPlayerPosition(below, bossRest_);
		startFloorHeight_ = floors.GetCurrent().GetHeight();
		landingFloorHeight_ = below.GetHeight();
		stage_ = Stage::Fall;
		timer_ = 0.0f;
		player.SetPose(PlayerPose::Kind::Flail);
	}

	//段階ごとに1フレーム進める
	TutorialFinale::Result TutorialFinale::Update(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera, GameDirection& direction)
	{
		Result result{};
		timer_ += deltaTime;
		if (stage_ == Stage::Fall)
		{
			//横はなめらかに、下へはだんだん速くじたばた落ちて、カメラも下の層へ
			const float t = std::clamp(timer_ / TutorialConfig::kDescentTime, 0.0f, 1.0f);
			const float smooth = SmoothStep(t);
			Vector3 position = Floor::AlongGrid(fallStart_, landing_, smooth);
			position.y = fallStart_.y + (landing_.y - fallStart_.y) * t * t;
			player.SetPosition(position);
			camera.SetFloorHeight(startFloorHeight_ + (landingFloorHeight_ - startFloorHeight_) * smooth);
			if (t < 1.0f)
			{
				return result;
			}

			//着いたらチュートリアルの床を上の層にして、どすんと尻もち
			floors.AdvanceLayer();
			camera.SetFloorHeight(floors.GetCurrent().GetHeight());
			player.Place(landing_);
			player.SetPose(PlayerPose::Kind::Sit);
			Floor& current = floors.GetCurrent();
			current.Jiggle(current.GetCellAt(landing_), DirectionConfig::kFallLandJiggle);
			player.BurstHearts(DirectionConfig::kFallLandHearts);
			direction.Shake(DirectionConfig::kFallLandShake);
			stage_ = Stage::Sit;
			timer_ = 0.0f;
			return result;
		}
		if (stage_ == Stage::Sit)
		{
			//少し待ってから残った足場をボスへ飛ばす
			if (timer_ >= TutorialConfig::kLandSitTime)
			{
				barrage_.Begin(floors.GetUpper());
				stage_ = Stage::Barrage;
				timer_ = 0.0f;
				player.SetPose(PlayerPose::Kind::Normal);
			}
			return result;
		}
		if (stage_ == Stage::Barrage)
		{
			//寝ているボスへ足場が飛んで、届いた分だけダメージ
			Floor* upper = floors.GetUpper();
			if (upper != nullptr)
			{
				const Vector3 target = bossRest_ + Vector3{ 0.0f, 0.75f, 0.0f };
				result.bossDamage = barrage_.Advance(*upper, target, deltaTime);
			}
			if (upper == nullptr || barrage_.IsFinished())
			{
				floors.ReleaseUpper();
				stage_ = Stage::Wake;
				timer_ = 0.0f;
			}
			return result;
		}
		if (stage_ == Stage::Wake)
		{
			//はっと起きて起き上がり、ぴょんと跳ねて着地
			const float t = std::clamp(timer_ / TutorialConfig::kWakeTime, 0.0f, 1.0f);
			const float rise = SmoothStep(std::clamp(t * 2.0f - 0.5f, 0.0f, 1.0f));
			boss.SetSleep(1.0f - rise);
			const float hop = std::clamp(t * 2.0f - 1.0f, 0.0f, 1.0f);
			boss.SetPosition({ bossRest_.x, bossRest_.y + 4.0f * TutorialConfig::kWakeHopHeight * hop * (1.0f - hop), bossRest_.z });
			if (t >= 1.0f)
			{
				boss.SetSleep(0.0f);
				boss.SetPosition(bossRest_);
				Floor& current = floors.GetCurrent();
				current.Jiggle(current.GetCellAt(bossRest_), DirectionConfig::kJiggleBossPower);
				direction.Shake(TutorialConfig::kWakeShake);
				stage_ = Stage::Done;
				result.isFinished = true;
			}
		}
		return result;
	}

	//寝ている間はZzz、起きるときはびっくり
	const char* TutorialFinale::GetBossText() const
	{
		if (stage_ == Stage::Fall || stage_ == Stage::Sit || stage_ == Stage::Barrage)
		{
			return "Zzz...";
		}
		if (stage_ == Stage::Wake)
		{
			return "!?";
		}
		return nullptr;
	}
}
