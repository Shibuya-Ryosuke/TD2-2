#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <string>
#include <vector>
#include "Cell.h"
#include "FloorGrid.h"
#include "FloorTile.h"
#include "FloorView.h"

namespace App
{
	// 1層ぶんの床 (Unity のプロトタイプの FloorGrid と同じ動き)
	// ・CSV (0: 穴 / 1: 床) を読んでタイルを並べる。床全体の真ん中が原点、タイルの上の面の高さが GetHeight()
	// ・崩すと決めたタイルは、少したつと穴になって下へ落ちる (ScheduleCollapse → Tick)
	//   崩れるまでの間は 色が変わる → 点滅 → 赤 になる
	// ・崩す形の予告を床に出す (SetPreview / ClearPreview)
	// ・プレイヤー・ボスがマスからマスへ跳ぶときの着地マス (JumpLanding)
	// ・切り離された床の揺れ・落下 (Shake / Release / AnimateDrop / FinishDrop)
	// 役割ごとに分けたクラス:
	// ・マス目の計算 (マス ⇔ 位置・範囲・跳ぶ道すじ) → FloorGrid (親クラス)
	// ・CSV を読む → FloorCsv
	// ・道さがし・切り離しの判定 → FloorPathFinder (ボスの次のマス・逃げ先・たどり着けるか は直接呼ぶ)
	// ・見た目 (モデル・色・光り方) → FloorView
	// ・残った足場をボスへ飛ばす → TileBarrage (Game 側)
	class Floor : public FloorGrid
	{
	public:
		Floor() = default;
		~Floor() = default;

		Floor(const Floor&) = delete;
		Floor& operator=(const Floor&) = delete;

		// CSV を読んでタイルを並べる。読めなかったら 10x10 の床にする
		// csvPath: 例: "Resources/ApplicationResources/Stage/layer1.csv"
		// height: タイルの上の面の高さ
		// isForeground: 今いる層か (false なら暗く描く)
		void Initialize(const std::string& csvPath, float height, bool isForeground);

		// タイルの色・行列を確定する (毎フレーム、動かし終わったあとに呼ぶ)
		void UpdateVisuals();
		// 見えているタイルを描く
		void Draw();

		// 今いる層として明るく描く (下の層から上がってきたとき)
		void ShowAsCurrentFloor() { isForeground_ = true; }
		// 今いる層か (false なら暗く描く)
		bool IsForeground() const { return isForeground_; }

		// --- 形 (マスの数・大きさ・マス ⇔ 位置 は FloorGrid) ---
		// どれだけ床が消えたか (0: 全部ある 〜 1: 全部消えた)
		float GetCollapsedFraction() const;

		// --- マスの状態 ---
		bool IsSolid(const Cell& cell) const; // 今、床があるか (範囲外は false)
		bool IsHole(const Cell& cell) const; // 穴か (範囲の中で、床がない所)
		bool IsIsolated(const Cell& cell) const; // 切り離された床か
		bool IsCollapseScheduled(const Cell& cell) const; // 崩れる予定か
		// 穴のとなりなら 4 (足場を飛ばすときのダメージの倍率)
		int32_t GetBarrageMultiplier(const Cell& cell) const;

		// --- タイル (FloorView・TileBarrage・FloorPathFinder 用) ---
		size_t GetTileCount() const { return tiles_.size(); }
		// タイルの状態 (読むだけ。範囲の中のマスだけ)
		const FloorTile& GetTile(const Cell& cell) const { return tiles_[ToIndex(cell)]; }
		// タイルの位置・回転・大きさを変える (足場を飛ばすとき)
		void SetTileMotion(const Cell& cell, const RyoEngine::Vector3& position, const RyoEngine::Vector3& rotate, float scaleRate);
		// タイルを見えなくする
		void HideTile(const Cell& cell);

		// --- マスからマスへ跳ぶ ---
		// from から direction へ跳んだときの着地マス
		// maxGap: 穴をいくつまで飛び越えられるか (いつもは 1。ジャンプのアイテムを持っていると 2)
		Cell JumpLanding(const Cell& from, const Cell& direction, int32_t maxGap) const;

		// --- 床を崩す ---
		// そのマスを delay 秒後に崩す。崩すことにできたら true (床がない・もう崩す予定・切り離された床は false)
		bool ScheduleCollapse(const Cell& cell, float delay);
		// いくつかのマスをまとめて崩す。崩すことにできた数を返す
		int32_t ScheduleCellsCollapse(const std::vector<Cell>& cells, float delay);
		// 崩す予定のタイルの時間を進めて、時間が来たら穴にする。穴ができたら true。穴になったタイルを落とす
		bool Tick(float deltaTime);
		// 全部のタイルを、CSV を読んだときの形に戻す
		void Reset();

		// --- 崩す形の予告 ---
		// そのマスに、崩す形の予告を出す (前の予告は消える)。blend: 色をどれだけ混ぜるか
		void SetPreview(const std::vector<Cell>& cells, const RyoEngine::Vector4& color, float blend);
		// 予告を消す
		void ClearPreview();
		const RyoEngine::Vector4& GetPreviewColor() const { return previewColor_; }
		float GetPreviewBlend() const { return previewBlend_; }

		// --- 切り離し ---
		// ボスのいる島が、それより大きい島から切り離されていたら、その島を「切り離された床」にして true
		bool TryIsolate(const Cell& bossCell);
		// 切り離された床を揺らす (elapsed: 揺れ始めてからの時間)
		void Shake(float elapsed, float amplitude, float frequency);
		// 切り離された床を穴にする (見た目はまだ残る)
		void Release();
		// 切り離された床を distance だけ下げる
		void AnimateDrop(float distance);
		// 穴になった床を全部消す
		void FinishDrop();

	private:
		void OpenHole(const Cell& cell); // 穴にする
		void Restore(const Cell& cell); // 1マスを元に戻す
		FloorTile& At(const Cell& cell) { return tiles_[ToIndex(cell)]; }

		bool isForeground_ = true;
		int32_t solidTileCount_ = 0;
		int32_t startSolidTileCount_ = 0;

		RyoEngine::Vector4 previewColor_ = { 1.0f, 1.0f, 1.0f, 1.0f }; // 予告の色 (光る色にも使う)
		float previewBlend_ = 0.0f;                                    // 予告の色をどれだけ混ぜるか

		std::vector<FloorTile> tiles_;   // 手前の行から順に、左から右へ
		std::vector<Cell> dropping_;     // 切り離されて落ちる床
		std::vector<Cell> previewCells_; // 予告を出しているマス
		FloorView view_;                 // 見た目 (モデル)
	};
}
