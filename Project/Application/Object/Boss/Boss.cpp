#include "Boss.h"
#include "../../Config/GameConfig.h"
#include "../../Stage/Grid/FloorPathFinder.h"
#include "BossPlanner.h"
#include "BossDetail.h"
#include <algorithm>
#include <array>
#include <string>
#include <cmath>
#include <numbers>

using namespace RyoEngine;
using namespace App::BossDetail;

namespace App
{
	void Boss::Initialize()
	{
		//体と拳と攻撃の予告のモデルはBossViewが作る
		view_.Initialize();
		fistLocal_ = kFistRest;
		RegisterParams();
	}

	//ボスの調整する値をまとめてParamEditorに登録する
	void Boss::RegisterParams() {
		ParamEditor::BeginGroup("Boss");
		groundPunch_.RegisterParams();
		view_.RegisterParams();

		//層ごとの強さ。6層目からは1層目に戻る
		ParamEditor::BeginGroup("層ごとの強さ");
		std::array<BossTuning, kBossTuningLayerCount>& table = GetBossTuningTable();
		for (int32_t i = 0; i < kBossTuningLayerCount; ++i) {
			ParamEditor::BeginGroup(std::to_string(i + 1) + "層目");
			RegisterTuningParams(table[static_cast<size_t>(i)]);
			ParamEditor::EndGroup();
		}
		ParamEditor::EndGroup();

		ParamEditor::EndGroup();
	}

	//1層分の強さをParamEditorに登録する
	//長さと速さは床の大きさの倍率をかける前の値
	void Boss::RegisterTuningParams(BossTuning& tuning) {
		ParamEditor::RegisterValue("移動の速さ", &tuning.moveSpeed, 0.01f, 0.0f, 20.0f);
		ParamEditor::RegisterValue("ダメージ", &tuning.damage, 1.0f, 0, 10);

		ParamEditor::BeginGroup("近接攻撃");
		ParamEditor::RegisterValue("届く距離", &tuning.meleeRange, 0.01f, 0.0f, 10.0f);
		ParamEditor::RegisterValue("予告の秒数", &tuning.meleeWindup, 0.01f, 0.0f, 5.0f);
		ParamEditor::RegisterValue("上下に届く距離", &tuning.meleeHeight, 0.01f, 0.0f, 10.0f);
		ParamEditor::EndGroup();

		ParamEditor::BeginGroup("ジャンプ");
		ParamEditor::RegisterFlag("穴を飛び越える", &tuning.canJump);
		ParamEditor::RegisterValue("跳ぶ高さ", &tuning.jumpHeight, 0.01f, 0.0f, 10.0f);
		ParamEditor::RegisterValue("逃げるときに跳ぶ秒数", &tuning.jumpDuration, 0.01f, 0.0f, 5.0f);
		ParamEditor::RegisterValue("着地の余裕の秒数", &tuning.landingSafetyMargin, 0.01f, 0.0f, 5.0f);
		ParamEditor::EndGroup();

		ParamEditor::BeginGroup("パンチ");
		ParamEditor::RegisterFlag("使う", &tuning.punch);
		ParamEditor::RegisterValue("届く距離", &tuning.punchRange, 0.01f, 0.0f, 20.0f);
		ParamEditor::RegisterValue("横の幅", &tuning.punchWidth, 0.01f, 0.0f, 20.0f);
		ParamEditor::RegisterValue("予告の秒数", &tuning.punchWindup, 0.01f, 0.0f, 5.0f);
		ParamEditor::RegisterValue("拳が伸びる秒数", &tuning.punchStrikeDuration, 0.01f, 0.0f, 5.0f);
		ParamEditor::RegisterValue("拳が戻る秒数", &tuning.punchRecovery, 0.01f, 0.0f, 5.0f);
		ParamEditor::RegisterValue("はじく速さ", &tuning.punchPushSpeed, 0.1f, 0.0f, 50.0f);
		ParamEditor::RegisterValue("はじかれる秒数", &tuning.punchPushDuration, 0.01f, 0.0f, 5.0f);
		ParamEditor::RegisterValue("浮く速さ", &tuning.punchLiftSpeed, 0.1f, 0.0f, 20.0f);
		ParamEditor::EndGroup();

		ParamEditor::BeginGroup("突進");
		ParamEditor::RegisterValue("ためる秒数", &tuning.rushWindup, 0.01f, 0.0f, 5.0f);
		ParamEditor::RegisterValue("速さ", &tuning.rushSpeed, 0.1f, 0.0f, 50.0f);
		ParamEditor::RegisterValue("硬直の秒数", &tuning.rushRecovery, 0.01f, 0.0f, 5.0f);
		ParamEditor::RegisterValue("最大の距離", &tuning.rushMaxDistance, 0.5f, 0.0f, 200.0f);
		ParamEditor::RegisterValue("体の半径", &tuning.rushBodyRadius, 0.01f, 0.0f, 5.0f);
		ParamEditor::RegisterValue("当たる距離", &tuning.rushHitRadius, 0.01f, 0.0f, 5.0f);
		ParamEditor::EndGroup();

		ParamEditor::BeginGroup("地面パンチ");
		ParamEditor::RegisterValue("予備動作の秒数", &tuning.groundPunchAnnouncement, 0.01f, 0.0f, 5.0f);
		ParamEditor::RegisterValue("硬直の秒数", &tuning.groundPunchRecovery, 0.01f, 0.0f, 5.0f);
		ParamEditor::EndGroup();
	}

