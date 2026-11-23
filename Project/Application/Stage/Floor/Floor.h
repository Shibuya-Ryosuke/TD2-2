#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <string>
#include <vector>
#include "../Grid/Cell.h"
#include "../Grid/FloorGrid.h"
#include "FloorTile.h"
#include "FloorView.h"

namespace App
{
	//CSVからタイルを並べて、崩したり切り離したりする1層ぶんの床
	class Floor : public FloorGrid
	{
	public:
		Floor() = default;
		~Floor() = default;

		Floor(const Floor&) = delete;
		Floor& operator=(const Floor&) = delete;

		//CSVを読んでタイルを並べ、読めなかったら10x10にする
		void Initialize(const std::string& csvPath, float height, bool isForeground);

		//動かし終わったあとにタイルの色と行列を確定する
		void UpdateVisuals();
		//見えているタイルを描く
		void Draw();

		//下の層から上がってきたら今いる層として明るく描く
		void ShowAsCurrentFloor() { isForeground_ = true; }
		//今いる層か
		bool IsForeground() const { return isForeground_; }

		//形
		//0が全部あって1が全部消えた床の消えた割合
		float GetCollapsedFraction() const;

		//マスの状態
		bool IsSolid(const Cell& cell) const; //範囲外はfalseの今床があるか
		bool IsHole(const Cell& cell) const; //範囲の中で床がない穴か
		bool IsIsolated(const Cell& cell) const; //切り離された床か
		bool IsCollapseScheduled(const Cell& cell) const; //崩れる予定か
		//足場を飛ばすときに穴のとなりなら4倍
		int32_t GetBarrageMultiplier(const Cell& cell) const;

		//着地したマスから見た目だけぷるるんと揺らす
		void Jiggle(const Cell& center, float power) { view_.Jiggle(*this, center, power); }

		//タイル
		size_t GetTileCount() const { return tiles_.size(); }
		//範囲の中のタイルの状態を読む
		const FloorTile& GetTile(const Cell& cell) const { return tiles_[ToIndex(cell)]; }
		//足場を飛ばすときにタイルの位置と回転と大きさを変える
		void SetTileMotion(const Cell& cell, const RyoEngine::Vector3& position, const RyoEngine::Vector3& rotate, float scaleRate);
		//タイルを見えなくする
		void HideTile(const Cell& cell);

		//マスからマスへ跳ぶ
		//穴をmaxGap個まで飛び越えてdirectionへ跳んだときの着地マス
		Cell JumpLanding(const Cell& from, const Cell& direction, int32_t maxGap) const;

		//床を崩す
		//そのマスをdelay秒後にハートの向きで崩す予定にできたらtrue
		bool ScheduleCollapse(const Cell& cell, float delay, float heartAngle = 0.0f);
		//いくつかのマスをまとめて崩し、崩す予定にできた数を返す
		int32_t ScheduleCellsCollapse(const std::vector<Cell>& cells, float delay, float heartAngle = 0.0f);
		//崩れる時間を進めて、穴ができたらtrue
		bool Tick(float deltaTime);
		//全部のタイルをCSVを読んだときの形に戻す
		void Reset();

		//崩す形の予告
		//前の予告を消してそのマスに崩す形の予告を出す
		void SetPreview(const std::vector<Cell>& cells, const RyoEngine::Vector4& color, float blend);
		//予告を消す
		void ClearPreview();
		const RyoEngine::Vector4& GetPreviewColor() const { return previewColor_; }
		float GetPreviewBlend() const { return previewBlend_; }

		//切り離し
		//ボスの島がより大きい島から切り離されていたら切り離された床にしてtrue
		bool TryIsolate(const Cell& bossCell);
		//揺れ始めてからの時間で切り離された床を揺らす
		void Shake(float elapsed, float amplitude, float frequency);
		//見た目を残して切り離された床を穴にする
		void Release();
		//切り離された床をdistanceだけ下げる
		void AnimateDrop(float distance);
		//穴になった床を全部消す
		void FinishDrop();

	private:
		void OpenHole(const Cell& cell); //穴にする
		void Restore(const Cell& cell); //1マスを元に戻す
		FloorTile& At(const Cell& cell) { return tiles_[ToIndex(cell)]; }

		bool isForeground_ = true;
		int32_t solidTileCount_ = 0;
		int32_t startSolidTileCount_ = 0;

		RyoEngine::Vector4 previewColor_ = { 1.0f, 1.0f, 1.0f, 1.0f }; //光る色にも使う予告の色
		float previewBlend_ = 0.0f; //予告の色をどれだけ混ぜるか

		std::vector<FloorTile> tiles_; //手前の行から順に左から右へ並ぶタイル
		std::vector<Cell> dropping_; //切り離されて落ちる床
		std::vector<Cell> previewCells_; //予告を出しているマス
		FloorView view_; //見た目
	};
}
