#include "BossComboRunner.h"
#include <algorithm>

namespace App {
	//コンボを始める
	void BossComboRunner::Start(const BossCombo& combo) {
		steps_ = combo.steps;
		stepIndex_ = 0;
		waitTimer_ = 0.0f;
		isActive_ = !steps_.empty();
		if (isActive_) {
			remaining_ = std::max(1, steps_[0].repeat);
		}
	}

	//コンボをやめる
	void BossComboRunner::Cancel() {
		isActive_ = false;
		steps_.clear();
	}

	//次の攻撃を出すか決める
	bool BossComboRunner::Next(float deltaTime, BossAttackType& attack, bool& skipRecovery) {
		if (!isActive_) {
			return false;
		}
		//間隔を待っている
		if (waitTimer_ > 0.0f) {
			waitTimer_ -= deltaTime;
			return false;
		}
		//全部の段を出し終わった
		if (stepIndex_ >= steps_.size()) {
			isActive_ = false;
			return false;
		}

		//今の段の攻撃を1回出す
		const BossComboStep& step = steps_[stepIndex_];
		attack = step.attack;
		waitTimer_ = std::max(0.0f, step.interval);
		--remaining_;

		//今の段を出し切ったら次の段へ
		if (remaining_ <= 0) {
			++stepIndex_;
			if (stepIndex_ < steps_.size()) {
				remaining_ = std::max(1, steps_[stepIndex_].repeat);
			}
		}

		//コンボ最後の攻撃だけ硬直する。最後のあとは待たない
		const bool isLast = stepIndex_ >= steps_.size();
		skipRecovery = !isLast;
		if (isLast) {
			waitTimer_ = 0.0f;
		}
		return true;
	}
}
