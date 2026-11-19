#include "BossFallFlow.h"
#include "../../Config/GameConfig.h"
#include "../../Stage/LayerFloors.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Player/Player.h"

using namespace RyoEngine;

namespace App
{
	// プレイヤーを動かせない場面か
	bool BossFallFlow::IsLockedPhase(GamePhase phase)
	{
		if (phase == GamePhase::Descent || phase == GamePhase::LandingImpact || phase == GamePhase::TileBarrage)
		{
			return true;
		}
		return false;
	}

	// ボスの島が切り離された: 揺れ始める
	void BossFallFlow::BeginEscape()
	{
		phase_ = GamePhase::Escape;
		timer_ = GameConfig::kEscapeDelay;
	}

	// ボスが下の層へ落ち始める
	void BossFallFlow::BeginDrop(LayerFloors& floors, const Player& player, const Boss& boss)
	{
		// プレイヤーの行き先 = 下の層の最初の位置
		floors.EnsureBelow();
		const Vector3 destination = LayerFloors::GetSpawnPosition(*floors.GetBelow(), GameConfig::kPlayerSpawnX, GameConfig::kPlayerSpawnZ);
		descent_.Begin(boss.GetPosition(), player.GetPosition(), destination);
		phase_ = GamePhase::Descent;
	}

	// 1フレーム進める (場面ごとに)
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
			// 少し待ってから、足場を飛ばし始める
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

	// 揺れる → 時間が来たらボスごと落とす
	void BossFallFlow::UpdateEscape(float deltaTime, LayerFloors& floors, Player& player, Boss& boss, Result& result)
	{
		Floor& floor = floors.GetCurrent();
		timer_ -= deltaTime;
		if (timer_ > 0.0f)
		{
			// 切り離された床が揺れる
			floor.Shake(GameConfig::kEscapeDelay - timer_, GameConfig::kShakeDistance, GameConfig::kShakeFrequency);
			return;
		}

		// 落ちる。プレイヤーが切り離された床・穴の上にいたら、一緒に落ちる (跳んでいる途中なら着地するマスで調べる)
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

		// 切り離された床を穴にして、ボスを落とす
		floor.Release();
		boss.Stop();
		BeginDrop(floors, player, boss);
		result.isDropStarted = true;
	}

	// ボスが落ちる・プレイヤーとカメラも下の層へ
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

		// 下の層に着いた: 層を1つ進める (配置・回復などは Game がする)
		floors.AdvanceLayer();
		timer_ = 0.0f;
		phase_ = GamePhase::LandingImpact;
		result.isArrived = true;
	}

	// 上の層に残った足場を、ボスへ飛ばし始める
	void BossFallFlow::BeginBarrage(LayerFloors& floors, Result& result)
	{
		// 飛ばす足場を用意する。1つもなければすぐ終わり
		barrage_.Begin(floors.GetUpper());
		phase_ = GamePhase::TileBarrage;
		if (barrage_.IsFinished())
		{
			floors.ReleaseUpper();
			result.isFinished = true;
		}
	}

	// 足場を飛ばして、届いた分だけダメージ
	void BossFallFlow::UpdateBarrage(float deltaTime, LayerFloors& floors, const Boss& boss, Result& result)
	{
		// 上の層に残った足場が、ボスへ飛んでいく
		Floor* upper = floors.GetUpper();
		if (upper != nullptr)
		{
			const Vector3 target = boss.GetPosition() + Vector3{ 0.0f, 0.75f, 0.0f };
			result.bossDamage += barrage_.Advance(*upper, target, deltaTime);
		}

		// 全部届いたら、上の層を消して終わり
		if (upper == nullptr || barrage_.IsFinished())
		{
			floors.ReleaseUpper();
			result.isFinished = true;
		}
	}
}
