#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "Cell.h"

namespace App {
	/// <summary>
	/// 1層ぶんの床 (Unity のプロトタイプの FloorGrid と同じ動き)。
	/// ・CSV (0: 穴 / 1: 床) を読んでタイルを並べる。床全体の真ん中が原点、タイルの上の面の高さが GetHeight()
	/// ・崩すと決めたタイルは、少したつと穴になって下へ落ちる (ScheduleCollapse → Tick)
	///   崩れるまでの間は 色が変わる → 点滅 → 赤 になる
	/// ・崩す形の予告を床に出す (SetPreview / ClearPreview)
	/// ・プレイヤー・ボスがマスからマスへ跳ぶときの位置 (JumpLanding / StepOffset / AlongGrid)
	/// ・ボスの道さがし (NextStep)・逃げ先さがし (TryFindLandingNeighbor)
	/// ・ボスのいる島が切り離されたか (TryIsolate)・おたがいにたどり着けるか (CanReunite)
	/// ・切り離された床の揺れ・落下 (Shake / Release / AnimateDrop / FinishDrop)
	/// ・残った足場がボスへ飛んでいく (PrepareBarrage / AnimateBarrage)
	/// </summary>
	class Floor {
	public:
		Floor() = default;
		~Floor() = default;

		Floor(const Floor&) = delete;
		Floor& operator=(const Floor&) = delete;

		/// <summary>
		/// CSV を読んでタイルを並べる。読めなかったら 10x10 の床にする
		/// </summary>
		/// <param name="csvPath">例: "Resources/ApplicationResources/Stage/layer1.csv"</param>
		/// <param name="height">タイルの上の面の高さ</param>
		/// <param name="isForeground">今いる層か (false なら暗く描く)</param>
		void Initialize(const std::string& csvPath, float height, bool isForeground);

		/// <summary>タイルの色・行列を確定する (毎フレーム、動かし終わったあとに呼ぶ)</summary>
		void UpdateVisuals();
		/// <summary>見えているタイルを描く</summary>
		void Draw();

		/// <summary>今いる層として明るく描く (下の層から上がってきたとき)</summary>
		void ShowAsCurrentFloor() { isForeground_ = true; }

		// --- 形 ---
		int32_t GetWidth() const { return width_; }
		int32_t GetDepth() const { return depth_; }
		float GetHeight() const { return height_; }
		/// <summary>どれだけ床が消えたか (0: 全部ある 〜 1: 全部消えた)</summary>
		float GetCollapsedFraction() const;

		// --- マス ---
		/// <summary>その位置が何マス目か (床の外なら、範囲外の番号が返る)</summary>
		Cell GetCellAt(const RyoEngine::Vector3& position) const;
		/// <summary>マスの真ん中の位置 (高さはタイルの上の面)</summary>
		RyoEngine::Vector3 GetCellCenter(const Cell& cell) const;
		/// <summary>床の外に出ないように位置を収める (radius: 体の半径)</summary>
		RyoEngine::Vector3 ClampToBoundary(const RyoEngine::Vector3& position, float radius) const;
		bool Contains(const Cell& cell) const; // 床の範囲の中か
		/// <summary>今、床があるか (範囲外は false)</summary>
		bool IsSolid(const Cell& cell) const;
		/// <summary>穴か (範囲の中で、床がない所)</summary>
		bool IsHole(const Cell& cell) const;
		/// <summary>切り離された床か</summary>
		bool IsIsolated(const Cell& cell) const;

		// --- マスからマスへ跳ぶ ---
		/// <summary>from から direction へ跳んだときの着地マス (となりが穴で、その先が床なら飛び越える)</summary>
		Cell JumpLanding(const Cell& from, const Cell& direction) const;
		/// <summary>マスの向き → 1マス分のずれ (ワールドの長さ)</summary>
		RyoEngine::Vector3 StepOffset(const Cell& direction) const;
		/// <summary>
		/// start から end へ、マスの線にそって進む (先に x、次に z。progress: 0 〜 1。高さは直線で変わる)
		/// </summary>
		static RyoEngine::Vector3 AlongGrid(const RyoEngine::Vector3& start, const RyoEngine::Vector3& end, float progress);

		// --- 床を崩す ---
		/// <summary>そのマスを delay 秒後に崩す。崩すことにできたら true (床がない・もう崩す予定・切り離された床は false)</summary>
		bool ScheduleCollapse(const Cell& cell, float delay);
		/// <summary>いくつかのマスをまとめて崩す。崩すことにできた数を返す</summary>
		int32_t ScheduleCellsCollapse(const std::vector<Cell>& cells, float delay);
		/// <summary>崩す予定のタイルの時間を進めて、時間が来たら穴にする。穴ができたら true。穴になったタイルを落とす</summary>
		bool Tick(float deltaTime);
		/// <summary>全部のタイルを、CSV を読んだときの形に戻す</summary>
		void Reset();

		// --- 崩す形の予告 ---
		/// <summary>そのマスに、崩す形の予告を出す (前の予告は消える)。blend: 色をどれだけ混ぜるか</summary>
		void SetPreview(const std::vector<Cell>& cells, const RyoEngine::Vector4& color, float blend);
		/// <summary>予告を消す</summary>
		void ClearPreview();

		// --- ボスの道さがし ---
		/// <summary>start から target へ向かうときの、次のマス (canJump: 穴を1つ飛び越えられる)</summary>
		Cell NextStep(const Cell& start, const RyoEngine::Vector3& target, bool canJump) const;
		/// <summary>上下左右のマスで、timeToLand 秒のうちに崩れない一番近い床を探す</summary>
		bool TryFindLandingNeighbor(const RyoEngine::Vector3& position, float timeToLand, Cell& landing) const;

