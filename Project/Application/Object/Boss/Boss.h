#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <memory>
#include "BossTuning.h"
#include "BossPlan.h"
#include <vector>
#include "../../Stage/Floor/Floor.h"
#include "BossView.h"
#include "Attack/Rush.h"
#include "Attack/GroundPunch.h"
#include "Phase/BossPhaseController.h"
#include "Phase/BossComboRunner.h"

namespace App
{
	//プレイヤーと同時に動き、BossPlannerで近づくか攻撃するか逃げるかを決めるボス
	class Boss
	{
	public:
		Boss() = default;
		~Boss() = default;

		Boss(const Boss&) = delete;
		Boss& operator=(const Boss&) = delete;

		void Initialize(); //体と拳と攻撃の予告のモデルを作る
		//ボスの調整する値をまとめてParamEditorに登録する
		void RegisterParams();

		//攻撃を全部やめて床の上に置く
		void Place(const Floor& floor, const RyoEngine::Vector3& position);
		//攻撃とジャンプを全部やめる
		void Stop();

		//足元が崩れたら跳んで逃げる
		bool TryEvadeCollapse(const Floor& floor, const BossTuning& tuning);
		//攻撃中に足元が崩れそうなら攻撃をやめて逃げることにする
		bool TryInterruptUnsafeFooting(const Floor& floor, const BossTuning& tuning);

		//1フレーム動かして当たった攻撃のダメージを返す
		int32_t Tick(const Floor& floor, const RyoEngine::Vector3& target, const BossTuning& tuning, float moveSpeed, float deltaTime);

		//動かし終わったあとに行列を確定する
		void UpdateVisuals();
		void Draw(); //体と拳と攻撃の予告を描く

		bool IsJumping() const { return isJumping_; }
		//跳んでいるか攻撃中で、この間は新しい行動を始めない
		bool IsBusy() const;
		//このフレームでジャンプから着地したか
		bool LandedThisTick() const { return landedThisTick_; }
		//このフレームでパンチが当たったか
		bool PunchHitThisTick() const { return punchHitThisTick_; }
		//パンチではじく向き
		const RyoEngine::Vector3& GetPunchPushDirection() const { return punchPushDirection_; }

		const RyoEngine::Vector3& GetPosition() const { return position_; }
		void SetPosition(const RyoEngine::Vector3& position) { position_ = position; }
		float GetYawDeg() const { return yawDeg_; }
		//前の向き
		RyoEngine::Vector3 GetForward() const;

		//ボスのローカル座標をワールド座標にする
		RyoEngine::Vector3 ToWorld(const RyoEngine::Vector3& local) const;
		//ワールド座標をボスのローカル座標にする
		RyoEngine::Vector3 ToLocal(const RyoEngine::Vector3& world) const;

		//倒したら描かない
		void SetActive(bool isActive) { isActive_ = isActive; }
		bool IsActive() const { return isActive_; }
		//寝ている割合で、1なら横になって寝ている見た目
		void SetSleep(float sleep) { sleep_ = sleep; }
		float GetSleep() const { return sleep_; }
		//足場が当たってむぎゅっとつぶれる
		void Flinch(float power);
		float GetFlinch() const { return flinch_; }
		//やられ演出などで体と拳をalphaの濃さに透かして、0なら描かない
		void SetFade(float alpha) { view_.SetFade(alpha); }

		//BossViewが読む見た目の値
		const RyoEngine::Vector3& GetFistLocal() const { return fistLocal_; } //ボスのローカルの拳の位置
		bool IsPunching() const { return punchPhase_ != PunchPhase::None; } //拳を引いてから戻るまでのパンチ中か
		bool IsMeleeTellVisible() const { return isMeleeTellVisible_; } //近接攻撃の予告を出すか
		float GetMeleeTellSize() const { return meleeTellSize_; } //近接攻撃の予告の大きさ
		bool IsPunchTellVisible() const { return isPunchTellVisible_; } //パンチの予告を出すか
		float GetPunchTellRange() const { return punchTellRange_; } //パンチの予告の長さ
		float GetPunchTellWidth() const { return punchTellWidth_; } //パンチの予告の幅


		// 警告表示で使う
		const Rush& GetRush() const { return rush_; }
		const GroundPunch& GetGroundPunch() const { return groundPunch_; }

