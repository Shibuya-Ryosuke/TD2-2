#include "GroundPunch.h"
#include <deque>
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App {
	void GroundPunch::Start(const Floor& floor, const RyoEngine::Vector3& bossPos, const RyoEngine::Vector3& target, const BossTuning& tuning) {
		(void)target;

		// 開始位置はボスの位置
		origin_ = floor.GetCellAt(bossPos);

		// 足元と上下左右の5マスを警告
		const Cell cross[5] = { {0,0,},{0,1},{1,0},{0,-1},{-1,0} };
		warningCells_.clear();
		warningCenters_.clear();
		for (const Cell& offset : cross) {
			const Cell cell = origin_ + offset;
			if (!floor.Contains(cell)) {
				continue;
			}
			warningCells_.push_back(cell);
			warningCenters_.push_back(floor.GetCellCenter(cell));
		}
		tileSize_ = { floor.GetTileSizeX(),floor.GetTileSizeZ() };

		// 前の攻撃の結果を消す
		steps_.clear();
		maxStep_ = 0;
		waveTime_ = 0.0f;
		prevFront_ = -1;
		isFirstFrameChecked_ = false;
		hasWaveHit_ = false;
		isHitThisTick_ = false;
		isImpactThisTick_ = false;

		// 予備動作から始める
		announcementTime_ = tuning.groundPunchAnnouncement;
		//コンボの途中なら硬直は1フレームで終わらせる
		recoveryTime_ = skipRecovery_ ? 0.0f : tuning.groundPunchRecovery;
		skipRecovery_ = false;
		phase_ = AttackPhase::Announcement;
	}

	int32_t GroundPunch::Update(const Floor& floor, const RyoEngine::Vector3& bossPos, const RyoEngine::Vector3& target, const BossTuning& tuning, float deltaTime) {
		(void)bossPos; // パンチしたマスはStart()で保存済
		(void)deltaTime; // 時間はTimeDecreaseと同じくGetScaleTimeで進める

		// このフレームの結果を消す
		isHitThisTick_ = false;
		isImpactThisTick_ = false;
		int32_t damage = 0;

		switch (phase_) {
		case AttackPhase::Announcement:
			// 予告時間の減少
			TimeDecrease(announcementTime_, AttackPhase::Attack);
			break;

		case AttackPhase::Attack:
			damage = UpdateWave(floor, target, tuning, GetScaleTime());
			break;

		case AttackPhase::Recovery:
			// 硬直時間減少
			TimeDecrease(recoveryTime_, AttackPhase::None);
			break;

		default:
			break;
		}
		return damage;
	}

	//調整する値をParamEditorに登録する
	void GroundPunch::RegisterParams() {
		ParamEditor::BeginGroup("地面パンチ");
		ParamEditor::RegisterValue("揺らすマスの数", &waveMaxSteps_, 1.0f, 1, 50);
		ParamEditor::RegisterValue("1マス伝わる秒数", &waveStepDelay_, 0.005f, 0.01f, 1.0f);
		ParamEditor::RegisterValue("跳ねる高さ", &bounceHeight_, 0.01f, 0.0f, 5.0f);
		ParamEditor::RegisterValue("跳ねる速さ", &bounceSpeed_, 0.5f, 0.0f, 100.0f);
		ParamEditor::RegisterValue("揺れが収まる速さ", &bounceDamping_, 0.1f, 0.0f, 50.0f);
		ParamEditor::RegisterValue("吹っ飛ばすマスの数", &knockbackCells_, 1.0f, 0, 10);
		ParamEditor::EndGroup();
	}

	//地面パンチをやめて警告を消す
	void GroundPunch::Cancel() {
		AttackBase::Cancel();
		warningCells_.clear();
		warningCenters_.clear();
	}

	int32_t GroundPunch::BuildWaveSteps(const Floor& floor, const Cell& origin, std::vector<int32_t>& steps) {
		// 全マスを「まだ届いていない」にする
		steps.assign(floor.GetTileCount(), -1);
		if (!floor.Contains(origin)) {
			return 0;
		}

		// { x, z }（奥、右、手前、左）の向き
		const Cell directions[4] = { {0,1},{1,0},{0,-1},{-1,0} };
		int32_t maxStep = 0;

		// 調べるマスの行列
		std::deque<Cell> queue;
		steps[floor.ToIndex(origin)] = 0; // パンチ位置を0で書きこみ
		queue.push_back(origin);

		// 隣接しているマスへ伝播
		// 広げられなくなるまで繰り返される
		while(!queue.empty()){
			// 行列の先頭情報を貰い、それを削除
			const Cell cell = queue.front();
			queue.pop_front();
			const int32_t step = steps[floor.ToIndex(cell)];

			// 4つの向きについて
			for (const Cell& direction : directions) {
				// 向きに合わせた隣のマスを出す
				const Cell next = cell + direction;
				// 盤面の外には広げない
				if (!floor.Contains(next)) {
					continue;
				}
				// 隣が穴なら飛ばす
				if (!floor.IsSolid(next)) {
					continue;
				}
				// 既に歩数が入ってるマスは飛ばす
				if (steps[floor.ToIndex(next)] >= 0) {
					continue;
				}
				// 揺らす範囲より先には広げない
				if (step + 1 > waveMaxSteps_) {
					continue;
				}
				// 隣の歩数を今+1
				steps[floor.ToIndex(next)] = step + 1;
				maxStep = std::max(maxStep, step + 1);
				// 行列の最後に追加
				queue.push_back(next);
			}
		}
		return maxStep;
	}

	int32_t GroundPunch::UpdateWave(const Floor& floor, const Vector3& target, const BossTuning& tuning, float deltaTime) {
		const Cell playerCell = floor.GetCellAt(target);
		int32_t damage = 0;

		// 最初のフレームだけ十字チェック
		if (!isFirstFrameChecked_) {
			isFirstFrameChecked_ = true;
			isImpactThisTick_ = true;
			maxStep_ = BuildWaveSteps(floor, origin_, steps_);
			// 叩いたら警告は消す
			warningCenters_.clear();

			// 十字の中にいたら直撃。直撃したら波ではもう当てない
			if (std::find(warningCells_.begin(), warningCells_.end(), playerCell) != warningCells_.end()) {
				damage = tuning.damage;
				hasWaveHit_ = true;
				Hit(playerCell);
			}
		}

		// 波を進めて、今の先頭が何歩目かを出す
		waveTime_ += deltaTime;
		const int32_t front = static_cast<int32_t>(waveTime_ / waveStepDelay_);

		// 前のフレームの先頭の次から今の先頭までに、プレイヤーのマスがあれば当たり
		if (!hasWaveHit_ && floor.Contains(playerCell)) {
			const int32_t playerStep = steps_[floor.ToIndex(playerCell)];
			if (playerStep > prevFront_ && playerStep <= front) {
				hasWaveHit_ = true;
				Hit(playerCell);
			}
		}
		prevFront_ = front;

		// 一番遠いマスまで届いたら硬直へ。揺れの余韻は床の見た目が続ける
		if (front > maxStep_) {
			phase_ = AttackPhase::Recovery;
		}
		return damage;
	}

	//当たったことを覚えて、パンチしたマスから外へ向かう向きを決める
	void GroundPunch::Hit(const Cell& playerCell) {
		isHitThisTick_ = true;

		const Cell diff = playerCell - origin_;
		// 同じマスにいたら奥へ
		if (diff.x == 0 && diff.z == 0) {
			knockbackDirection_ = { 0, 1 };
			return;
		}
		// 差の大きいほうの軸で上下左右にそろえる
		if (std::abs(diff.x) >= std::abs(diff.z)) {
			knockbackDirection_ = { diff.x > 0 ? 1 : -1, 0 };
		} else {
			knockbackDirection_ = { 0, diff.z > 0 ? 1 : -1 };
		}
	}
}
