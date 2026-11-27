#include "BossFallFlow.h"
#include "../../Camera/StageCamera.h"
#include "../../Config/BossConfig.h"
#include "../../Config/BossFallConfig.h"
#include "../../Config/FloorConfig.h"
#include "../../Direction/GameDirection.h"
#include "../../Stage/Floor/LayerFloors.h"
#include "SpawnPlanner.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Player/Player.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		//ゆっくり始まってゆっくり止まる進み具合
		float Smooth(float t)
		{
			const float clamped = std::clamp(t, 0.0f, 1.0f);
			return clamped * clamped * (3.0f - 2.0f * clamped);
		}
	}

	//プレイヤーを動かせない場面か
	bool BossFallFlow::IsLockedPhase(GamePhase phase)
	{
		if (phase == GamePhase::Descent || phase == GamePhase::LandingImpact || phase == GamePhase::TileBarrage)
		{
			return true;
		}
		return false;
	}

	//ボスの島が切り離されたら揺れ始める
	void BossFallFlow::BeginEscape()
	{
		phase_ = GamePhase::Escape;
		timer_ = FloorConfig::kEscapeDelay;
	}

	//段階を変えて、その段階の場面にする
	void BossFallFlow::SetStage(Stage stage)
	{
		stage_ = stage;
		stageTimer_ = 0.0f;
		if (stage == Stage::ZoomIn || stage == Stage::Drop || stage == Stage::Slam || stage == Stage::Widen)
		{
			phase_ = GamePhase::Descent;
		}
		else if (stage == Stage::Hop || stage == Stage::Dive || stage == Stage::Hammer)
		{
			phase_ = GamePhase::LandingImpact;
		}
		else
		{
			phase_ = GamePhase::TileBarrage;
		}
	}

	//ボスをパーツのボスに入れかえてカメラを寄せ始め、プレイヤーは上の層の近くの足場へ着地しなおさせる
	void BossFallFlow::BeginDrop(LayerFloors& floors, Player& player, Boss& boss, const StageCamera& camera)
	{
		floors.EnsureBelow();
		const Floor& current = floors.GetCurrent();
		const Floor& below = *floors.GetBelow();
		upperHeight_ = current.GetHeight();
		lowerHeight_ = below.GetHeight();
		isAdvanced_ = false;

		//跳んでいる途中なら着く先から、近くの立てる足場を探す
		Vector3 stand = player.GetPosition();
		if (player.IsJumping())
		{
			stand = current.GetCellCenter(player.GetJumpDestination());
		}
		settleFrom_ = player.GetPosition();
		settleTo_ = SpawnPlanner::FindNearestStand(current, stand);
		isSettled_ = false;

		//ボスは下の層の着地点へ落ちて、パーツのボスはカメラの方を少しななめに向く
		bossLanding_ = SpawnPlanner::FindBossPosition(below, boss.GetPosition());
		const Vector3 forward = camera.GetViewForward();
		show_.Begin(boss.GetPosition(), ToYawDeg({ -forward.x, 0.0f, -forward.z }) + BossFallConfig::kFaceCameraOffsetDeg);
		boss.SetFade(0.0f);

		camera_.Begin(camera);
		isGetUpDecided_ = false;
		isBossStaysDown_ = false;
		isBossRevealed_ = false;
		SetStage(Stage::ZoomIn);
	}

	//場面ごとに1フレーム進める
	BossFallFlow::Result BossFallFlow::Update(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera, GameDirection& direction)
	{
		Result result{};
		if (phase_ == GamePhase::Escape)
		{
			UpdateEscape(deltaTime, floors, player, boss, camera, result);
			show_.Update(deltaTime);
			return result;
		}

		stageTimer_ += deltaTime;
		if (stage_ == Stage::ZoomIn)
		{
			UpdateZoomIn(floors, player, boss, camera);
		}
		else if (stage_ == Stage::Drop)
		{
			UpdateDrop(deltaTime, floors, boss, camera, direction, result);
		}
		else if (stage_ == Stage::Slam)
		{
			UpdateSlam(camera);
		}
		else if (stage_ == Stage::Widen)
		{
			UpdateWiden(floors, player, camera);
		}
		else if (stage_ == Stage::Hop)
		{
			UpdateHop(deltaTime, player, camera);
		}
		else if (stage_ == Stage::Dive)
		{
			UpdateDive(deltaTime, floors, player, boss, camera, direction);
		}
		else if (stage_ == Stage::Hammer)
		{
			UpdateHammer(floors, player, camera, direction);
		}
		else if (stage_ == Stage::Lift)
		{
			UpdateLift(deltaTime, floors, camera);
		}
		else if (stage_ == Stage::Barrage)
		{
			UpdateBarrage(deltaTime, floors, camera, direction, result);
		}
		else
		{
			UpdateGetUp(floors, boss, camera, result);
		}
		show_.Update(deltaTime);
		return result;
	}

	//揺れて時間が来たらボスごと落とす
	void BossFallFlow::UpdateEscape(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, const StageCamera& camera, Result& result)
	{
		Floor& floor = floors.GetCurrent();
		timer_ -= deltaTime;
		if (timer_ > 0.0f)
		{
			//切り離された床が揺れる
			floor.Shake(FloorConfig::kEscapeDelay - timer_, FloorConfig::kShakeDistance, FloorConfig::kShakeFrequency);
			return;
		}

		//プレイヤーが切り離された床か穴の上にいたら一緒に落ちる
		Cell playerCell = floor.GetCellAt(player.GetPosition());
		if (player.IsJumping())
		{
			playerCell = player.GetJumpDestination();
		}
		if (!floor.IsSolid(playerCell) || floor.IsIsolated(playerCell) || player.GetPosition().y < floor.GetHeight())
		{
			result.isPlayerCaught = true;
			return;
		}

		//切り離された床を穴にしてボスを落とす
		floor.Release();
		boss.Stop();
		BeginDrop(floors, player, boss, camera);
		result.isDropStarted = true;
	}

	//ボスへ寄りながら今の層のタイルを透かして、ボスは足元がなくなって「えっ」となる
	void BossFallFlow::UpdateZoomIn(LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera)
	{
		//跳んでいる途中だったプレイヤーは近くの足場へ小さく跳んで立ちなおす
		if (!isSettled_)
		{
			const float t = std::clamp(stageTimer_ / BossFallConfig::kSettleTime, 0.0f, 1.0f);
			Vector3 position = settleFrom_ + (settleTo_ - settleFrom_) * t;
			position.y += 4.0f * BossFallConfig::kSettleHop * t * (1.0f - t);
			player.SetPosition(position);
			if (t >= 1.0f)
			{
				player.Place(settleTo_);
				isSettled_ = true;
			}
		}

		const float t = stageTimer_ / BossFallConfig::kZoomInTime;
		camera_.Focus(camera, show_.GetFocus(), BossFallConfig::kBossZoomOrthoSize, t);
		SetTopFade(floors, 1.0f + (BossFallConfig::kTileFadeAlpha - 1.0f) * Smooth(t));
		if (t < 1.0f)
		{
			return;
		}

		//寄りきったらじたばた落ち始める
		if (!isSettled_)
		{
			player.Place(settleTo_);
			isSettled_ = true;
		}
		descent_.Begin(boss.GetPosition(), bossLanding_);
		show_.StartFlail();
		SetStage(Stage::Drop);
	}

	//ボスはじたばた落ちてカメラがついていき、着いたらドスンと叩きつけられて層を進める
	void BossFallFlow::UpdateDrop(float deltaTime, LayerFloors& floors, Boss& boss, StageCamera& camera, GameDirection& direction, Result& result)
	{
		const bool isLanded = descent_.Advance(deltaTime, floors.GetCurrent(), boss);
		show_.SetFeet(boss.GetPosition());
		camera_.Focus(camera, show_.GetFocus(), BossFallConfig::kBossZoomOrthoSize, 1.0f);
		if (!isLanded)
		{
			return;
		}

		//着いたら落下ダメージを入れて層を1つ進める
		result.bossDamage += BossConfig::kBossFallDamage;
		floors.AdvanceLayer();
		isAdvanced_ = true;
		result.isArrived = true;

		//ドスンでつぶれて煙が出て、画面と床が揺れる
		show_.Slam();
		direction.Shake(BossFallConfig::kSlamShake);
		Floor& lower = floors.GetCurrent();
		lower.Jiggle(lower.GetCellAt(boss.GetPosition()), BossFallConfig::kSlamJiggle);
		SetStage(Stage::Slam);
	}

	//座りこんで頭を押さえるのを寄ったまま見せる
	void BossFallFlow::UpdateSlam(StageCamera& camera)
	{
		camera_.Focus(camera, show_.GetFocus(), BossFallConfig::kBossZoomOrthoSize, 1.0f);
		if (stageTimer_ < BossFallConfig::kSlamHoldTime)
		{
			return;
		}
		camera_.Cut(camera);
		SetStage(Stage::Widen);
	}

	//両方の層を映す所まで引いて、透かしたタイルを戻し、プレイヤーが跳ぶ穴のふちを決める
	void BossFallFlow::UpdateWiden(LayerFloors& floors, Player& player, StageCamera& camera)
	{
		const float t = stageTimer_ / BossFallConfig::kWideReturnTime;
		camera_.Wide(camera, upperHeight_, lowerHeight_, t);
		SetTopFade(floors, BossFallConfig::kTileFadeAlpha + (1.0f - BossFallConfig::kTileFadeAlpha) * Smooth(t));
		if (t < 1.0f)
		{
			return;
		}
		SetTopFade(floors, 1.0f);

		//いちばん近い穴のふちまで跳んでから、その穴へ飛びこむ
		Vector3 edge = settleTo_;
		diveHole_ = settleTo_;
		float tileSize = 1.0f;
		const Floor* top = GetTopFloor(floors);
		if (top != nullptr)
		{
			FindDiveEdge(*top, player.GetPosition(), edge, diveHole_);
			tileSize = top->GetTileSizeX();
		}
		hop_.Begin(player.GetPosition(), edge, tileSize, player);
		SetStage(Stage::Hop);
	}
}
