#include "TutorialFinale.h"
#include "../Game/SpawnPlanner.h"
#include "../../Camera/StageCamera.h"
#include "../../Config/DirectionConfig.h"
#include "../../Config/TutorialConfig.h"
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

		//aからbへtだけ進めた値
		float LerpFloat(float a, float b, float t)
		{
			return a + (b - a) * t;
		}

		//足元から胸の高さ
		Vector3 ChestOf(const Vector3& feet)
		{
			return feet + Vector3{ 0.0f, DirectionConfig::kClearFocusHeight, 0.0f };
		}
	}

	//始まっていない状態にする
	void TutorialFinale::Initialize()
	{
		stage_ = Stage::Idle;
	}

	//寝ているボスの反対側の床へじたばた落ち始める
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
			UpdateFall(floors, player, camera, direction);
		}
		else if (stage_ == Stage::Sit)
		{
			//寄ったまま尻もちを見せてから、足場が飛び始める
			camera.SetFocusOffset(camera.GetCenteringOffset(ChestOf(player.GetPosition())));
			if (timer_ >= TutorialConfig::kLandSitTime)
			{
				barrage_.Begin(floors.GetUpper());
				player.SetPose(PlayerPose::Kind::Normal);
				stage_ = Stage::Barrage;
				timer_ = 0.0f;
			}
		}
		else if (stage_ == Stage::Barrage)
		{
			result.bossDamage = UpdateBarrage(deltaTime, floors, boss, camera, direction);
		}
		else if (stage_ == Stage::ZoomBack)
		{
			UpdateZoomBack(camera);
			if (stage_ == Stage::Done)
			{
				result.isFinished = true;
			}
		}
		return result;
	}

	//寄りながらじたばた落ちて、着いたらチュートリアルの床を上の層にして尻もち
	void TutorialFinale::UpdateFall(LayerFloors& floors, Player& player, StageCamera& camera, GameDirection& direction)
	{
		const float t = std::clamp(timer_ / TutorialConfig::kDescentTime, 0.0f, 1.0f);
		const float smooth = SmoothStep(t);
		Vector3 position = Floor::AlongGrid(fallStart_, landing_, smooth);
		position.y = LerpFloat(fallStart_.y, landing_.y, t * t);
		player.SetPosition(position);

		//カメラは下の層へ降りながらプレイヤーへ寄る
		const float zoom = SmoothStep(std::clamp(timer_ / DirectionConfig::kFallZoomTime, 0.0f, 1.0f));
		camera.SetFloorHeight(LerpFloat(startFloorHeight_, landingFloorHeight_, smooth));
		camera.SetOrthoSize(LerpFloat(camera.GetBaseOrthoSize(), DirectionConfig::kFallZoomOrthoSize, zoom));
		camera.SetFocusOffset(camera.GetCenteringOffset(ChestOf(position)) * zoom);
		if (t < 1.0f)
		{
			return;
		}

		//どすんと尻もちでハートが飛ぶ
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
	}

	//カメラを寝ているボスへ向けて、足場が当たるたびにつぶれてハートと数字を出す
	int32_t TutorialFinale::UpdateBarrage(float deltaTime, LayerFloors& floors, Boss& boss, StageCamera& camera, GameDirection& direction)
	{
		const float look = SmoothStep(std::clamp(timer_ / TutorialConfig::kBarrageLookTime, 0.0f, 1.0f));
		camera.SetOrthoSize(LerpFloat(DirectionConfig::kFallZoomOrthoSize, TutorialConfig::kBarrageOrthoSize, look));
		const Vector3 playerFocus = camera.GetCenteringOffset(ChestOf(landing_));
		const Vector3 bossFocus = camera.GetCenteringOffset(bossRest_ + Vector3{ 0.0f, TutorialConfig::kBarrageFocusHeight, 0.0f });
		camera.SetFocusOffset(playerFocus + (bossFocus - playerFocus) * look);

		//カメラがボスを向いてから足場を飛ばす
		if (timer_ < TutorialConfig::kBarrageLookTime)
		{
			return 0;
		}
		int32_t damage = 0;
		Floor* upper = floors.GetUpper();
		if (upper != nullptr)
		{
			const Vector3 target = bossRest_ + Vector3{ 0.0f, TutorialConfig::kBarrageTargetHeight, 0.0f };
			damage = barrage_.Advance(*upper, target, deltaTime);
		}
		if (damage > 0)
		{
			HitBoss(boss, floors, direction);
		}

		//全部当たったら上の層を消して、少し余韻を見せてから戻る
		if (upper == nullptr || barrage_.IsFinished())
		{
			floors.ReleaseUpper();
			if (timer_ >= TutorialConfig::kBarrageLookTime + TutorialConfig::kBarrageMinTime)
			{
				zoomFrom_ = camera.GetOrthoSize();
				focusFrom_ = bossFocus;
				stage_ = Stage::ZoomBack;
				timer_ = 0.0f;
			}
		}
		return damage;
	}

	//当たった足場の分だけつぶれて揺れる
	void TutorialFinale::HitBoss(Boss& boss, LayerFloors& floors, GameDirection& direction)
	{
		boss.Flinch(1.0f);
		direction.Shake(TutorialConfig::kBarrageHitShake);
		Floor& current = floors.GetCurrent();
		current.Jiggle(current.GetCellAt(bossRest_), TutorialConfig::kBarrageHitJiggle);
	}

	//いつもの映し方へ戻して終わる
	void TutorialFinale::UpdateZoomBack(StageCamera& camera)
	{
		const float back = SmoothStep(std::clamp(timer_ / TutorialConfig::kFinaleZoomBackTime, 0.0f, 1.0f));
		camera.SetOrthoSize(LerpFloat(zoomFrom_, camera.GetBaseOrthoSize(), back));
		camera.SetFocusOffset(focusFrom_ * (1.0f - back));
		if (back >= 1.0f)
		{
			camera.ResetShot();
			stage_ = Stage::Done;
		}
	}
}
