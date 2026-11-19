#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <memory>
#include "../Game/BossTuning.h"
#include "../Stage/Floor.h"

namespace App
{
	//ボス (Unity のプロトタイプの BossAgent と同じ動き)。
	//マスからマスへ1マスずつ跳んで、プレイヤーへ向かって進む
	//近接攻撃: 足元に予告を出して、少したってから当たる
	//パンチ: 前方に予告 → 拳が伸びる。当たるとはじかれる (4層目から)
	//足元の床が崩れたら、となりの床へ跳んで逃げる (TryEvadeCollapse)
	//プレイヤーが跳んでぶつかってくると、はじかれる (KnockBack)
	//動かす順番はゲームシーン (Game) が決めるので、GameObject にはしていない。
	class Boss
	{
	public:
		Boss() = default;
		~Boss() = default;

		Boss(const Boss&) = delete;
		Boss& operator=(const Boss&) = delete;

		void Initialize(); // モデルを作る (体・拳・攻撃の予告)

		//床の上の位置に置く (攻撃・弾は全部やめる)
		void Place(const Floor& floor, const RyoEngine::Vector3& position);
		//攻撃・ジャンプ・弾を全部やめる
		void Stop();

		//足元の床が崩れたとき、跳んで逃げられたら true
		bool TryEvadeCollapse(const Floor& floor, const BossTuning& tuning);

		//direction の向きのとなりのマスへ、duration 秒で跳ばされる (床の外なら今のマスで跳ねる)
		void KnockBack(const Floor& floor, const Cell& direction, float duration);

		//1フレーム動かす。プレイヤーに当たった攻撃のダメージを返す (当たらなければ 0)
		/// <param name="target">プレイヤーの足元の位置
		/// <param name="moveSpeed">進む速さ (床が減るほど速くなる)
		int32_t Tick(const Floor& floor, const RyoEngine::Vector3& target, const BossTuning& tuning, float moveSpeed, float deltaTime);

		//行列を確定する (動かし終わったあとに呼ぶ)
		void UpdateVisuals();
		void Draw(); // 体・拳・攻撃の予告を描く

		bool IsJumping() const { return isJumping_; }
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

		//>ボスのローカル座標 → ワールド座標 (向きだけ回す)
		RyoEngine::Vector3 ToWorld(const RyoEngine::Vector3& local) const;
		//ワールド座標 → ボスのローカル座標
		RyoEngine::Vector3 ToLocal(const RyoEngine::Vector3& world) const;

		//描くか (倒したら消す)
		void SetActive(bool isActive) { isActive_ = isActive; }

	private:
		enum class PunchPhase 
		{
			None,     // パンチしていない
			Windup,   // 予告 (拳を後ろに引く)
			Strike,   // 拳が伸びる
			Recovery, // 拳が戻る
		};

		int32_t AdvancePunch(const Floor& floor, const RyoEngine::Vector3& target, const BossTuning& tuning, float deltaTime); // パンチを進める (予告 → 伸びる → 戻る)。当たったらダメージを返す
		void CancelPunch(); // パンチをやめて、拳を戻す
		void StartJump(const Floor& floor, const Cell& landing, float duration); // landing のマスへ duration 秒で跳び始める
		void AdvanceJump(const Floor& floor, const BossTuning& tuning, float deltaTime); // 跳んでいる途中を進める
		void LookAt(const RyoEngine::Vector3& direction); // その向きを向く (y は無視)

		//状態
		RyoEngine::Vector3 position_{}; // 足元の位置
		float yawDeg_ = 0.0f;           // 向き (度。+Z が 0)
		bool isActive_ = true;
		Cell cell_{};                   // 今いるマス
		Cell next_{};                   // 次に向かうマス
		float meleeCooldown_ = 0.0f;

		// 近接攻撃
		bool isWindingUp_ = false;      // 予告中か
		float windup_ = 0.0f;           // 当たるまでの残り時間
		bool isMeleeTellVisible_ = false;
		float meleeTellSize_ = 4.0f;    // 予告の大きさ (届く距離 x 2)

		//跳ぶ (歩くときも1マスずつ跳ぶ)
		bool isJumping_ = false;
		float jumpElapsed_ = 0.0f;
		float jumpDuration_ = 0.0f;
		RyoEngine::Vector3 jumpOrigin_{};
		bool landedThisTick_ = false;

		//パンチ
		PunchPhase punchPhase_ = PunchPhase::None;
		RyoEngine::Vector3 punchDirection_{};
		float punchTimer_ = 0.0f;
		float punchCooldown_ = 0.0f;
		bool isPunchResolved_ = false;  // このパンチの当たり判定を済ませたか
		bool punchHitThisTick_ = false;
		RyoEngine::Vector3 punchPushDirection_{};
		bool isPunchTellVisible_ = false;
		float punchTellRange_ = 4.0f;
		float punchTellWidth_ = 2.5f;
		RyoEngine::Vector3 fistLocal_{}; // 拳の今の位置 (ボスのローカル)

		//見た目 (仮: cube の色を変えたもの)
		std::unique_ptr<RyoEngine::Model> body_{};
		std::unique_ptr<RyoEngine::Model> fist_{};
		std::unique_ptr<RyoEngine::Model> meleeTell_{};
		std::unique_ptr<RyoEngine::Model> punchTell_{};
	};
}
