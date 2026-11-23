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
		//奥、右、手前、左のとなりの4マス
		constexpr Cell kDirections[4] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };

		constexpr float kTileFallSpeed = 12.0f; //穴になったタイルが落ちる速さ
		constexpr float kTileFallTime = 0.5f; //穴になったタイルが消えるまでの秒数
	}

	void Floor::Initialize(const std::string& csvPath, float height, bool isForeground)
	{
		//今いる層でなければ暗く出す
		isForeground_ = isForeground;

		//CSVから床の形を読み、読めなければ10x10
		FloorLayout layout{};
		if (!FloorCsv::Load(csvPath, layout))
		{
			Logger::LogWarning("[Floor] CSV が読めなかったので 10x10 の床にしました: {}", csvPath);
			layout = FloorCsv::CreateDefault(10, 10);
		}
		//マスの数と高さを決める
		SetupGrid(layout.width, layout.depth, height);

		//タイルを作って、崩れた割合に使う最初の床の数を数える
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

		//モデルを作って全部のマスを最初の状態にする
		view_.CreateModels(*this);
		Reset();
	}

	void Floor::UpdateVisuals()
	{
		view_.Update(*this);
	}

	//見えているタイルを描く
	void Floor::Draw()
	{
		view_.Draw(*this);
	}

	//0が全部あって1が全部ない床の崩れた割合
	float Floor::GetCollapsedFraction() const
	{
		if (startSolidTileCount_ <= 0)
		{
			return 1.0f;
		}
		return 1.0f - static_cast<float>(solidTileCount_) / static_cast<float>(startSolidTileCount_);
	}

	//範囲の外はfalseの床があるか
	bool Floor::IsSolid(const Cell& cell) const
	{
		return Contains(cell) && GetTile(cell).isSolid;
	}

	//穴か
	bool Floor::IsHole(const Cell& cell) const
	{
		return Contains(cell) && !GetTile(cell).isSolid;
	}

	//切り離された床か
	bool Floor::IsIsolated(const Cell& cell) const
	{
		return Contains(cell) && GetTile(cell).isIsolated;
	}

	//崩れる予定か
	bool Floor::IsCollapseScheduled(const Cell& cell) const
	{
		return Contains(cell) && GetTile(cell).remaining >= 0.0f;
	}

	//穴のとなりなら4倍
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

	//タイルの位置と回転と大きさを変える
	void Floor::SetTileMotion(const Cell& cell, const Vector3& position, const Vector3& rotate, float scaleRate)
	{
		FloorTile& tile = At(cell);
		tile.position = position;
		tile.rotate = rotate;
		tile.scaleRate = scaleRate;
	}

	//タイルを見えなくする
	void Floor::HideTile(const Cell& cell)
	{
		At(cell).isVisible = false;
	}

	//穴をmaxGap個まで飛び越えてdirectionへ跳んだときに着くマス
	Cell Floor::JumpLanding(const Cell& from, const Cell& direction, int32_t maxGap) const
	{
		//穴が続く間はmaxGap個まで先へ進む
		Cell next = from + direction;
		int32_t holeCount = 0;
		while (holeCount < maxGap && IsHole(next))
		{
			next = next + direction;
			++holeCount;
		}
		//穴を越えた先が床ならそこに着地する
		if (holeCount > 0 && IsSolid(next))
		{
			return next;
		}
		return from + direction;
	}

	//1マスをdelay秒後に崩す予定にできたらtrue
	bool Floor::ScheduleCollapse(const Cell& cell, float delay, float heartAngle)
	{
		if (!IsSolid(cell))
		{
			return false;
		}
		FloorTile& tile = At(cell);
		if (tile.remaining >= 0.0f || tile.isIsolated)
		{
			return false; //もう崩す予定か切り離された床
		}
		tile.remaining = delay;
		tile.heartAngle = heartAngle;
		return true;
	}

	//いくつかのマスをまとめて崩す
	int32_t Floor::ScheduleCellsCollapse(const std::vector<Cell>& cells, float delay, float heartAngle)
	{
		int32_t count = 0;
		for (const Cell& cell : cells)
		{
			if (ScheduleCollapse(cell, delay, heartAngle))
			{
				++count;
			}
		}
		return count;
	}

	//崩れる時間を進めて穴になったタイルを落とす
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
					continue; //切り離された床は揺れと落下を別に動かす
				}

				//穴になったタイルは下へ落ちて消える
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
					tile.dropRate = tile.fallTime / kTileFallTime;
					if (tile.fallTime >= kTileFallTime)
					{
						tile.isVisible = false;
					}
					continue;
				}

				//崩れる予定のタイルは時間が来たら穴にする
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

	//穴にする
	void Floor::OpenHole(const Cell& cell)
	{
		At(cell).isSolid = false;
		--solidTileCount_;
	}

	//1マスを元に戻す
	void Floor::Restore(const Cell& cell)
	{
		//CSVのとおりの最初の状態に戻す
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
		tile.shakeRate = 0.0f;
		tile.dropRate = 0.0f;
		tile.heartAngle = 0.0f;
	}

	//全部のマスを元に戻す
	void Floor::Reset()
	{
		//切り離しと予告と床の数を戻す
		dropping_.clear();
		previewCells_.clear();
		solidTileCount_ = startSolidTileCount_;

		//1マスずつ戻す
		for (int32_t z = 0; z < depth_; ++z)
		{
			for (int32_t x = 0; x < width_; ++x)
			{
				Restore({ x, z });
			}
		}
	}

	//床があるマスだけ崩す形の予告を出す
	void Floor::SetPreview(const std::vector<Cell>& cells, const Vector4& color, float blend)
	{
		//前の予告を消して色と混ぜる割合を覚える
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

	//予告を消す
	void Floor::ClearPreview()
	{
		for (const Cell& cell : previewCells_)
		{
			At(cell).isPreview = false;
		}
		previewCells_.clear();
	}

	bool Floor::TryIsolate(const Cell& bossCell)
	{
		//ボスの島が切り離されているか調べる
		std::vector<Cell> region;
		if (!FloorPathFinder::FindIsolatedRegion(*this, bossCell, region))
		{
			return false;
		}

		//ボスの島を切り離された床にする
		for (const Cell& cell : region)
		{
			At(cell).isIsolated = true;
			dropping_.push_back(cell);
		}
		return true;
	}

	//切り離された床を揺らす
	void Floor::Shake(float elapsed, float amplitude, float frequency)
	{
		const float pi = std::numbers::pi_v<float>;
		for (const Cell& cell : dropping_)
		{
			//マスごとに少しずらして揺らす
			const float phase = elapsed * frequency * pi * 2.0f + static_cast<float>(cell.x + cell.z) * 0.35f;
			Vector3 position = GetRestPosition(cell);
			position.x += amplitude * std::sin(phase);
			At(cell).position = position;
			At(cell).shakeRate = elapsed / GameConfig::kEscapeDelay;
		}
	}

	//切り離された床を穴にする
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

	//切り離された床を下げる
	void Floor::AnimateDrop(float distance)
	{
		for (const Cell& cell : dropping_)
		{
			Vector3 position = GetRestPosition(cell);
			position.y -= distance;
			At(cell).position = position;
			At(cell).dropRate = distance / GameConfig::kFloorSpacing;
		}
	}

	//穴になった床を全部消す
	void Floor::FinishDrop()
	{
		//切り離されて落ちた床と落ちている途中の床を全部消す
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
