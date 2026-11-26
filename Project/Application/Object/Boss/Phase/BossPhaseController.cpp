#include "BossPhaseController.h"
#include "BossPhaseTable.h"
#include <algorithm>

namespace App {
	//第1形態のHP満タンに戻す
	void BossPhaseController::Reset() {
		isDefeated_ = false;
		EnterPhase(0);
	}

	//今の形態のHPを減らす。尽きたら次の形態へ進む
	BossPhaseController::Event BossPhaseController::Damage(int32_t amount) {
		if (isDefeated_ || amount <= 0) {
			return Event::None;
		}
		hp_ = std::max(0, hp_ - amount);
		if (hp_ > 0) {
			CheckTriggers();
			return Event::None;
		}

		//次の形態があればそこへ。余ったダメージは持ち越さない
		const int32_t count = static_cast<int32_t>(BossPhaseTable::GetPhases().size());
		if (index_ + 1 < count) {
			EnterPhase(index_ + 1);
			return Event::PhaseChanged;
		}
		isDefeated_ = true;
		pendingTriggers_.clear();
		return Event::Defeated;
	}

	//表が編集されても今の番号とHPが範囲の中に収まるようにする
	void BossPhaseController::Sync() {
		const int32_t count = static_cast<int32_t>(BossPhaseTable::GetPhases().size());
		//形態が消されて番号がはみ出したら最後の形態にする
		if (index_ >= count) {
			index_ = std::max(0, count - 1);
		}
		hp_ = std::clamp(hp_, 0, GetMaxHp());
		//トリガーが増えたり減ったりしたら印の数を合わせる
		isTriggerFired_.resize(GetPhase().triggers.size(), false);
	}

	//指定した形態にHP満タンで切り替える
	void BossPhaseController::ForcePhase(int32_t index) {
		const int32_t count = static_cast<int32_t>(BossPhaseTable::GetPhases().size());
		isDefeated_ = false;
		EnterPhase(std::clamp(index, 0, std::max(0, count - 1)));
	}

	//発動を待っているHPトリガーの攻撃を1つ取り出す
	bool BossPhaseController::PopTrigger(BossAttackType& action) {
		if (pendingTriggers_.empty()) {
			return false;
		}
		action = pendingTriggers_.front();
		pendingTriggers_.pop_front();
		return true;
	}

	//今の形態の設定
	const BossPhaseData& BossPhaseController::GetPhase() const {
		return BossPhaseTable::GetPhases()[static_cast<size_t>(index_)];
	}

	//index番目の形態にHP満タンで入る
	void BossPhaseController::EnterPhase(int32_t index) {
		index_ = index;
		hp_ = GetPhase().maxHp;
		//トリガーは形態ごとに最初から
		isTriggerFired_.assign(GetPhase().triggers.size(), false);
		pendingTriggers_.clear();
	}

	//HPの割合がトリガーの割合以下になったら1回だけ発動を待たせる
	void BossPhaseController::CheckTriggers() {
		const BossPhaseData& phase = GetPhase();
		isTriggerFired_.resize(phase.triggers.size(), false);
		const float rate = static_cast<float>(hp_) / static_cast<float>(std::max(1, phase.maxHp));
		for (size_t i = 0; i < phase.triggers.size(); ++i) {
			if (isTriggerFired_[i] || rate > phase.triggers[i].hpRate) {
				continue;
			}
			isTriggerFired_[i] = true;
			pendingTriggers_.push_back(phase.triggers[i].action);
		}
	}
}