		//HPと形態
		//第1形態のHP満タンに戻す
		void ResetHealth() { phaseController_.Reset(); }
		//今の形態のHPを減らす。尽きたら次の形態へ進み、やりかけの攻撃はやめる
		void Damage(int32_t amount);
		//最後の形態まで倒したか
		bool IsDefeated() const { return phaseController_.IsDefeated(); }
		int32_t GetHp() const { return phaseController_.GetHp(); }
		int32_t GetMaxHp() const { return phaseController_.GetMaxHp(); }
		//今の形態の移動の速さと飛び越えをtuningに入れる
		void ApplyPhaseTuning(BossTuning& tuning) const;
		const BossPhaseController& GetPhaseController() const { return phaseController_; }
		BossPhaseController& GetPhaseController() { return phaseController_; }
		//試し用: 指定した形態にHP満タンで切り替える
		void DebugForcePhase(int32_t index);

		//コンボを始める。今の攻撃が終わったら上の段から順に出す
		void StartCombo(const BossCombo& combo) { comboRunner_.Start(combo); }
		//コンボの途中か
		bool IsComboActive() const { return comboRunner_.IsActive(); }

	private:
		enum class PunchPhase
		{
			None, //パンチしていない
			Windup, //拳を後ろに引く予告
			Strike, //拳が伸びる
			Recovery, //拳が戻る
		};

		//やりかけの攻撃を全部やめる。位置やジャンプはそのまま
		void CancelAttacks();
		//コンボの攻撃を始める。skipRecoveryなら硬直を飛ばす
		void StartComboAttack(const Floor& floor, const RyoEngine::Vector3& target, const BossTuning& tuning, BossAttackType type, bool skipRecovery);
		//近接かパンチの攻撃を始める
		bool StartAttack(const Floor& floor, const RyoEngine::Vector3& target, const BossTuning& tuning, BossActionKind kind);
		int32_t AdvancePunch(const Floor& floor, const RyoEngine::Vector3& target, const BossTuning& tuning, float deltaTime); //パンチを進めて当たったらダメージを返す
		void CancelPunch(); //パンチをやめて拳を戻す
		void StartJump(const Floor& floor, const Cell& landing, float duration, bool isEvacuation = false); //landingのマスへduration秒で跳び始める
		void AdvanceJump(const Floor& floor, const BossTuning& tuning, float deltaTime); //跳んでいる途中を進める
		void LookAt(const RyoEngine::Vector3& direction); //その向きを向く
		//1層分の強さをParamEditorに登録する
		static void RegisterTuningParams(BossTuning& tuning);

		//状態
		RyoEngine::Vector3 position_{}; //足元の位置
		float yawDeg_ = 0.0f; //奥が0の向きの角度
		bool isActive_ = true;
		float sleep_ = 0.0f; //寝ている見た目の割合
		float flinch_ = 0.0f; //足場が当たってつぶれている割合
		Cell cell_{}; //今いるマス
		Cell next_{}; //次に向かうマス
		int32_t nextPatternTurn_ = 0; //移動と攻撃を交互にするための数

		//崩れそうな床から逃げる
		bool isEvacuating_ = false; //逃げている途中か
		Cell evacuationDestination_{}; //逃げ先のマス

		//近接攻撃
		bool isWindingUp_ = false; //予告中か
		float windup_ = 0.0f; //当たるまでの残り時間
		bool isMeleeTellVisible_ = false;
		float meleeTellSize_ = 4.0f; //届く距離の2倍の予告の大きさ

		//跳ぶ
		bool isJumping_ = false;
		float jumpElapsed_ = 0.0f;
		float jumpDuration_ = 0.0f;
		RyoEngine::Vector3 jumpOrigin_{};
		bool landedThisTick_ = false;

		//パンチ
		PunchPhase punchPhase_ = PunchPhase::None;
		RyoEngine::Vector3 punchDirection_{};
		float punchTimer_ = 0.0f;
		bool isPunchResolved_ = false; //このパンチの当たり判定を済ませたか
		bool punchHitThisTick_ = false;
		RyoEngine::Vector3 punchPushDirection_{};
		bool isPunchTellVisible_ = false;
		float punchTellRange_ = 4.0f;
		float punchTellWidth_ = 2.5f;
		RyoEngine::Vector3 fistLocal_{}; //ボスのローカルの拳の今の位置

		// 突進
		Rush rush_{};

		// 地面パンチ
		GroundPunch groundPunch_{};
		bool isGroundPunchRequested_ = false; //試し用: 次に行動を選ぶときに地面パンチする

		//HPと形態
		BossPhaseController phaseController_;

		//コンボ
		BossComboRunner comboRunner_;

		//見た目
		BossView view_;
	};
}
