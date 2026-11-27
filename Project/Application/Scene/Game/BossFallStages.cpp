#include "BossFallFlow.h"
#include "../../Camera/StageCamera.h"
#include "../../Config/BossFallConfig.h"
#include "../../Direction/BossFall/BossFallParticles.h"
#include "../../Direction/GameDirection.h"
#include "../../Stage/Floor/LayerFloors.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Player/Player.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numbers>

using namespace RyoEngine;

//BossFallFlowのうち、プレイヤーが穴のふちへ跳んでからボスが起き上がるまでの段階と、使う計算
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

	//水平な向きをパーツのボスの向きの度数にする
	float BossFallFlow::ToYawDeg(const Vector3& direction)
	{
		return std::atan2(direction.x, direction.z) * (180.0f / std::numbers::pi_v<float>);
	}

	//穴のとなりにある立てるマスのうちfromにいちばん近いものと、そのとなりの穴のマスのまん中を探す
	bool BossFallFlow::FindDiveEdge(const Floor& floor, const Vector3& from, Vector3& edge, Vector3& hole)
	{
		const std::array<Cell, 4> directions = { { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } } };
		float best = std::numeric_limits<float>::infinity();
		bool isFound = false;
		for (int32_t z = 0; z < floor.GetDepth(); ++z)
		{
			for (int32_t x = 0; x < floor.GetWidth(); ++x)
			{
				const Cell cell = { x, z };
				if (!floor.IsSolid(cell))
				{
					continue;
				}
				for (const Cell& direction : directions)
				{
					const Cell next = cell + direction;
					if (!floor.Contains(next) || floor.IsSolid(next))
					{
						continue;
					}
					const Vector3 center = floor.GetCellCenter(cell);
					const float distance = std::hypot(center.x - from.x, center.z - from.z);
					if (distance < best)
					{
						best = distance;
						isFound = true;
						edge = center;
						hole = floor.GetCellCenter(next);
					}
				}
			}
		}
		return isFound;
	}

	//ボスが落ちる前に立っていた層で、層を進めたあとは上の層
	Floor* BossFallFlow::GetTopFloor(LayerFloors& floors) const
	{
		if (isAdvanced_)
		{
			return floors.GetUpper();
		}
		return &floors.GetCurrent();
	}

	//ボスが落ちる前に立っていた層のタイルを透かす
	void BossFallFlow::SetTopFade(LayerFloors& floors, float alpha) const
	{
		Floor* top = GetTopFloor(floors);
		if (top != nullptr)
		{
			top->SetFade(alpha);
		}
	}

	//引いた映し方のまま穴のふちまで跳んで、ためきったら飛びこむ
	void BossFallFlow::UpdateHop(float deltaTime, Player& player, StageCamera& camera)
	{
		camera_.Wide(camera, upperHeight_, lowerHeight_, 1.0f);
		if (!hop_.Update(deltaTime, player))
		{
			return;
		}
		dive_.Begin(player.GetPosition(), diveHole_, playerLanding_, player);
		camera_.Cut(camera);
		SetStage(Stage::Dive);
	}

	//えいっと飛びこむ間にカメラがプレイヤーへ寄ってタイルが透け、寄ったまま下の層の着地点まで落ちる
	void BossFallFlow::UpdateDive(float deltaTime, LayerFloors& floors, Player& player, const Boss& boss, StageCamera& camera, GameDirection& direction)
	{
		const bool isLanded = dive_.Update(deltaTime, player);
		const float t = stageTimer_ / BossFallConfig::kDiveHopTime;
		const Vector3 focus = dive_.GetPosition() + Vector3{ 0.0f, BossFallConfig::kDiveFocusHeight, 0.0f };
		camera_.Focus(camera, focus, BossFallConfig::kDiveZoomOrthoSize, t);
		SetTopFade(floors, 1.0f + (BossFallConfig::kTileFadeAlpha - 1.0f) * Smooth(t));
		if (!isLanded)
		{
			return;
		}

		//足から着地して、ボスの方を向いてすぐハンマーを振る
		player.Place(playerLanding_);
		player.SetFacingDirection(boss.GetPosition() - playerLanding_);
		Floor& lower = floors.GetCurrent();
		lower.Jiggle(lower.GetCellAt(playerLanding_), BossFallConfig::kLandJiggle);
		player.PlayLandingParticles();
		direction.Shake(BossFallConfig::kLandShake);
		player.StartSwing();
		SetStage(Stage::Hammer);
	}

	//寄ったままハンマーを振りながらタイルを戻し、当たった瞬間に上の足場を浮かせ始める
	void BossFallFlow::UpdateHammer(LayerFloors& floors, Player& player, StageCamera& camera, GameDirection& direction)
	{
		const Vector3 focus = player.GetPosition() + Vector3{ 0.0f, BossFallConfig::kDiveFocusHeight, 0.0f };
		camera_.Focus(camera, focus, BossFallConfig::kDiveZoomOrthoSize, 1.0f);
		SetTopFade(floors, BossFallConfig::kTileFadeAlpha + (1.0f - BossFallConfig::kTileFadeAlpha) * Smooth(stageTimer_ / BossFallConfig::kWideReturnTime));
		if (!player.WasHammerImpact() && stageTimer_ < BossFallConfig::kHammerTimeout)
		{
			return;
		}

		//ドン！で床が揺れて、光の輪とハートが広がる
		Floor& lower = floors.GetCurrent();
		const Cell front = lower.GetCellAt(player.GetPosition()) + player.GetFacing();
		Vector3 contact = lower.GetCellCenter(front);
		contact.y = lower.GetHeight();
		lower.Jiggle(front, BossFallConfig::kHammerJiggle);
		direction.Shake(BossFallConfig::kHammerShake);
		player.BurstHeartsAt(contact, BossFallConfig::kHammerHearts, BossFallConfig::kHammerHeartSize);
		BossFallParticles::HammerRing(contact);

		//合図で上の足場がふわっと浮き始める
		SetTopFade(floors, 1.0f);
		Floor* upper = floors.GetUpper();
		if (upper != nullptr)
		{
			lift_.Begin(*upper);
		}
		camera_.Cut(camera);
		SetStage(Stage::Lift);
	}

	//両方の層を映す所まで引きながら、上の足場が浮いて光る
	void BossFallFlow::UpdateLift(float deltaTime, LayerFloors& floors, StageCamera& camera)
	{
		camera_.Wide(camera, upperHeight_, lowerHeight_, stageTimer_ / BossFallConfig::kLiftTime);
		Floor* upper = floors.GetUpper();
		if (upper != nullptr)
		{
			lift_.Update(deltaTime, *upper);
		}
		if (stageTimer_ < BossFallConfig::kLiftTime)
		{
			return;
		}
		barrage_.Begin(upper);
		SetStage(Stage::Barrage);
	}

	//足場がいっせいにボスへ降って当たるたびにむぎゅっとつぶれ、最後の1枚でいちばん大きくつぶれる
	void BossFallFlow::UpdateBarrage(float deltaTime, LayerFloors& floors, StageCamera& camera, GameDirection& direction, Result& result)
	{
		camera_.Wide(camera, upperHeight_, lowerHeight_, 1.0f);
		Floor* upper = floors.GetUpper();
		if (upper != nullptr)
		{
			result.bossDamage += barrage_.Advance(*upper, show_.GetHitTarget(), deltaTime);
			if (barrage_.GetArrivedNow() > 0)
			{
				float squash = BossFallConfig::kHitSquash;
				if (barrage_.IsFinished())
				{
					squash = BossFallConfig::kLastHitSquash;
				}
				show_.Hit(squash);
				direction.Shake(BossFallConfig::kHitShake);
			}
		}
		if (upper != nullptr && !barrage_.IsFinished())
		{
			return;
		}
		floors.ReleaseUpper();
		camera_.Cut(camera);
		SetStage(Stage::GetUp);
	}

	//ボスが起き上がりながらカメラが下の層のいつもの映し方へ戻り、倒しきったら倒れたまま
	void BossFallFlow::UpdateGetUp(LayerFloors& floors, Boss& boss, StageCamera& camera, Result& result)
	{
		//足場のダメージが入ったあとで、起き上がるか倒れたままか決める
		if (!isGetUpDecided_)
		{
			isGetUpDecided_ = true;
			isBossStaysDown_ = boss.IsDefeated();
			if (!isBossStaysDown_)
			{
				show_.BeginGetUp(ToYawDeg(boss.GetForward()));
			}
		}
		camera_.Normal(camera, floors.GetCurrent().GetHeight(), stageTimer_ / BossFallConfig::kReturnTime);

		//起き上がりきったらいつものボスに戻す
		if (!isBossStaysDown_ && !isBossRevealed_ && show_.IsGetUpFinished())
		{
			isBossRevealed_ = true;
			boss.SetFade(1.0f);
			show_.Hide();
		}
		const bool isBossDone = isBossStaysDown_ || isBossRevealed_;
		if (stageTimer_ < BossFallConfig::kReturnTime || !isBossDone)
		{
			return;
		}
		camera_.Finish(camera, floors.GetCurrent().GetHeight());
		result.isFinished = true;
		phase_ = GamePhase::Battle;
	}
}
