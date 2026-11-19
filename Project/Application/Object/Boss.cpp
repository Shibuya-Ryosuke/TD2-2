#include "Boss.h"
#include "../Game/GameConfig.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App {
	namespace {
		// --- 見た目の大きさ・位置 (Unity のプロトタイプのボスと同じ。ボスのローカル) ---
		constexpr Vector3 kBodySize = { 1.55f, 1.5f, 1.45f };
		constexpr Vector3 kBodyCenter = { 0.0f, 0.75f, 0.0f };
		constexpr Vector3 kFistSize = { 0.75f, 0.75f, 0.9f };
		constexpr Vector3 kFistRest = { 1.15f, 0.85f, 0.2f };      // 拳のいつもの位置
		constexpr float kFistPullBack = 0.65f;                     // 予告で拳を後ろに引く距離
		constexpr float kMeleeTellHeight = 0.025f;                 // 近接攻撃の予告の厚さ
		constexpr float kMeleeTellY = 0.035f;
		constexpr float kPunchTellHeight = 0.035f;                 // パンチの予告の厚さ
		constexpr float kPunchTellY = 0.05f;

		// 長さの2乗 (比べるだけなので sqrt しない)
		float LengthSquared(const Vector3& v) {
			return v.x * v.x + v.y * v.y + v.z * v.z;
		}


		// Y 軸で回す (度。+Z が +X の方へ回るのが +)
		Vector3 RotateY(const Vector3& v, float degree) {
			const float radian = degree * std::numbers::pi_v<float> / 180.0f;
			const float c = std::cos(radian);
			const float s = std::sin(radian);
			return Vector3{ v.x * c + v.z * s, v.y, -v.x * s + v.z * c };
		}

		// cube.obj (1辺 2) を、1辺が size の箱にする大きさ
		Vector3 CubeScale(const Vector3& size) {
			return size * (1.0f / GameConfig::kCubeModelSize);
		}
	}

	// =====================================================================
	// 初期化
	// =====================================================================
	void Boss::Initialize() {
		// 体 (赤い箱)
		body_ = Model::Create(GameConfig::kCubeModel);
		body_->SetColor(GameConfig::kBossColor);

		// 拳
		fist_ = Model::Create(GameConfig::kCubeModel);
		fist_->SetColor(GameConfig::kBossArmorColor);

		// 近接攻撃の予告 (足元の板)
		meleeTell_ = Model::Create(GameConfig::kCubeModel);
		meleeTell_->SetColor(GameConfig::kAttackColor);
		meleeTell_->SetCastShadow(false);

		// パンチの予告 (前にのびる板)
		punchTell_ = Model::Create(GameConfig::kCubeModel);
		punchTell_->SetColor(GameConfig::kWarningColor);
		punchTell_->SetCastShadow(false);

		fistLocal_ = kFistRest;
	}

	// 床の上に置いて、攻撃・待ち時間を全部戻す
	void Boss::Place(const Floor& floor, const Vector3& position) {
		Stop();
		cell_ = floor.GetCellAt(position);
		next_ = cell_;
		position_ = position;
		yawDeg_ = 0.0f;
		meleeCooldown_ = 0.0f;
		punchCooldown_ = 0.0f;
	}

	// 攻撃・ジャンプをやめる
	void Boss::Stop() {
		isWindingUp_ = false;
		isJumping_ = false;
		CancelPunch();
		isMeleeTellVisible_ = false;
	}

	// 足元が崩れたら、となりの床へ跳んで逃げる
	bool Boss::TryEvadeCollapse(const Floor& floor, const BossTuning& tuning) {
		if (!tuning.canJump || isJumping_) {
			return false;
		}
		Cell landing{};
		if (!floor.TryFindLandingNeighbor(position_, tuning.jumpDuration, landing)) {
			return false;
		}
		StartJump(floor, landing, tuning.jumpDuration);
		return true;
	}

	// 体当たりされて、となりのマスへ跳ばされる
	void Boss::KnockBack(const Floor& floor, const Cell& direction, float duration) {
		// となりのマスへ (床の外なら、今のマスでその場で跳ねる)
		const Cell current = floor.GetCellAt(position_);
		Cell target = current + direction;
		if (!floor.Contains(target)) {
			target = current;
		}
		StartJump(floor, target, duration);
	}

	// =====================================================================
	// 1フレーム動かす (Unity の BossAgent.Tick と同じ流れ)
	// =====================================================================
	int32_t Boss::Tick(const Floor& floor, const Vector3& target, const BossTuning& tuning, float moveSpeed, float deltaTime) {
		// このフレームの結果を消して、攻撃の待ち時間を減らす
		landedThisTick_ = false;
		punchHitThisTick_ = false;
		meleeCooldown_ -= deltaTime;
		punchCooldown_ -= deltaTime;

		// パンチ中・跳んでいる途中は、それだけ進める
		if (punchPhase_ != PunchPhase::None) {
			return AdvancePunch(floor, target, tuning, deltaTime);
		}
		if (isJumping_) {
			AdvanceJump(floor, tuning, deltaTime);
			return 0;
		}

		// 今のマス・次のマスが崩れていたら、立て直す
		if (!floor.IsSolid(cell_)) {
			cell_ = floor.GetCellAt(position_);
		}
		if (!floor.IsSolid(next_)) {
			next_ = cell_;
		}

		// マスの真ん中にいるか
		const Vector3 cellCenter = floor.GetCellCenter(cell_);
		const bool isAtCenter = LengthSquared(position_ - cellCenter) < 1e-10f;

		// プレイヤーの方を向く
		Vector3 toTarget = target - position_;
		toTarget.y = 0.0f;
		const float distance = std::sqrt(LengthSquared(toTarget));
		if (LengthSquared(toTarget) > 0.0001f) {
			LookAt(toTarget);
		}

		// 近接攻撃の予告中: 時間が来たら当てる
		if (isWindingUp_) {
			windup_ -= deltaTime;
			if (windup_ > 0.0f) {
				return 0;
			}
			isWindingUp_ = false;
			isMeleeTellVisible_ = false;
			meleeCooldown_ = tuning.meleeInterval;
			if (distance <= tuning.meleeRange && std::abs(target.y - floor.GetHeight()) <= tuning.meleeHeight) {
				return tuning.damage;
			}
			return 0;
		}

		// 止まっている (マスの真ん中で、次のマスもない) ときだけ攻撃を始める
		const bool isStill = isAtCenter && cell_ == next_;

		// パンチを始める
		if (isStill && tuning.punch && punchCooldown_ <= 0.0f && distance <= tuning.punchRange && distance > 0.001f) {
			punchDirection_ = Normalize(toTarget);
			LookAt(punchDirection_);
			punchTimer_ = 0.0f;
			punchPhase_ = PunchPhase::Windup;
			punchTellRange_ = tuning.punchRange;
			punchTellWidth_ = tuning.punchWidth;
			isPunchTellVisible_ = true;
			return 0;
		}

		// 近接攻撃を始める (予告を出す)
		if (isStill && distance <= tuning.meleeRange && meleeCooldown_ <= 0.0f) {
			isWindingUp_ = true;
			windup_ = tuning.meleeWindup;
			meleeTellSize_ = tuning.meleeRange * 2.0f;
			isMeleeTellVisible_ = true;
			return 0;
		}

		// 進む: マスの真ん中にいれば次のマスへ、ずれていれば今のマスの真ん中へ、1マス跳ぶ
		if (moveSpeed <= 0.0f) {
			return 0;
		}
		if (isAtCenter && distance <= tuning.meleeRange) {
			return 0; // もう近いので進まない
		}
		Cell destination = cell_;
		if (isAtCenter) {
			destination = floor.NextStep(cell_, target, tuning.canJump);
		}
		const Vector3 toDestination = floor.GetCellCenter(destination) - position_;
		const float jumpDistance = std::sqrt(LengthSquared(toDestination));
		if (LengthSquared(toDestination) <= 0.0001f) {
			return 0;
		}
		// 跳ぶ時間 = 距離 / 速さ
		StartJump(floor, destination, jumpDistance / moveSpeed);
		AdvanceJump(floor, tuning, deltaTime);
		return 0;
	}

	// その向きを向く
	void Boss::LookAt(const Vector3& direction) {
		yawDeg_ = std::atan2(direction.x, direction.z) * 180.0f / std::numbers::pi_v<float>;
	}

	// 前の向き
	Vector3 Boss::GetForward() const {
		return RotateY({ 0.0f, 0.0f, 1.0f }, yawDeg_);
	}

	// ボスから見た位置 → ワールドの位置
	Vector3 Boss::ToWorld(const Vector3& local) const {
		return position_ + RotateY(local, yawDeg_);
	}

	// ワールドの位置 → ボスから見た位置
	Vector3 Boss::ToLocal(const Vector3& world) const {
		return RotateY(world - position_, -yawDeg_);
	}

	// =====================================================================
	// パンチ
	// =====================================================================
	int32_t Boss::AdvancePunch(const Floor& floor, const Vector3& target, const BossTuning& tuning, float deltaTime) {
		// 拳の位置: 引いた所と、伸ばしきった所
		punchTimer_ += deltaTime;
		const Vector3 pulledBack = kFistRest + Vector3{ 0.0f, 0.0f, -kFistPullBack };
		const Vector3 extended = { 0.0f, kFistRest.y, tuning.punchRange };

		// 予告: 拳を後ろに引く
		if (punchPhase_ == PunchPhase::Windup) {
			const float t = std::clamp(punchTimer_ / tuning.punchWindup, 0.0f, 1.0f);
			fistLocal_ = kFistRest + Vector3{ 0.0f, 0.0f, -kFistPullBack * t };
			if (t < 1.0f) {
				return 0;
			}
			punchPhase_ = PunchPhase::Strike;
			punchTimer_ = 0.0f;
			isPunchResolved_ = false;
			isPunchTellVisible_ = false;
			return 0;
		}

		// 拳が伸びる。半分まで伸びたら当たり判定
		if (punchPhase_ == PunchPhase::Strike) {
			const float s = std::clamp(punchTimer_ / tuning.punchStrikeDuration, 0.0f, 1.0f);
			fistLocal_ = Lerp(pulledBack, extended, s);
			if (s >= 1.0f) {
				punchPhase_ = PunchPhase::Recovery;
				punchTimer_ = 0.0f;
			}
			if (isPunchResolved_ || s < 0.5f) {
				return 0;
			}
			isPunchResolved_ = true;

			// 前方の長方形 (長さ punchRange・幅 punchWidth) の中にいるか
			const Vector3 relative = target - position_;
			const float forwardDistance = Dot(relative, punchDirection_);
			const float sideDistance = Dot(relative, Cross(Vector3{ 0.0f, 1.0f, 0.0f }, punchDirection_));
			if (forwardDistance < 0.0f || forwardDistance > tuning.punchRange) {
				return 0;
			}
			if (std::abs(sideDistance) > tuning.punchWidth * 0.5f) {
				return 0;
			}
			if (std::abs(target.y - floor.GetHeight()) > tuning.meleeHeight) {
				return 0;
			}

			// はじく向き (プロトタイプと同じく、床の真ん中からプレイヤーへの向き)
			Vector3 push = target - Vector3{ 0.0f, floor.GetHeight(), 0.0f };
			push.y = 0.0f;
			if (LengthSquared(push) > 0.001f) {
				punchPushDirection_ = Normalize(push);
			} else {
				punchPushDirection_ = punchDirection_;
			}
			punchHitThisTick_ = true;
			return tuning.damage;
		}

		// 拳が戻る
		if (punchPhase_ == PunchPhase::Recovery) {
			const float r = std::clamp(punchTimer_ / tuning.punchRecovery, 0.0f, 1.0f);
			fistLocal_ = Lerp(extended, kFistRest, r);
			if (r < 1.0f) {
				return 0;
			}
			CancelPunch();
			punchCooldown_ = tuning.punchInterval;
		}
		return 0;
	}

	// パンチをやめて、拳を戻す
	void Boss::CancelPunch() {
		punchPhase_ = PunchPhase::None;
		punchTimer_ = 0.0f;
		isPunchTellVisible_ = false;
		fistLocal_ = kFistRest;
	}

	// =====================================================================
	// ジャンプ
	// =====================================================================
	void Boss::StartJump(const Floor& floor, const Cell& landing, float duration) {
		// 攻撃をやめて、跳ぶ準備をする (行き先・時間・始まりの位置)
		CancelPunch();
		isWindingUp_ = false;
		isMeleeTellVisible_ = false;
		isJumping_ = true;
		next_ = landing;
		jumpElapsed_ = 0.0f;
		jumpDuration_ = duration;
		jumpOrigin_ = position_;
		// 着地先の方を向く
		const Vector3 toLanding = floor.GetCellCenter(landing) - jumpOrigin_;
		if (LengthSquared(Vector3{ toLanding.x, 0.0f, toLanding.z }) > 0.0001f) {
			LookAt(toLanding);
		}
	}

	// 跳んでいる途中を進める。着いたら LandedThisTick
	void Boss::AdvanceJump(const Floor& floor, const BossTuning& tuning, float deltaTime) {
		jumpElapsed_ += deltaTime;
		// 進み具合 (0 〜 1)
		float t = 1.0f;
		if (jumpDuration_ > 0.0f) {
			t = std::clamp(jumpElapsed_ / jumpDuration_, 0.0f, 1.0f);
		}
		// マスの線にそって進みながら、山なりに上下する
		Vector3 position = Floor::AlongGrid(jumpOrigin_, floor.GetCellCenter(next_), t);
		position.y += 4.0f * tuning.jumpHeight * t * (1.0f - t);
		position_ = position;
		if (t < 1.0f) {
			return;
		}
		// 着いた
		cell_ = next_;
		isJumping_ = false;
		landedThisTick_ = true;
	}

	// =====================================================================
	// 見た目
	// =====================================================================
	void Boss::UpdateVisuals() {
		// ボスの向き (Y 軸だけ回す)
		const Vector3 rotate = { 0.0f, yawDeg_, 0.0f };

		// 体
		body_->SetScale(CubeScale(kBodySize));
		body_->SetRotate(rotate);
		body_->SetTranslate(ToWorld(kBodyCenter));
		body_->TransferMatrix();

		// 拳
		fist_->SetScale(CubeScale(kFistSize));
		fist_->SetRotate(rotate);
		fist_->SetTranslate(ToWorld(fistLocal_));
		fist_->TransferMatrix();

		// 近接攻撃の予告
		meleeTell_->SetScale(CubeScale({ meleeTellSize_, kMeleeTellHeight, meleeTellSize_ }));
		meleeTell_->SetRotate(rotate);
		meleeTell_->SetTranslate(ToWorld({ 0.0f, kMeleeTellY, 0.0f }));
		meleeTell_->TransferMatrix();

		// パンチの予告
		punchTell_->SetScale(CubeScale({ punchTellWidth_, kPunchTellHeight, punchTellRange_ }));
		punchTell_->SetRotate(rotate);
		punchTell_->SetTranslate(ToWorld({ 0.0f, kPunchTellY, punchTellRange_ * 0.5f }));
		punchTell_->TransferMatrix();
	}

	// 体・拳・攻撃の予告を描く
	void Boss::Draw() {
		if (!isActive_) {
			return;
		}
		body_->Draw();
		fist_->Draw();
		if (isMeleeTellVisible_) {
			meleeTell_->Draw();
		}
		if (isPunchTellVisible_) {
			punchTell_->Draw();
		}
	}
}
