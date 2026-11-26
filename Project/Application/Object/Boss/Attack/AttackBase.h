#pragma once
#include <RyoEngine.h>
#include "../BossTuning.h"
#include "../../../Stage/Floor/Floor.h"

namespace App {
    class AttackBase {
    public:
        enum class AttackPhase {
            Announcement,
            Attack,
            Recovery,

            Count,

            None,
        };


        virtual ~AttackBase() = default;
        virtual void Start(const Floor& floor, const RyoEngine::Vector3& bossPos, const RyoEngine::Vector3& target, const BossTuning& tuning) = 0;
        virtual int32_t Update(const Floor& floor, const RyoEngine::Vector3& bossPos, const RyoEngine::Vector3& target, const BossTuning& tuning, float deltaTime) = 0; //当たったらダメージ
        virtual void Cancel() { phase_ = AttackPhase::None; }
        virtual bool IsActive() const { return phase_ != AttackPhase::None; }

        //次のStartだけ硬直を飛ばす。コンボの途中の攻撃で使う
        void SetSkipRecovery(bool skipRecovery) { skipRecovery_ = skipRecovery; }

        /// <summary>
        /// 任意のタイム減少（ScaleTimeで減算）
        /// </summary>
        /// <param name="time">減らしたいタイム</param>
        /// <param name="nextPhase">次にしたいフェーズ</param>
        inline void TimeDecrease(float& time, AttackPhase nextPhase) {
            time -= RyoEngine::GetScaleTime();

            if (time <= 0.0f) {
                time = 0.0f;
                phase_ = nextPhase;
            }
        }

    protected:
        // 共通フェーズ
        AttackPhase phase_ = AttackPhase::None;

        float announcementTime_ = 0.0f; // 予告時間
        float recoveryTime_ = 0.0f; // 硬直時間
        bool skipRecovery_ = false; // 次のStartだけ硬直を飛ばすか
    };
}