		// --- 切り離し ---
		/// <summary>ボスのいる島が、それより大きい島から切り離されていたら、その島を「切り離された床」にして true</summary>
		bool TryIsolate(const Cell& bossCell);
		/// <summary>プレイヤーとボスが、おたがいにたどり着けるか (canCrossGaps: 穴を1つ飛び越えられる)</summary>
		bool CanReunite(const Cell& playerCell, const Cell& bossCell, bool canCrossGaps) const;
		/// <summary>切り離された床を揺らす (elapsed: 揺れ始めてからの時間)</summary>
		void Shake(float elapsed, float amplitude, float frequency);
		/// <summary>切り離された床を穴にする (見た目はまだ残る)</summary>
		void Release();
		/// <summary>切り離された床を distance だけ下げる</summary>
		void AnimateDrop(float distance);
		/// <summary>穴になった床を全部消す</summary>
		void FinishDrop();

		// --- 残った足場をボスへ飛ばす ---
		/// <summary>飛ばす足場を決める (今ある床全部)。数を返す</summary>
		int32_t PrepareBarrage();
		/// <summary>
		/// 足場を飛ばす。ボスまで届いた数を返す
		/// </summary>
		/// <param name="target">飛んでいく先</param>
		/// <param name="elapsed">飛ばし始めてからの時間</param>
		/// <param name="damageUnits">届いた足場のダメージの合計 (穴のとなりは 4)</param>
		int32_t AnimateBarrage(const RyoEngine::Vector3& target, float elapsed, float flightDuration, float launchWindow, int64_t& damageUnits);

	private:
		struct Tile {
			bool isSolid = false;         // 今、床があるか
			bool isSolidAtStart = false;  // CSV での形 (Reset でここに戻す)
			bool isIsolated = false;      // 切り離された床か
			bool isVisible = false;       // 描くか
			bool isPreview = false;       // 崩す形の予告を出しているか
			float remaining = -1.0f;      // 崩れるまでの残り時間 (負: 崩す予定はない)
			float fallTime = 0.0f;        // 穴になってから落ちている時間
			RyoEngine::Vector3 position{};  // 今の位置 (揺れ・落下・飛んでいく途中も)
			RyoEngine::Vector3 rotate{};    // 今の回転 (度。飛んでいく途中だけ回る)
			float scaleRate = 1.0f;         // 今の大きさの割合 (飛んでいく途中だけ小さくなる)
		};

		struct BarrageTile {
			Cell cell{};
			int32_t multiplier = 1; // ダメージの倍率 (穴のとなりは 4)
		};

		bool LoadCsv(const std::string& csvPath); // CSV を読む
		void CreateDefaultTiles(); // CSV が読めなかったときの 10x10 の床
		void CreateModels(); // モデルを作る
		void OpenHole(const Cell& cell); // 穴にする
		void Restore(const Cell& cell); // 1マスを元に戻す
		RyoEngine::Vector3 GetRestPosition(const Cell& cell) const; // タイルの真ん中の、いつもの位置
		int32_t GetBarrageMultiplier(const Cell& cell) const;       // 穴のとなりなら 4
		RyoEngine::Vector4 GetBaseColor(const Cell& cell) const;    // 切り離し・穴のとなり・ふつう の色
		RyoEngine::Vector4 GetTileColor(const Cell& cell) const;    // 今の色 (崩れる前の点滅・予告も入れる)
		bool IsPreviewGlowing(const Cell& cell) const;              // 予告で光らせるマスか

		/// <summary>
		/// start から、床の上をたどって行ける所を全部さがす (幅優先)。
		/// parents: マスごとの「1つ前のマス」の番号 (-1: 行けない)、order: 見つけた順
		/// </summary>
		void Explore(const Cell& start, bool canJump, std::vector<int32_t>& parents, std::vector<Cell>& order) const;

		size_t ToIndex(const Cell& cell) const; // マス → 配列の番号
		Cell ToCell(size_t index) const; // 配列の番号 → マス
		Tile& At(const Cell& cell) { return tiles_[ToIndex(cell)]; }
		const Tile& At(const Cell& cell) const { return tiles_[ToIndex(cell)]; }

		int32_t width_ = 0;
		int32_t depth_ = 0;
		float tileSizeX_ = 2.0f;
		float tileSizeZ_ = 2.0f;
		float height_ = 0.0f;
		bool isForeground_ = true;
		int32_t solidTileCount_ = 0;
		int32_t startSolidTileCount_ = 0;

		RyoEngine::Vector4 previewColor_ = { 1.0f, 1.0f, 1.0f, 1.0f }; // 予告の色 (光る色にも使う)
		float previewBlend_ = 0.0f;                                    // 予告の色をどれだけ混ぜるか
		float previewGlowTime_ = 0.0f;                                 // 予告の明滅の時間

		std::vector<Tile> tiles_;                                // 手前の行から順に、左から右へ
		std::vector<std::unique_ptr<RyoEngine::Model>> models_;  // tiles_ と同じ並び (CSV で穴のマスは nullptr)
		std::vector<Cell> dropping_;                             // 切り離されて落ちる床
		std::vector<BarrageTile> barrage_;                       // ボスへ飛ばす足場
		std::vector<Cell> previewCells_;                         // 予告を出しているマス
	};
}