	//攻撃を全部やめて床の上に置く
	void Boss::Place(const Floor& floor, const Vector3& position)
	{
		Stop();
		cell_ = floor.GetCellAt(position);
		next_ = cell_;
		position_ = position;
		yawDeg_ = 0.0f;
		nextPatternTurn_ = 0;
	}

	//今の形態のHPを減らす。次の形態に進んだらやりかけの攻撃はやめる
	void Boss::Damage(int32_t amount) {
		if (phaseController_.Damage(amount) == BossPhaseController::Event::PhaseChanged) {
			CancelAttacks();
		}
	}

	//今の形態の移動の速さと飛び越えをtuningに入れる
	void Boss::ApplyPhaseTuning(BossTuning& tuning) const {
		const BossPhaseData& phase = phaseController_.GetPhase();
		//層の強さと同じく床の大きさの倍率をかける
		tuning.moveSpeed = phase.moveSpeed * GameConfig::kWorldScale;
		tuning.canJump = phase.canJump;
	}

	//試し用: 指定した形態にHP満タンで切り替える
	void Boss::DebugForcePhase(int32_t index) {
		phaseController_.ForcePhase(index);
		CancelAttacks();
	}

	//やりかけの攻撃を全部やめる。位置やジャンプはそのまま
	void Boss::CancelAttacks() {
		isWindingUp_ = false;
		isMeleeTellVisible_ = false;
		CancelPunch();
		rush_.Cancel();
		groundPunch_.Cancel();
	}

	//HPトリガーで決まった攻撃を始める
	void Boss::StartTriggeredAttack(const Floor& floor, const Vector3& target, const BossTuning& tuning, BossAttackType type) {
		switch (type) {
		case BossAttackType::Rush:
			rush_.Start(floor, position_, target, tuning);
			LookAt(rush_.GetForward());
			break;
		case BossAttackType::GroundPunch:
			groundPunch_.Start(floor, position_, target, tuning);
			break;
		case BossAttackType::Melee:
			StartAttack(floor, target, tuning, BossActionKind::Melee);
			break;
		default:
			break;
		}
	}

	//攻撃とジャンプをやめる
	void Boss::Stop()
	{
		isEvacuating_ = false;
		isWindingUp_ = false;
		isJumping_ = false;
		CancelPunch();
		rush_.Cancel();
		groundPunch_.Cancel();
		isMeleeTellVisible_ = false;
	}

