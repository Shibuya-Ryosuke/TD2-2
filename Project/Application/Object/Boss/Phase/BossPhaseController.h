#pragma once
#include <cstdint>
#include <deque>
#include <vector>
#include "BossPhaseData.h"

namespace App {
	//ボスのHPと今の形態を持ち、HPが尽きたら次の形態へ進める
	//形態の中身はBossPhaseTableの表を毎回読むので、エディタで変えた値がすぐ反映される
	class BossPhaseController {
	public:
		//HPを減らした結果
		enum class Event {
			None,         //何も起きていない
			PhaseChanged, //次の形態へ進んだ
			Defeated,     //最後の形態のHPが尽きた
		};

		//第1形態のHP満タンに戻す
		void Reset();
		//今の形態のHPを減らす。尽きたら次の形態へ進む
		Event Damage(int32_t amount);
		//表が編集されて形態が減ったりHPが変わったりしたら、今の番号とHPを収める
		void Sync();
		//指定した形態にHP満タンで切り替える
		void ForcePhase(int32_t index);

		//発動を待っているHPトリガーの攻撃を1つ取り出す。なければfalse
		bool PopTrigger(BossAttackType& action);

		//今の形態の設定
		const BossPhaseData& GetPhase() const;
		int32_t GetIndex() const { return index_; }
		int32_t GetHp() const { return hp_; }
		int32_t GetMaxHp() const { return GetPhase().maxHp; }
		bool IsDefeated() const { return isDefeated_; }

	private:
		//index番目の形態にHP満タンで入る
		void EnterPhase(int32_t index);
		//HPが減ったときにHPトリガーを調べる
		void CheckTriggers();

		int32_t index_ = 0; //今の形態の番号
		int32_t hp_ = 0; //今の形態の残りHP
		bool isDefeated_ = false; //最後の形態まで倒したか
		std::vector<bool> isTriggerFired_; //今の形態のトリガーをもう発動したか
		std::deque<BossAttackType> pendingTriggers_; //発動を待っている攻撃
	};
}
