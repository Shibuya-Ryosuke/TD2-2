#include "Stamp.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App {
	//その場で止まって、プレイヤーのマスの周り9マスに警告を出す
	void Stamp::Start(const Floor& floor, const Vector3& bossPos, const Vector3& target, const BossTuning& tuning) {
		(void)tuning; // 秒数や高さはこのクラスのメンバを使う

		// 跳ぶ前の位置と、着地するマス・位置
		startPos_ = bossPos;
		position_ = bossPos;
		targetCell_ = floor.GetCellAt(target);
		endPos_ = floor.GetCellCenter(targetCell_);

		// 着地するマスと周りの8マス。盤面の外は入れない
		warningCells_.clear();
		warningCenters_.clear();
		for (int32_t dz = -1; dz <= 1; ++dz) {
			for (int32_t dx = -1; dx <= 1; ++dx) {
				const Cell cell = targetCell_ + Cell{ dx, dz };
				if (!floor.Contains(cell)) {
					continue;
				}
				warningCells_.push_back(cell);
				warningCenters_.push_back(floor.GetCellCenter(cell));
			}
		}
		tileSize_ = { floor.GetTileSizeX(), floor.GetTileSizeZ() };

		// 前のスタンプの結果を消す
		jumpTimer_ = 0.0f;
		isLandedThisTick_ = false;

		// 予告から始める。コンボの途中なら硬直は1フレームで終わらせる
		announcementTime_ = announcementDuration_;
		recoveryTime_ = skipRecovery_ ? 0.0f : recoveryDuration_;
		skipRecovery_ = false;
		phase_ = AttackPhase::Announcement;
	}

	int32_t Stamp::Update(const Floor& floor, const Vector3& bossPos, const Vector3& target, const BossTuning& tuning, float deltaTime) {
		(void)bossPos; // 跳んでいる途中の位置はこのクラスが持つ

		// このフレームの結果を消す
		isLandedThisTick_ = false;
		int32_t damage = 0;

		switch (phase_) {
		case AttackPhase::Announcement:
			// 予告の間はその場で止まっている
			TimeDecrease(announcementTime_, AttackPhase::Attack);
			break;

		case AttackPhase::Attack:
			damage = UpdateJump(floor, target, tuning, deltaTime);
			break;

		case AttackPhase::Recovery:
			// 硬直が終わったら警告も消す
			TimeDecrease(recoveryTime_, AttackPhase::None);
			if (phase_ == AttackPhase::None) {
				warningCells_.clear();
				warningCenters_.clear();
			}
			break;

		default:
			break;
		}
		return damage;
	}

	//山なりに跳んで、着地したフレームだけプレイヤーとの当たり判定をする
	int32_t Stamp::UpdateJump(const Floor& floor, const Vector3& target, const BossTuning& tuning, float deltaTime) {
		// 進み具合は 跳んでからの時間 ÷ 跳ぶ時間
		jumpTimer_ += deltaTime;
		float t = 1.0f;
		if (jumpDuration_ > 0.0f) {
			t = std::clamp(jumpTimer_ / jumpDuration_, 0.0f, 1.0f);
		}

		// 横はまっすぐ、高さは山なりに足す
		position_ = Lerp(startPos_, endPos_, t);
		position_.y += 4.0f * jumpHeight_ * t * (1.0f - t);
		if (t < 1.0f) {
			return 0;
		}

		// 着地した。穴なら落ちる・切り離しの判定はBossBattleが見る
		position_ = endPos_;
		isLandedThisTick_ = true;
		phase_ = AttackPhase::Recovery;

		// プレイヤーが9マスの中にいて、床の高さにいたら当たり
		const Cell playerCell = floor.GetCellAt(target);
		const bool isInside = std::find(warningCells_.begin(), warningCells_.end(), playerCell) != warningCells_.end();
		const bool isOnFloor = std::abs(target.y - floor.GetHeight()) <= tuning.meleeHeight;
		if (isInside && isOnFloor) {
			return tuning.damage;
		}
		return 0;
	}

	//調整する値をParamEditorに登録する
	void Stamp::RegisterParams() {
		ParamEditor::BeginGroup("スタンプ");
		ParamEditor::RegisterValue("予告の秒数", &announcementDuration_, 0.01f, 0.0f, 5.0f);
		ParamEditor::RegisterValue("跳ぶ高さ", &jumpHeight_, 0.01f, 0.0f, 20.0f);
		ParamEditor::RegisterValue("跳ぶ秒数", &jumpDuration_, 0.01f, 0.05f, 5.0f);
		ParamEditor::RegisterValue("硬直の秒数", &recoveryDuration_, 0.01f, 0.0f, 5.0f);
		ParamEditor::EndGroup();
	}

	//スタンプをやめて警告を消す
	void Stamp::Cancel() {
		AttackBase::Cancel();
		warningCells_.clear();
		warningCenters_.clear();
	}
}
