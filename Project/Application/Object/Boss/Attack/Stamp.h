#pragma once
#include <RyoEngine.h>
#include <vector>
#include "AttackBase.h"
#include "../../../Stage/Grid/Cell.h"

namespace App {
	//その場で止まってプレイヤーのマスに跳び、着地で周りの9マスを攻撃する
	class Stamp : public AttackBase {
	public:
		void Start(const Floor& floor, const RyoEngine::Vector3& bossPos, const RyoEngine::Vector3& target, const BossTuning& tuning) override;
		int32_t Update(const Floor& floor, const RyoEngine::Vector3& bossPos, const RyoEngine::Vector3& target, const BossTuning& tuning, float deltaTime) override; //当たったらダメージ
		//スタンプをやめて警告を消す
		void Cancel() override;

		//跳んでいる途中のボスの位置
		const RyoEngine::Vector3& GetPosition() const { return position_; }
		//このフレームで着地したか。穴なら落ちる、切り離しの判定に使う
		bool IsLandedThisTick() const { return isLandedThisTick_; }

		//警告を出すマスの真ん中の位置
		const std::vector<RyoEngine::Vector3>& GetWarningCenters() const { return warningCenters_; }
		//1マスの大きさ
		const RyoEngine::Vector2& GetTileSize() const { return tileSize_; }

		//調整する値をParamEditorに登録する
		void RegisterParams();

	private:
		//跳んで、着地したらプレイヤーとの当たり判定をする。当たったらダメージを返す
		int32_t UpdateJump(const Floor& floor, const RyoEngine::Vector3& target, const BossTuning& tuning, float deltaTime);

	private:
		RyoEngine::Vector3 startPos_{}; // 跳ぶ前の位置
		RyoEngine::Vector3 endPos_{};   // 着地する位置
		Cell targetCell_{};             // 着地するマス
		RyoEngine::Vector3 position_{}; // 今の位置

		float jumpTimer_ = 0.0f;        // 跳んでからの経過時間
		bool isLandedThisTick_ = false; // このフレームで着地したか

		std::vector<Cell> warningCells_{};               // 警告と当たり判定の9マス
		std::vector<RyoEngine::Vector3> warningCenters_; // その真ん中の位置
		RyoEngine::Vector2 tileSize_{};                  // 1マスの大きさ

		// 調整する値
		float announcementDuration_ = 1.0f; // 予告の秒数
		float jumpHeight_ = 3.0f;           // 跳ぶ高さ
		float jumpDuration_ = 0.6f;         // 跳んでから着地までの秒数
		float recoveryDuration_ = 0.6f;     // 硬直の秒数
	};
}