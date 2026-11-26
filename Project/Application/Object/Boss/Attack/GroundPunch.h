#pragma once
#include <RyoEngine.h>
#include <vector>
#include "AttackBase.h"
#include "../../../Stage/Grid/Cell.h"

namespace App {
	//その場で地面を叩き、足元の十字に直撃、盤面全体に波を広げる攻撃
	class GroundPunch : public AttackBase {
	public:

		void Start(const Floor& floor, const RyoEngine::Vector3& bossPos, const RyoEngine::Vector3& target, const BossTuning& tuning) override;
		int32_t Update(const Floor& floor, const RyoEngine::Vector3& bossPos, const RyoEngine::Vector3& target, const BossTuning& tuning, float deltaTime) override;
		//地面パンチをやめて警告を消す
		void Cancel() override;

		const std::vector<RyoEngine::Vector3>& GetWarningCenters() const { return warningCenters_; }
		const RyoEngine::Vector2& GetTileSize() const { return tileSize_; }
		bool IsHitThisTick() const { return isHitThisTick_; }
		const Cell& GetKnockbackDirection() const { return knockbackDirection_; }
		int32_t GetKnockbackCells() const { return knockbackCells_; }

		//このフレームで地面を叩いたか。盤面を跳ねさせるのに使う
		bool IsImpactThisTick() const { return isImpactThisTick_; }
		//各マスまでの歩数。盤面を跳ねさせるのに使う
		const std::vector<int32_t>& GetSteps() const { return steps_; }
		//1マス伝わるのにかかる秒数
		float GetWaveStepDelay() const { return waveStepDelay_; }
		//跳ねる高さ
		float GetBounceHeight() const { return bounceHeight_; }
		//跳ねる速さ
		float GetBounceSpeed() const { return bounceSpeed_; }
		//揺れが収まる速さ
		float GetBounceDamping() const { return bounceDamping_; }

		//調整する値をParamEditorに登録する
		void RegisterParams();

	private:
		// originから各マスまでの歩数を数える
		int32_t BuildWaveSteps(const Floor& floor, const Cell& origin, std::vector<int32_t>& steps);

		// 最初のフレームで直撃判定、そのあと波を広げる
		int32_t UpdateWave(const Floor& floor, const RyoEngine::Vector3& target, const BossTuning& tuning, float deltaTime);

		//当たったことを覚えて、吹っ飛ばす向きを決める
		void Hit(const Cell& playerCell);

	private:
		// 最初の１フレームだけ当たり判定
		bool isFirstFrameChecked_ = false;

		float waveStepDelay_ = 0.05f;  // 隣のマスに伝播する速度
		int32_t waveMaxSteps_ = 4;     // 中心から何マス先まで揺らすか
		float bounceHeight_ = 0.36f;   // マスがどれほど跳ねるのか
		float bounceSpeed_ = 32.0f;    // 跳ねる速さ。大きいほど小刻みに跳ねる
		float bounceDamping_ = 7.0f;   // 揺れが収まる速さ。大きいほど早く収まる
		int32_t knockbackCells_ = 2;  // 吹っ飛ばすマスの数
		bool hasWaveHit_ = false;     // もう波に当たったか

		Cell origin_{};                 // パンチしたマス
		std::vector<int32_t> steps_;    // 各マスまでの歩数。届かないマスは-1
		int32_t maxStep_ = 0;           // 一番遠いマスの歩数
		float waveTime_ = 0.0f;         // パンチしてからの経過時間
		int32_t prevFront_ = -1;        // 前のフレームの波の先頭の歩数
		Cell knockbackDirection_{};     // 吹っ飛ばす向き
		bool isHitThisTick_ = false;    // このフレームで当たったか。外が吹っ飛ばすのに使う
		bool isImpactThisTick_ = false; // このフレームで地面を叩いたか

		std::vector<Cell> warningCells_;                // 赤くする十字5マス
		std::vector<RyoEngine::Vector3> warningCenters_; // その真ん中の位置
		RyoEngine::Vector2 tileSize_{};                  // 1マスの大きさ
	};
}