	//足元が崩れたらとなりの床へ跳んで逃げる
	bool Boss::TryEvadeCollapse(const Floor& floor, const BossTuning& tuning)
	{
		if (isEvacuating_ || !tuning.canJump || isJumping_)
		{
			return false;
		}
		Cell landing{};
		if (!FloorPathFinder::FindLandingNeighbor(floor, position_, tuning.jumpDuration + tuning.landingSafetyMargin, landing))
		{
			return false;
		}
		StartJump(floor, landing, tuning.jumpDuration);
		++nextPatternTurn_;
		return true;
	}

	//攻撃中に足元が崩れそうなら攻撃をやめて逃げることにする
	bool Boss::TryInterruptUnsafeFooting(const Floor& floor, const BossTuning& tuning)
	{
		if (isJumping_ || !IsBusy())
		{
			return false;
		}
		Vector3 destination{};
		if (!BossPlanner::TryFindSafetyStep(floor, position_, tuning.moveSpeed, 0.0f, tuning.landingSafetyMargin, destination))
		{
			return false;
		}
		Stop();
		isEvacuating_ = true;
		evacuationDestination_ = floor.GetCellAt(destination);
		return true;
	}

	//跳んでいるか攻撃中か
	bool Boss::IsBusy() const
	{
		return isJumping_ || isWindingUp_ || punchPhase_ != PunchPhase::None || rush_.IsActive() || groundPunch_.IsActive();
	}

	int32_t Boss::Tick(const Floor& floor, const Vector3& target, const BossTuning& tuning, float moveSpeed, float deltaTime)
	{
		//このフレームの結果を消す
		landedThisTick_ = false;
		punchHitThisTick_ = false;

		//攻撃中に足元が崩れそうならやめて逃げることにする
		TryInterruptUnsafeFooting(floor, tuning);

#ifdef NRELEASE
		//試し用: Tキーで突進を始める
		if (Input::TriggerKey(DIK_T) && !IsBusy()) {
			rush_.Start(floor, position_, target, tuning);
			LookAt(rush_.GetForward());
		}
		//試し用: Gキーを押したら、次に行動を選ぶときに地面パンチする
		if (Input::TriggerKey(DIK_G)) {
			isGroundPunchRequested_ = true;
		}
#endif

		//突進中はそれだけ進める
		if (rush_.IsActive()) {
			const int32_t damage = rush_.Update(floor, position_, target, tuning, deltaTime);
			position_ = rush_.GetPosition();
			//終わったら今いるマスを合わせる
			if (!rush_.IsActive()) {
				cell_ = floor.GetCellAt(position_);
				next_ = cell_;
			}
			return damage;
		}

		//地面パンチ中はそれだけ進める
		if (groundPunch_.IsActive()) {
			return groundPunch_.Update(floor, position_, target, tuning, deltaTime);
		}

		//やりかけのパンチとジャンプはそれだけ進める
		if (punchPhase_ != PunchPhase::None)
		{
			return AdvancePunch(floor, target, tuning, deltaTime);
		}
		if (isJumping_)
		{
			AdvanceJump(floor, tuning, deltaTime);
			return 0;
		}

		//近接攻撃の予告が終わったら当てる
		if (isWindingUp_)
		{
			windup_ -= deltaTime;
			if (windup_ > 0.0f)
			{
				return 0;
			}
			isWindingUp_ = false;
			isMeleeTellVisible_ = false;
			Vector3 toTarget = target - position_;
			toTarget.y = 0.0f;
			if (std::sqrt(LengthSquared(toTarget)) <= tuning.meleeRange && std::abs(target.y - floor.GetHeight()) <= tuning.meleeHeight)
			{
				return tuning.damage;
			}
			return 0;
		}

		//逃げ先に着いた
		if (isEvacuating_ && floor.GetCellAt(position_) == evacuationDestination_)
		{
			isEvacuating_ = false;
		}

		//HPトリガーが発動を待っていれば、行動を選ぶ代わりにその攻撃をする
		BossAttackType triggered{};
		if (phaseController_.PopTrigger(triggered)) {
			StartTriggeredAttack(floor, target, tuning, triggered);
			return 0;
		}

		//試し用の地面パンチの予約があれば、行動を選ぶ代わりに地面パンチする
		if (isGroundPunchRequested_) {
			isGroundPunchRequested_ = false;
			groundPunch_.Start(floor, position_, target, tuning);
			return 0;
		}

		//BossPlannerで何をするか決める
		const BossPlannedAction action = BossPlanner::Choose(floor, position_, target, tuning, moveSpeed, nextPatternTurn_, 0.0f,
			isEvacuating_, floor.GetCellCenter(evacuationDestination_));
		if (action.kind == BossActionKind::Fall)
		{
			return 0; //落ちるのはGameが見る
		}
		if (action.kind == BossActionKind::Move)
		{
			//1マス跳び、逃げるときは逃げ先を覚えておく
			if (!action.isBlocked)
			{
				if (action.isEvading)
				{
					isEvacuating_ = true;
					evacuationDestination_ = floor.GetCellAt(action.to);
				}
				StartJump(floor, floor.GetCellAt(action.to), action.duration, isEvacuating_);
				AdvanceJump(floor, tuning, deltaTime);
			}
		}
		else if (!StartAttack(floor, target, tuning, action.kind))
		{
			return 0;
		}

		//1回動いた
		++nextPatternTurn_;
		return 0;
	}

