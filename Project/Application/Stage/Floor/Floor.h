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
		//穴のとなりでヒビが入り、飛ばすと1ダメージの床か
		bool IsHoleAdjacent(const Cell& cell) const;

		//着地したマスから見た目だけぷるるんと揺らす
		void Jiggle(const Cell& center, float power) { view_.Jiggle(*this, center, power); }
		//地面パンチで、歩数だけ遅らせてマスを跳ねさせる
		void Bounce(const std::vector<int32_t>& steps, float height, float stepDelay, float speed, float damping) { view_.Bounce(steps, height, stepDelay, speed, damping); }

		//タイル
		size_t GetTileCount() const { return tiles_.size(); }
		//範囲の中のタイルの状態を読む
		const FloorTile& GetTile(const Cell& cell) const { return tiles_[ToIndex(cell)]; }
		//足場を飛ばすときにタイルの位置と回転と大きさを変える
		void SetTileMotion(const Cell& cell, const RyoEngine::Vector3& position, const RyoEngine::Vector3& rotate, float scaleRate);
		//タイルを見えなくする
		void HideTile(const Cell& cell);
		//残った床の崩れる予定と切り離しを消して飛ばせる状態にする
		void PrepareBarrage();

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

		//崩すマスの予告
		//前の予告を消してそのマスに崩す予告を出す
		void SetPreview(const std::vector<Cell>& cells, const RyoEngine::Vector4& color, float blend);
		//予告を消す
		void ClearPreview();
		const RyoEngine::Vector4& GetPreviewColor() const { return previewColor_; }
		float GetPreviewBlend() const { return previewBlend_; }
		//残った足場が浮いてボスへ飛ぶ前に、全部のタイルを光らせる色と強さで、強さ0で消す
		void SetChargeGlow(const RyoEngine::Vector3& color, float strength) { chargeColor_ = color; chargeGlow_ = strength; }
		const RyoEngine::Vector3& GetChargeColor() const { return chargeColor_; }
		float GetChargeGlow() const { return chargeGlow_; }
		//ボスやプレイヤーが落ちるのを見せるために、床があるタイルをalphaの濃さに透かし、0で描かない
		void SetFade(float alpha) { fade_ = alpha; }
		float GetFade() const { return fade_; }

		//切り離し
		//ボスの島がより大きい島から切り離されていたら切り離された床にしてtrue
		bool TryIsolate(const Cell& bossCell);
		//揺れ始めてからの時間で切り離された床を揺らす
		void Shake(float elapsed, float amplitude, float frequency);
		//見た目を残して切り離された床を穴にする
		void Release();
		//切り離されて落ちていく床のマス
		const std::vector<Cell>& GetDroppingCells() const { return dropping_; }
		//切り離された床をdistanceだけ下げる
		void AnimateDrop(float distance);
		//穴になった床を全部消す
		void FinishDrop();

		//演出とチュートリアルから床を直接いじる
		//見えているタイルを全部rateの大きさにして、0なら消す
		void ShrinkAll(float rate);
		//穴になったマスをCSVのときの床に戻して、戻せたらtrue
		bool RestoreTile(const Cell& cell);
		//崩す予定のマスの穴になるまでの残りを、ハンマーが当たった瞬間に合わせ直す
		void SyncCollapse(const Cell& cell, float remaining);
		//残っている床の崩れる予定を全部やめる
		void CancelCollapses();

	private:
		void OpenHole(const Cell& cell); //穴にする
		void Restore(const Cell& cell); //1マスを元に戻す
		FloorTile& At(const Cell& cell) { return tiles_[ToIndex(cell)]; }

		bool isForeground_ = true;
		int32_t solidTileCount_ = 0;
		int32_t startSolidTileCount_ = 0;

		RyoEngine::Vector4 previewColor_ = { 1.0f, 1.0f, 1.0f, 1.0f }; //光る色にも使う予告の色
		float previewBlend_ = 0.0f; //予告の色をどれだけ混ぜるか
		RyoEngine::Vector3 chargeColor_{}; //ためるときに光らせる色
		float chargeGlow_ = 0.0f; //ためるときの光の強さで、0なら光らない
		float fade_ = 1.0f; //床があるタイルの濃さで、1で透けていない

		std::vector<FloorTile> tiles_; //手前の行から順に左から右へ並ぶタイル
		std::vector<Cell> dropping_; //切り離されて落ちる床
		std::vector<Cell> previewCells_; //予告を出しているマス
		FloorView view_; //見た目
	};
}
