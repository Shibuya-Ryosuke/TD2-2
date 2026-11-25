#include "BossFallFlow.h"
#include "../../Config/GameConfig.h"
#include "../../Stage/Floor/LayerFloors.h"
#include "SpawnPlanner.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Player/Player.h"

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
		timer_ = GameConfig::kEscapeDelay;
	}

	//ボスが下の層へ落ち始める
	void BossFallFlow::BeginDrop(LayerFloors& floors, const Player& player, const Boss& boss)
	{
		//プレイヤーの行き先は下の層でボスが落ちる所の反対側
		floors.EnsureBelow();
		const Floor& below = *floors.GetBelow();
		const Vector3 destination = SpawnPlanner::FindPlayerPosition(below, SpawnPlanner::FindBossPosition(below, boss.GetPosition()));
		descent_.Begin(boss.GetPosition(), player.GetPosition(), destination);
		phase_ = GamePhase::Descent;
	}

	//場面ごとに1フレーム進める
	BossFallFlow::Result BossFallFlow::Update(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera)
	{
		Result result{};
		if (phase_ == GamePhase::Escape)
		{
			UpdateEscape(deltaTime, floors, player, boss, result);
		}
		else if (phase_ == GamePhase::Descent)
		{
			UpdateDescent(deltaTime, floors, player, boss, camera, result);
		}
		else if (phase_ == GamePhase::LandingImpact)
		{
			//少し待ってから足場を飛ばし始める
			timer_ += deltaTime;
			if (timer_ >= GameConfig::kLandingImpactDuration)
			{
				BeginBarrage(floors, result);
			}
		}
		else if (phase_ == GamePhase::TileBarrage)
		{
			UpdateBarrage(deltaTime, floors, boss, result);
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
			floor.Shake(GameConfig::kEscapeDelay - timer_, GameConfig::kShakeDistance, GameConfig::kShakeFrequency);
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

	//ボスが落ちてプレイヤーとカメラも下の層へ
	void BossFallFlow::UpdateDescent(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, StageCamera& camera, Result& result)
	{
		const DescentSequence::Result descent = descent_.Advance(deltaTime, floors.GetCurrent(), *floors.GetBelow(), player, boss, camera);
		if (descent.isBossLandedNow)
		{
			result.bossDamage += GameConfig::kBossFallDamage;
		}
		if (!descent.isFinished)
		{
			return;
		}

		//下の層に着いたら層を1つ進める
		floors.AdvanceLayer();
		timer_ = 0.0f;
		phase_ = GamePhase::LandingImpact;
		result.isArrived = true;
	}

	//上の層に残った足場をボスへ飛ばし始める
	void BossFallFlow::BeginBarrage(LayerFloors& floors, Result& result)
	{
		//飛ばす足場がなければすぐ終わる
		barrage_.Begin(floors.GetUpper());
		phase_ = GamePhase::TileBarrage;
		if (barrage_.IsFinished())
		{
			floors.ReleaseUpper();
			result.isFinished = true;
		}
	}

	//足場を飛ばして届いた分だけダメージ
	void BossFallFlow::UpdateBarrage(float deltaTime, LayerFloors& floors, const Boss& boss, Result& result)
	{
		//上の層に残った足場がボスへ飛んでいく
		Floor* upper = floors.GetUpper();
		if (upper != nullptr)
		{
			const Vector3 target = boss.GetPosition() + Vector3{ 0.0f, 0.75f, 0.0f };
			result.bossDamage += barrage_.Advance(*upper, target, deltaTime);
		}

		//全部届いたら上の層を消して終わる
		if (upper == nullptr || barrage_.IsFinished())
		{
			floors.ReleaseUpper();
			result.isFinished = true;
		}
	}
}
