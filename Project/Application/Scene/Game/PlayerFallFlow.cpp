#include "PlayerFallFlow.h"
#include "SpawnPlanner.h"
#include "../../Camera/StageCamera.h"
#include "../../Config/DirectionConfig.h"
#include "../../Direction/GameDirection.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Player/Player.h"
#include "../../Stage/Floor/LayerFloors.h"
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

		//aからbへtの割合だけ進んだ値
		float LerpFloat(float a, float b, float t)
		{
			return a + (b - a) * t;
		}

		//プレイヤーの胸の高さ
		Vector3 ChestOf(const Vector3& feet)
		{
			return { feet.x, feet.y + DirectionConfig::kClearFocusHeight, feet.z };
		}
	}

	//落ちた所の真下の床と、ボスが降りる反対側の床を決めて始める
	void PlayerFallFlow::Begin(LayerFloors& floors, Player& player, Boss& boss)
	{
		floors.EnsureBelow();
		const Floor& below = *floors.GetBelow();
		fallStart_ = player.GetPosition();
		landing_ = SpawnPlanner::FindNearestStand(below, fallStart_);
		bossLanding_ = SpawnPlanner::FindPlayerPosition(below, landing_);
		startFloorHeight_ = floors.GetCurrent().GetHeight();
		landingFloorHeight_ = below.GetHeight();
		isBossLanded_ = false;
		stage_ = Stage::Fall;
		timer_ = 0.0f;

		//ボスは止めて隠し、プレイヤーはじたばた
		boss.Stop();
		boss.SetActive(false);
		player.SetPose(PlayerPose::Kind::Flail);
	}

	//段階ごとに1フレーム進める
	bool PlayerFallFlow::Update(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera, GameDirection& direction)
	{
		timer_ += deltaTime;
		if (stage_ == Stage::Fall)
		{
			UpdateFall(floors, player, camera, direction);
			return false;
		}
		if (stage_ == Stage::Sit)
		{
			//寄ったまま尻もちを見せる
			camera.SetFocusOffset(camera.GetCenteringOffset(ChestOf(player.GetPosition())));
			if (timer_ >= DirectionConfig::kFallSitTime)
			{
				stage_ = Stage::BossDrop;
				timer_ = 0.0f;
				boss.SetActive(true);
				boss.Place(floors.GetCurrent(), bossLanding_);
			}
			return false;
		}
		UpdateBossDrop(floors, player, boss, camera, direction);
		return timer_ >= DirectionConfig::kFallBossDropTime + DirectionConfig::kFallZoomOutTime;
	}

	//寄りながらじたばた落ちて、着いたら尻もち
	void PlayerFallFlow::UpdateFall(LayerFloors& floors, Player& player, StageCamera& camera, GameDirection& direction)
	{
		const float t = std::clamp(timer_ / DirectionConfig::kFallTime, 0.0f, 1.0f);

		//横はなめらかに、下へはだんだん速く落ちる
		const float smooth = SmoothStep(t);
		Vector3 position = Floor::AlongGrid(fallStart_, landing_, smooth);
		position.y = LerpFloat(fallStart_.y, landing_.y, t * t);
		player.SetPosition(position);

		//落ちた層の床はぽぽぽっと小さくなって消える
		const float shrink = std::clamp(timer_ / DirectionConfig::kFallFloorShrinkTime, 0.0f, 1.0f);
		floors.GetCurrent().ShrinkAll(1.0f - shrink);

		//カメラは下の層へ降りながらプレイヤーへ寄る
		const float zoom = SmoothStep(std::clamp(timer_ / DirectionConfig::kFallZoomTime, 0.0f, 1.0f));
		camera.SetFloorHeight(LerpFloat(startFloorHeight_, landingFloorHeight_, smooth));
		camera.SetOrthoSize(LerpFloat(camera.GetBaseOrthoSize(), DirectionConfig::kFallZoomOrthoSize, zoom));
		camera.SetFocusOffset(camera.GetCenteringOffset(ChestOf(position)) * zoom);
		if (t < 1.0f)
		{
			return;
		}

		//着いたら層を進めて、上の層はもう使わないので消す
		floors.AdvanceLayer();
		floors.ReleaseUpper();
		camera.SetFloorHeight(floors.GetCurrent().GetHeight());
		player.Place(landing_);
		player.SetPose(PlayerPose::Kind::Sit);

		//どすんと揺れてハートが飛ぶ
		Floor& current = floors.GetCurrent();
		current.Jiggle(current.GetCellAt(landing_), DirectionConfig::kFallLandJiggle);
		player.BurstHearts(DirectionConfig::kFallLandHearts);
		direction.Shake(DirectionConfig::kFallLandShake);
		stage_ = Stage::Sit;
		timer_ = 0.0f;
	}

	//ボスが上から降りてきて、着いたらカメラを戻す
	void PlayerFallFlow::UpdateBossDrop(LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera, GameDirection& direction)
	{
		//ボスはだんだん速く真下へ降りる
		const float drop = std::clamp(timer_ / DirectionConfig::kFallBossDropTime, 0.0f, 1.0f);
		const float height = DirectionConfig::kFallBossDropHeight * (1.0f - drop * drop);
		boss.SetPosition({ bossLanding_.x, bossLanding_.y + height, bossLanding_.z });

		//着いた瞬間にどすん
		if (drop >= 1.0f && !isBossLanded_)
		{
			isBossLanded_ = true;
			Floor& current = floors.GetCurrent();
			current.Jiggle(current.GetCellAt(bossLanding_), DirectionConfig::kJiggleBossPower);
			direction.Shake(DirectionConfig::kFallBossLandShake);
			player.SetPose(PlayerPose::Kind::Normal);
		}

		//ボスが降りている間に寄ったカメラをいつもの映し方へ戻す
		const float back = SmoothStep(std::clamp(timer_ / (DirectionConfig::kFallBossDropTime + DirectionConfig::kFallZoomOutTime), 0.0f, 1.0f));
		camera.SetOrthoSize(LerpFloat(DirectionConfig::kFallZoomOrthoSize, camera.GetBaseOrthoSize(), back));
		camera.SetFocusOffset(camera.GetCenteringOffset(ChestOf(player.GetPosition())) * (1.0f - back));
		if (back >= 1.0f)
		{
			camera.SetOrthoSize(0.0f);
			camera.SetFocusOffset({ 0.0f, 0.0f, 0.0f });
		}
	}
}