	//その向きを向く
	void Boss::LookAt(const Vector3& direction)
	{
		yawDeg_ = std::atan2(direction.x, direction.z) * 180.0f / std::numbers::pi_v<float>;
	}

	//前の向き
	Vector3 Boss::GetForward() const
	{
		return RotateY({ 0.0f, 0.0f, 1.0f }, yawDeg_);
	}

	//ボスから見た位置をワールドの位置にする
	Vector3 Boss::ToWorld(const Vector3& local) const
	{
		return position_ + RotateY(local, yawDeg_);
	}

	//ワールドの位置をボスから見た位置にする
	Vector3 Boss::ToLocal(const Vector3& world) const
	{
		return RotateY(world - position_, -yawDeg_);
	}

	void Boss::StartJump(const Floor& floor, const Cell& landing, float duration, bool isEvacuation)
	{
		//逃げるためのジャンプでなければ逃げるのはやめる
		if (!isEvacuation)
		{
			isEvacuating_ = false;
		}
		//攻撃をやめて行き先と時間と始まりの位置を決める
		CancelPunch();
		groundPunch_.Cancel();
		isWindingUp_ = false;
		isMeleeTellVisible_ = false;
		isJumping_ = true;
		next_ = landing;
		jumpElapsed_ = 0.0f;
		jumpDuration_ = duration;
		jumpOrigin_ = position_;
		//着地先の方を向く
		const Vector3 toLanding = floor.GetCellCenter(landing) - jumpOrigin_;
		if (LengthSquared(Vector3{ toLanding.x, 0.0f, toLanding.z }) > 0.0001f)
		{
			LookAt(toLanding);
		}
	}

	//跳んでいる途中を進める
	void Boss::AdvanceJump(const Floor& floor, const BossTuning& tuning, float deltaTime)
	{
		jumpElapsed_ += deltaTime;
		//0から1の進み具合
		float t = 1.0f;
		if (jumpDuration_ > 0.0f)
		{
			t = std::clamp(jumpElapsed_ / jumpDuration_, 0.0f, 1.0f);
		}
		//マスの線にそって山なりに進む
		Vector3 position = Floor::AlongGrid(jumpOrigin_, floor.GetCellCenter(next_), t);
		position.y += 4.0f * tuning.jumpHeight * t * (1.0f - t);
		position_ = position;
		//上下に動かず歩くときは穴の上に来たらそこで止まって落ちる
		if (tuning.jumpHeight <= 0.0f && !floor.IsSolid(floor.GetCellAt(position)))
		{
			next_ = floor.GetCellAt(position);
			cell_ = next_;
			isJumping_ = false;
			landedThisTick_ = true;
			return;
		}
		if (t < 1.0f)
		{
			return;
		}
		//着いた
		cell_ = next_;
		isJumping_ = false;
		landedThisTick_ = true;
	}

	void Boss::UpdateVisuals()
	{
		view_.Update(*this);
	}

	//体と拳と攻撃の予告を描く
	void Boss::Draw()
	{
		view_.Draw(*this);
	}
}
