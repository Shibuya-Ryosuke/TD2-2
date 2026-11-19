#include "Floor.h"
#include "FloorCsv.h"
#include "../Grid/FloorPathFinder.h"
#include "../../Config/GameConfig.h"
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		// となりの4マス (奥・右・手前・左。Unity と同じ順番)
		constexpr Cell kDirections[4] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };

		constexpr float kTileFallSpeed = 12.0f; // 穴になったタイルが落ちる速さ (距離 = 12 x 時間の2乗)
		constexpr float kTileFallTime = 0.5f;   // 穴になったタイルが消えるまで(秒)
	}

	// =====================================================================
	// 初期化
	// =====================================================================
	void Floor::Initialize(const std::string& csvPath, float height, bool isForeground)
	{
		// 今いる層か (今いる層でなければ暗く出す)
		isForeground_ = isForeground;

		// CSV から床の形を読む。読めなければ 10x10
		FloorLayout layout{};
		if (!FloorCsv::Load(csvPath, layout))
		{
			Logger::LogWarning("[Floor] CSV が読めなかったので 10x10 の床にしました: {}", csvPath);
			layout = FloorCsv::CreateDefault(10, 10);
		}
		// マスの数と高さを決める (FloorGrid)
		SetupGrid(layout.width, layout.depth, height);

		// タイルを作って、最初にある床の数を数える (崩れた割合を出すのに使う)
		tiles_.assign(layout.solid.size(), FloorTile{});
		startSolidTileCount_ = 0;
		for (size_t i = 0; i < tiles_.size(); ++i)
		{
			tiles_[i].isSolidAtStart = layout.solid[i];
			if (layout.solid[i])
			{
				++startSolidTileCount_;
			}
		}

		// モデルを作って、全部のマスを最初の状態にする
		view_.CreateModels(*this);
		Reset();
	}

	// =====================================================================
	// 見た目 (FloorView に任せる)
	// =====================================================================
	void Floor::UpdateVisuals()
	{
		view_.Update(*this);
	}

	// 見えているタイルを描く
	void Floor::Draw()
	{
		view_.Draw(*this);
	}

	// =====================================================================
	// 形・マス
	// =====================================================================
	// 床がどれだけ崩れたか (0 = 全部ある / 1 = 全部ない)
	float Floor::GetCollapsedFraction() const
	{
		if (startSolidTileCount_ <= 0)
		{
			return 1.0f;
		}
		return 1.0f - static_cast<float>(solidTileCount_) / static_cast<float>(startSolidTileCount_);
	}

	// 床があるか (範囲の外は false。以下の3つも同じ)
	bool Floor::IsSolid(const Cell& cell) const
	{
		return Contains(cell) && GetTile(cell).isSolid;
	}

	// 穴か
	bool Floor::IsHole(const Cell& cell) const
	{
		return Contains(cell) && !GetTile(cell).isSolid;
	}

	// 切り離された床か
	bool Floor::IsIsolated(const Cell& cell) const
	{
		return Contains(cell) && GetTile(cell).isIsolated;
	}

	// 崩れる予定か
	bool Floor::IsCollapseScheduled(const Cell& cell) const
	{
		return Contains(cell) && GetTile(cell).remaining >= 0.0f;
	}

	// 穴のとなりなら 4 倍
	int32_t Floor::GetBarrageMultiplier(const Cell& cell) const
	{
		for (const Cell& direction : kDirections)
		{
			if (IsHole(cell + direction))
			{
				return 4;
			}
		}
		return 1;
	}

	// タイルの位置・回転・大きさを変える
	void Floor::SetTileMotion(const Cell& cell, const Vector3& position, const Vector3& rotate, float scaleRate)
	{
		FloorTile& tile = At(cell);
		tile.position = position;
		tile.rotate = rotate;
		tile.scaleRate = scaleRate;
	}

	// タイルを見えなくする
	void Floor::HideTile(const Cell& cell)
	{
		At(cell).isVisible = false;
	}

	// =====================================================================
	// マスからマスへ跳ぶ
	// =====================================================================
	// direction へ跳んだときに着くマス (穴が maxGap 個までなら飛び越える)
	Cell Floor::JumpLanding(const Cell& from, const Cell& direction, int32_t maxGap) const
	{
		// 穴が続く間は先へ進む (maxGap 個まで)
		Cell next = from + direction;
		int32_t holeCount = 0;
		while (holeCount < maxGap && IsHole(next))
		{
			next = next + direction;
			++holeCount;
		}
		// 穴を越えた先が床なら、そこに着地する
		if (holeCount > 0 && IsSolid(next))
		{
			return next;
		}
		return from + direction;
	}

	// =====================================================================
	// 床を崩す
	// =====================================================================
	// 1マスを delay 秒後に崩す予定にする (できたら true)
	bool Floor::ScheduleCollapse(const Cell& cell, float delay)
	{
		if (!IsSolid(cell))
		{
			return false;
		}
		FloorTile& tile = At(cell);
		if (tile.remaining >= 0.0f || tile.isIsolated)
		{
			return false; // もう崩す予定・切り離された床
		}
		tile.remaining = delay;
		return true;
	}

	// いくつかのマスをまとめて崩す
	int32_t Floor::ScheduleCellsCollapse(const std::vector<Cell>& cells, float delay)
	{
		int32_t count = 0;
		for (const Cell& cell : cells)
		{
			if (ScheduleCollapse(cell, delay))
			{
				++count;
			}
		}
		return count;
	}

	// 崩れる時間を進める・穴になったタイルを落とす
	bool Floor::Tick(float deltaTime)
	{
		bool isChanged = false;
		for (int32_t z = 0; z < depth_; ++z)
		{
			for (int32_t x = 0; x < width_; ++x)
			{
				const Cell cell = { x, z };
				FloorTile& tile = At(cell);
				if (tile.isIsolated)
				{
					continue; // 切り離された床は、揺れ・落下を別に動かす
				}

				// 穴になったタイル: 下へ落ちていって消える
				if (!tile.isSolid)
				{
					if (!tile.isVisible)
					{
						continue;
					}
					tile.fallTime += deltaTime;
					Vector3 position = GetRestPosition(cell);
					position.y -= kTileFallSpeed * tile.fallTime * tile.fallTime;
					tile.position = position;
					if (tile.fallTime >= kTileFallTime)
					{
						tile.isVisible = false;
					}
					continue;
				}

				// 崩れる予定のタイル: 時間が来たら穴にする
				if (tile.remaining < 0.0f)
				{
					continue;
				}
				tile.remaining -= deltaTime;
				if (tile.remaining > 0.0f)
				{
					continue;
				}
				tile.remaining = 0.0f;
				OpenHole(cell);
				isChanged = true;
			}
		}
		return isChanged;
	}

	// 穴にする
	void Floor::OpenHole(const Cell& cell)
	{
		At(cell).isSolid = false;
		--solidTileCount_;
	}

	// 1マスを元に戻す
	void Floor::Restore(const Cell& cell)
	{
		// CSV のとおりの最初の状態に戻す
		FloorTile& tile = At(cell);
		tile.remaining = -1.0f;
		tile.isSolid = tile.isSolidAtStart;
		tile.isIsolated = false;
		tile.isVisible = tile.isSolidAtStart;
		tile.isPreview = false;
		tile.fallTime = 0.0f;
		tile.position = GetRestPosition(cell);
		tile.rotate = { 0.0f, 0.0f, 0.0f };
		tile.scaleRate = 1.0f;
	}

	// 全部のマスを元に戻す
	void Floor::Reset()
	{
		// 切り離し・予告・床の数を戻す
		dropping_.clear();
		previewCells_.clear();
		solidTileCount_ = startSolidTileCount_;

		// 1マスずつ戻す
		for (int32_t z = 0; z < depth_; ++z)
		{
			for (int32_t x = 0; x < width_; ++x)
			{
				Restore({ x, z });
			}
		}
	}

	// =====================================================================
	// 崩す形の予告
	// =====================================================================
	// 崩す形の予告を出す (床があるマスだけ)
	void Floor::SetPreview(const std::vector<Cell>& cells, const Vector4& color, float blend)
	{
		// 前の予告を消して、色と混ぜる割合を覚える
		ClearPreview();
		previewColor_ = color;
		previewBlend_ = blend;
		for (const Cell& cell : cells)
		{
			if (!IsSolid(cell))
			{
				continue;
			}
			At(cell).isPreview = true;
			previewCells_.push_back(cell);
		}
	}

	// 予告を消す
	void Floor::ClearPreview()
	{
		for (const Cell& cell : previewCells_)
		{
			At(cell).isPreview = false;
		}
		previewCells_.clear();
	}

	// =====================================================================
	// 切り離し
	// =====================================================================
	bool Floor::TryIsolate(const Cell& bossCell)
	{
		// ボスの島が切り離されているか調べる
		std::vector<Cell> region;
		if (!FloorPathFinder::FindIsolatedRegion(*this, bossCell, region))
		{
			return false;
		}

		// ボスの島を「切り離された床」にする
		for (const Cell& cell : region)
		{
			At(cell).isIsolated = true;
			dropping_.push_back(cell);
		}
		return true;
	}

	// 切り離された床を揺らす
	void Floor::Shake(float elapsed, float amplitude, float frequency)
	{
		const float pi = std::numbers::pi_v<float>;
		for (const Cell& cell : dropping_)
		{
			// マスごとに少しずらして揺らす
			const float phase = elapsed * frequency * pi * 2.0f + static_cast<float>(cell.x + cell.z) * 0.35f;
			Vector3 position = GetRestPosition(cell);
			position.x += amplitude * std::sin(phase);
			At(cell).position = position;
		}
	}

	// 切り離された床を穴にする
	void Floor::Release()
	{
		for (const Cell& cell : dropping_)
		{
			if (IsSolid(cell))
			{
				OpenHole(cell);
			}
		}
	}

	// 切り離された床を下げる
	void Floor::AnimateDrop(float distance)
	{
		for (const Cell& cell : dropping_)
		{
			Vector3 position = GetRestPosition(cell);
			position.y -= distance;
			At(cell).position = position;
		}
	}

	// 穴になった床を全部消す
	void Floor::FinishDrop()
	{
		// 穴になった床 (切り離されて落ちた床・落ちている途中の床) を全部消す
		for (FloorTile& tile : tiles_)
		{
			if (!tile.isSolid)
			{
				tile.isVisible = false;
			}
		}
		dropping_.clear();
	}
}
