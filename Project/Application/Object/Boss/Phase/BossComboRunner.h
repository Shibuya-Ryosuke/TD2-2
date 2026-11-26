#pragma once
#include <cstdint>
#include <vector>
#include "BossPhaseData.h"

namespace App {
	//コンボを上の段から順に進める
	//ボスは攻撃が終わるたびにNextを呼び、出す攻撃を受け取る
	class BossComboRunner {
	public:
		//コンボを始める。段は写して持つので、途中でエディタを触っても崩れない
		void Start(const BossCombo& combo);
		//コンボをやめる
		void Cancel();
		//コンボの途中か
		bool IsActive() const { return isActive_; }

		//今の攻撃が終わっているときに呼ぶ
		//次の攻撃を出すときはtrueを返し、attackに種類、skipRecoveryに硬直を飛ばすかを入れる
		//間隔を待っている間と、コンボが終わったときはfalse
		bool Next(float deltaTime, BossAttackType& attack, bool& skipRecovery);

	private:
		std::vector<BossComboStep> steps_; //出す段
		size_t stepIndex_ = 0; //今の段
		int32_t remaining_ = 0; //今の段であと何回出すか
		float waitTimer_ = 0.0f; //次を出すまでの残り秒数
		bool isActive_ = false;
	};
}
