#include "BossFallFlow.h"
#include "../../Camera/StageCamera.h"
#include "../../Config/DirectionConfig.h"
#include "../../Config/BossConfig.h"
#include "../../Config/FloorConfig.h"
#include "../../Config/LayerConfig.h"
#include "../../Stage/Floor/LayerFloors.h"
#include "SpawnPlanner.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Player/Player.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
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

	//ボスが下の層へ落ち始めて、プレイヤーは上の層の床の上に立たせておく
	void BossFallFlow::BeginDrop(LayerFloors& floors, Player& player, const Boss& boss)
	{
		floors.EnsureBelow();
		const Floor& current = floors.GetCurrent();
		Vector3 stand = player.GetPosition();
		if (player.IsJumping())
		{
			stand = current.GetCellCenter(player.GetJumpDestination());
		}
		player.Place(SpawnPlanner::FindNearestStand(current, stand));
		descent_.Begin(boss.GetPosition());
		isPlayerDropped_ = false;
		isUpperReleased_ = false;
		isCameraReturning_ = false;
		phase_ = GamePhase::Descent;
	}

	//場面ごとに1フレーム進める
	BossFallFlow::Result BossFallFlow::Update(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera, GameDirection& direction)
	{
		Result result{};
		if (phase_ == GamePhase::Escape)
		{
			UpdateEscape(deltaTime, floors, player, boss, result);
		}
		else if (phase_ == GamePhase::Descent)
		{
			UpdateDescent(deltaTime, floors, boss, camera, result);
		}
		else if (phase_ == GamePhase::LandingImpact)
		{
			//少し待ってから足場を飛ばし始める
			timer_ += deltaTime;
			if (timer_ >= LayerConfig::kLandingImpactDuration)
			{
				BeginBarrage(floors);
			}
		}
		else if (phase_ == GamePhase::TileBarrage && isCameraReturning_)
		{
			UpdateCameraReturn(deltaTime, floors, camera, result);
		}
		else if (phase_ == GamePhase::TileBarrage)
		{
			UpdateBarrage(deltaTime, floors, player, boss, direction, result);
		}
		return result;
	}

	//揺れて時間が来たらボスごと落とす
	void BossFallFlow::UpdateEscape(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, Result& result)
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
		BeginDrop(floors, player, boss);
		result.isDropStarted = true;
	}

	//ボスが落ちて、カメラは上の層と下の層の両方が見える所まで下がる
	void BossFallFlow::UpdateDescent(float deltaTime, LayerFloors& floors, Boss& boss, StageCamera& camera, Result& result)
	{
		const DescentSequence::Result descent = descent_.Advance(deltaTime, floors.GetCurrent(), *floors.GetBelow(), boss, camera);
		if (descent.isBossLandedNow)
		{
			result.bossDamage += BossConfig::kBossFallDamage;
		}
		if (!descent.isFinished)
		{
			return;
		}

		//ボスが着いたら層を1つ進めて、プレイヤーは上の層に立ったまま
		floors.AdvanceLayer();
		timer_ = 0.0f;
		phase_ = GamePhase::LandingImpact;
		result.isArrived = true;
	}

	//上の層に残った足場をボスへ飛ばし始める
	void BossFallFlow::BeginBarrage(LayerFloors& floors)
	{
		barrage_.Begin(floors.GetUpper());
		phase_ = GamePhase::TileBarrage;
	}

	//足場を飛ばして届いた分だけダメージ、プレイヤーの足場が飛び立ったらプレイヤーもばたばた落ちる
	void BossFallFlow::UpdateBarrage(float deltaTime, LayerFloors& floors, Player& player, const Boss& boss, GameDirection& direction, Result& result)
	{
		Floor* upper = floors.GetUpper();
		if (upper != nullptr)
		{
			const Vector3 target = boss.GetPosition() + Vector3{ 0.0f, 0.75f, 0.0f };
			result.bossDamage += barrage_.Advance(*upper, target, deltaTime);
		}

		//立っている足場が飛び立ったら、下の層のボスの反対側へ落ちる
		bool isFootingGone = upper == nullptr || barrage_.IsFinished();
		if (upper != nullptr && IsFootingGone(*upper, player))
		{
			isFootingGone = true;
		}
		if (!isPlayerDropped_ && isFootingGone)
		{
			isPlayerDropped_ = true;
			tumble_.Begin(player.GetPosition(), playerLanding_, player);
		}
		if (tumble_.IsActive())
		{
			tumble_.Update(deltaTime, player, floors.GetCurrent(), direction);
		}

		//全部届いたら上の層を消す
		if (barrage_.IsFinished() && !isUpperReleased_)
		{
			isUpperReleased_ = true;
			floors.ReleaseUpper();
		}

		//足場が全部届いてプレイヤーも立ち上がったらカメラを戻し始める
		if (isUpperReleased_ && isPlayerDropped_ && !tumble_.IsActive())
		{
			isCameraReturning_ = true;
			returnTimer_ = 0.0f;
			returnFromHeight_ = 0.0f;
			returnFromSize_ = 0.0f;
		}
	}

	//カメラを下の層のいつもの映し方へ戻して終わる
	void BossFallFlow::UpdateCameraReturn(float deltaTime, LayerFloors& floors, StageCamera& camera, Result& result)
	{
		if (returnTimer_ <= 0.0f)
		{
			returnFromHeight_ = camera.GetFloorHeight();
			returnFromSize_ = camera.GetOrthoSize();
		}
		returnTimer_ += deltaTime;
		const float t = std::clamp(returnTimer_ / DirectionConfig::kBossFallViewReturnTime, 0.0f, 1.0f);
		const float smooth = t * t * (3.0f - 2.0f * t);
		const float height = floors.GetCurrent().GetHeight();
		camera.SetFloorHeight(returnFromHeight_ + (height - returnFromHeight_) * smooth);
		camera.SetOrthoSize(returnFromSize_ + (camera.GetBaseOrthoSize() - returnFromSize_) * smooth);
		if (t < 1.0f)
		{
			return;
		}
		camera.ResetShot();
		camera.SetFloorHeight(height);
		isCameraReturning_ = false;
		result.isFinished = true;
	}

	//プレイヤーの立っている足場が飛び立ったか、もうないか
	bool BossFallFlow::IsFootingGone(const Floor& upper, const Player& player) const
	{
		const Cell cell = upper.GetCellAt(player.GetPosition());
		if (!upper.IsSolid(cell))
		{
			return true;
		}
		return barrage_.IsLaunched(cell);
	}
}
