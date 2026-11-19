#include "Floor.h"
#include "../Game/GameConfig.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <numbers>
#include <sstream>

using namespace RyoEngine;

namespace App {
	namespace {
		// となりの4マス (奥・右・手前・左。Unity と同じ順番)
		constexpr Cell kDirections[4] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };

		// 長さの2乗 (遠い・近いを比べるだけなので sqrt しない)
		float LengthSquared(const Vector3& v) {
			return v.x * v.x + v.y * v.y + v.z * v.z;
		}

		// current から target へ、最大 maxDistance だけ近づける
		float MoveTowardsFloat(float current, float target, float maxDistance) {
			if (std::abs(target - current) <= maxDistance) {
				return target;
			}
			if (target > current) {
				return current + maxDistance;
			}
			return current - maxDistance;
		}

		constexpr float kTileFallSpeed = 12.0f; // 穴になったタイルが落ちる速さ (距離 = 12 x 時間の2乗)
		constexpr float kTileFallTime = 0.5f;   // 穴になったタイルが消えるまで(秒)
	}

	// =====================================================================
	// 初期化
	// =====================================================================
	void Floor::Initialize(const std::string& csvPath, float height, bool isForeground) {
		// 高さと、今いる層か (今いる層でなければ暗く出す)
		height_ = height;
		isForeground_ = isForeground;

		// CSV から床の形を読む。読めなければ 10x10
		if (!LoadCsv(csvPath)) {
			Logger::LogWarning("[Floor] CSV が読めなかったので 10x10 の床にしました: {}", csvPath);
			CreateDefaultTiles();
		}

		// 床全体の大きさはいつも同じ。マスの数で割って1マスの大きさにする
		tileSizeX_ = GameConfig::kFloorSize / static_cast<float>(width_);
		tileSizeZ_ = GameConfig::kFloorSize / static_cast<float>(depth_);

		// 最初にある床の数を数える (崩れた割合を出すのに使う)
		startSolidTileCount_ = 0;
		for (const Tile& tile : tiles_) {
			if (tile.isSolidAtStart) {
				++startSolidTileCount_;
			}
		}

		// モデルを作って、全部のマスを最初の状態にする
		CreateModels();
		Reset();
	}

	// CSV を読んで、マスの数と床の形を決める
	bool Floor::LoadCsv(const std::string& csvPath) {
		std::ifstream file(csvPath);
		if (!file.is_open()) {
			return false;
		}

		// 1行ずつ読む (CSV の上の行が奥、下の行が手前)
		std::vector<std::vector<int32_t>> rows;
		std::string line;
		while (std::getline(file, line)) {
			// Excel などで付く先頭の BOM と、行末の \r を取り除く
			if (line.size() >= 3 &&
				static_cast<unsigned char>(line[0]) == 0xEF &&
				static_cast<unsigned char>(line[1]) == 0xBB &&
				static_cast<unsigned char>(line[2]) == 0xBF) {
				line.erase(0, 3);
			}
			if (!line.empty() && line.back() == '\r') {
				line.pop_back();
			}
			if (line.empty()) {
				continue;
			}

			// カンマで区切って数字にする
			std::vector<int32_t> row;
			std::istringstream stream(line);
			std::string value;
			while (std::getline(stream, value, ',')) {
				row.push_back(std::atoi(value.c_str())); // 空欄・数字でないものは 0 (穴)
			}
			rows.push_back(row);
		}
		if (rows.empty()) {
			return false;
		}

		// 一番長い行に合わせる (足りない所は穴)
		size_t maxWidth = 0;
		for (const std::vector<int32_t>& row : rows) {
			maxWidth = std::max(maxWidth, row.size());
		}
		if (maxWidth == 0) {
			return false;
		}
		width_ = static_cast<int32_t>(maxWidth);
		depth_ = static_cast<int32_t>(rows.size());

		// 読んだ数字をタイルに入れる (1 が床、それ以外は穴)
		tiles_.assign(static_cast<size_t>(width_ * depth_), Tile{});
		for (int32_t z = 0; z < depth_; ++z) {
			// z = 0 (手前) は CSV の一番下の行
			const std::vector<int32_t>& row = rows[static_cast<size_t>(depth_ - 1 - z)];
			for (int32_t x = 0; x < width_; ++x) {
				if (static_cast<size_t>(x) < row.size() && row[static_cast<size_t>(x)] == 1) {
					tiles_[ToIndex({ x, z })].isSolidAtStart = true;
				}
			}
		}
		return true;
	}

	// CSV が読めなかったときの 10x10 の床
	void Floor::CreateDefaultTiles() {
		width_ = 10;
		depth_ = 10;
		Tile solidTile{};
		solidTile.isSolidAtStart = true;
		tiles_.assign(static_cast<size_t>(width_ * depth_), solidTile);
	}

	// 床のマスにモデルを作る
	void Floor::CreateModels() {
		models_.clear();
		models_.resize(tiles_.size());
		for (size_t i = 0; i < tiles_.size(); ++i) {
			if (!tiles_[i].isSolidAtStart) {
				continue; // CSV で穴のマスは、モデルも作らない
			}
			models_[i] = Model::Create(GameConfig::kCubeModel);
		}
	}

	// =====================================================================
	// 見た目
	// =====================================================================
	void Floor::UpdateVisuals() {
		// 予告のマスの光の強さ: 弱い ⇔ 強い をゆっくりくり返す
		previewGlowTime_ += GetDeltaTime();
		const float wave = 0.5f + 0.5f * std::sin(previewGlowTime_ * GameConfig::kPreviewGlowSpeed * std::numbers::pi_v<float> * 2.0f);
		const float glow = GameConfig::kPreviewGlowMin + (GameConfig::kPreviewGlowMax - GameConfig::kPreviewGlowMin) * wave;
		const Vector3 glowColor = { previewColor_.x, previewColor_.y, previewColor_.z };

		// cube.obj の大きさに合わせて、タイルの大きさにする (すき間の分だけ小さく)
		const float modelScale = 1.0f / GameConfig::kCubeModelSize;
		const float sizeX = tileSizeX_ * (1.0f - GameConfig::kTileGap) * modelScale;
		const float sizeY = GameConfig::kTileThickness * modelScale;
		const float sizeZ = tileSizeZ_ * (1.0f - GameConfig::kTileGap) * modelScale;

		// タイルごとに 位置・向き・大きさ・色 を入れる
		for (size_t i = 0; i < tiles_.size(); ++i) {
			Model* model = models_[i].get();
			if (model == nullptr) {
				continue;
			}
			const Tile& tile = tiles_[i];
			model->SetTranslate(tile.position);
			model->SetRotate(tile.rotate);
			model->SetScale({ sizeX * tile.scaleRate, sizeY * tile.scaleRate, sizeZ * tile.scaleRate });
			model->SetColor(GetTileColor(ToCell(i)));

			// 崩す形の予告のマスは、形の色で光らせる (エンジンの発光。ブルームでにじむ)
			if (IsPreviewGlowing(ToCell(i))) {
				model->SetEmissive(glowColor, glow);
			} else {
				model->SetEmissive({ 0.0f, 0.0f, 0.0f }, 0.0f);
			}
			model->TransferMatrix();
		}
	}

	// 見えているタイルを描く
	void Floor::Draw() {
		for (size_t i = 0; i < tiles_.size(); ++i) {
			if (!tiles_[i].isVisible || models_[i] == nullptr) {
				continue;
			}
			models_[i]->Draw();
		}
	}

	// いつもの色 (切り離し・穴のとなり・ふつう)
	Vector4 Floor::GetBaseColor(const Cell& cell) const {
		// 切り離された床 → 穴のとなり (飛ばすと 4 倍) → ふつう
		if (At(cell).isIsolated) {
			return GameConfig::kIsolatedColor;
		}
		if (GetBarrageMultiplier(cell) == 4) {
			return GameConfig::kBonusColor;
		}
		return GameConfig::kFloorColor;
	}

	// 予告で光らせるマスか
	bool Floor::IsPreviewGlowing(const Cell& cell) const {
		const Tile& tile = At(cell);
		// 今いる層の、予告を出している床だけ (崩す予定・切り離された床は光らせない)
		if (!isForeground_ || !tile.isPreview || !tile.isSolid) {
			return false;
		}
		if (tile.isIsolated || tile.remaining >= 0.0f) {
			return false;
		}
		return true;
	}

	// 今の色 (下の層・崩れる前・予告 も入れる)
	Vector4 Floor::GetTileColor(const Cell& cell) const {
		const Tile& tile = At(cell);
		const Vector4 baseColor = GetBaseColor(cell);

		// 今いる層でなければ暗くする
		if (!isForeground_) {
			const float brightness = GameConfig::kLowerFloorBrightness;
			return Vector4{ baseColor.x * brightness, baseColor.y * brightness, baseColor.z * brightness, baseColor.w };
		}

		// 崩れる予定: ふつうはオレンジ → 最後の少しは点滅 → 最後は赤
		if (!tile.isIsolated && tile.remaining >= 0.0f) {
			if (tile.remaining <= GameConfig::kCollapseRedDuration) {
				return GameConfig::kCollapseRedColor;
			}
			if (tile.remaining <= GameConfig::kCollapseWarningDuration) {
				const float phase = (GameConfig::kCollapseWarningDuration - tile.remaining) * GameConfig::kCollapseBlinkFrequency;
				if (phase - std::floor(phase) < 0.5f) {
					return GameConfig::kCollapseBlinkColor;
				}
			}
			return GameConfig::kSteppedTileColor;
		}

		// 崩す形の予告: いつもの色に予告の色を混ぜる
		if (!tile.isIsolated && tile.isPreview) {
			const float t = previewBlend_;
			return Vector4{
				baseColor.x + (previewColor_.x - baseColor.x) * t,
				baseColor.y + (previewColor_.y - baseColor.y) * t,
				baseColor.z + (previewColor_.z - baseColor.z) * t,
				baseColor.w + (previewColor_.w - baseColor.w) * t,
			};
		}
		return baseColor;
	}

	// =====================================================================
	// 形・マス
	// =====================================================================
	// 床がどれだけ崩れたか (0 = 全部ある / 1 = 全部ない)
	float Floor::GetCollapsedFraction() const {
		if (startSolidTileCount_ <= 0) {
			return 1.0f;
		}
		return 1.0f - static_cast<float>(solidTileCount_) / static_cast<float>(startSolidTileCount_);
	}

	// 位置 → マス
	Cell Floor::GetCellAt(const Vector3& position) const {
		// 床の真ん中が 0 なので、マスの数の半分ずらして数える
		Cell cell{};
		cell.x = static_cast<int32_t>(std::floor(position.x / tileSizeX_ + static_cast<float>(width_) * 0.5f));
		cell.z = static_cast<int32_t>(std::floor(position.z / tileSizeZ_ + static_cast<float>(depth_) * 0.5f));
		return cell;
	}

	// マス → マスの真ん中の位置
	Vector3 Floor::GetCellCenter(const Cell& cell) const {
		Vector3 center{};
		center.x = (static_cast<float>(cell.x) + 0.5f - static_cast<float>(width_) * 0.5f) * tileSizeX_;
		center.y = height_;
		center.z = (static_cast<float>(cell.z) + 0.5f - static_cast<float>(depth_) * 0.5f) * tileSizeZ_;
		return center;
	}

	// タイルのいつもの位置 (タイルの真ん中)
	Vector3 Floor::GetRestPosition(const Cell& cell) const {
		Vector3 position = GetCellCenter(cell);
		position.y -= GameConfig::kTileThickness * 0.5f;
		return position;
	}

	// 床の外に出ないようにする
	Vector3 Floor::ClampToBoundary(const Vector3& position, float radius) const {
		const float marginX = std::min(std::max(0.001f, radius), tileSizeX_ * 0.5f);
		const float marginZ = std::min(std::max(0.001f, radius), tileSizeZ_ * 0.5f);
		const float limitX = static_cast<float>(width_) * tileSizeX_ * 0.5f - marginX;
		const float limitZ = static_cast<float>(depth_) * tileSizeZ_ * 0.5f - marginZ;

		// 範囲の中におさめる
		Vector3 result = position;
		result.x = std::clamp(result.x, -limitX, limitX);
		result.z = std::clamp(result.z, -limitZ, limitZ);
		return result;
	}

	// 床の範囲の中か
	bool Floor::Contains(const Cell& cell) const {
		if (cell.x < 0 || cell.x >= width_) {
			return false;
		}
		if (cell.z < 0 || cell.z >= depth_) {
			return false;
		}
		return true;
	}

	// 床があるか
	bool Floor::IsSolid(const Cell& cell) const {
		if (!Contains(cell)) {
			return false;
		}
		return At(cell).isSolid;
	}

	// 穴か
	bool Floor::IsHole(const Cell& cell) const {
		if (!Contains(cell)) {
			return false;
		}
		return !At(cell).isSolid;
	}

	// 切り離された床か
	bool Floor::IsIsolated(const Cell& cell) const {
		if (!Contains(cell)) {
			return false;
		}
		return At(cell).isIsolated;
	}

	// 穴のとなりなら 4 倍
	int32_t Floor::GetBarrageMultiplier(const Cell& cell) const {
		for (const Cell& direction : kDirections) {
			if (IsHole(cell + direction)) {
				return 4;
			}
		}
		return 1;
	}

	// マス → 配列の番号
	size_t Floor::ToIndex(const Cell& cell) const {
		return static_cast<size_t>(cell.z * width_ + cell.x);
	}

	// 配列の番号 → マス
	Cell Floor::ToCell(size_t index) const {
		const int32_t value = static_cast<int32_t>(index);
		return Cell{ value % width_, value / width_ };
	}

	// =====================================================================
	// マスからマスへ跳ぶ
	// =====================================================================
	// direction へ跳んだときに着くマス (穴が1つなら飛び越える)
	Cell Floor::JumpLanding(const Cell& from, const Cell& direction) const {
		const Cell first = from + direction;
		const Cell second = first + direction;
		// となりが穴で、その先が床なら飛び越える
		if (IsHole(first) && IsSolid(second)) {
			return second;
		}
		return first;
	}

	// マスの向き → 1マス分のずれ
	Vector3 Floor::StepOffset(const Cell& direction) const {
		return Vector3{ static_cast<float>(direction.x) * tileSizeX_, 0.0f, static_cast<float>(direction.z) * tileSizeZ_ };
	}

	// マスの線にそって進む (先に x、次に z)
	Vector3 Floor::AlongGrid(const Vector3& start, const Vector3& end, float progress) {
		if (progress >= 1.0f) {
			return end;
		}
		// 進んだ距離 (x と z の合計の長さのうち、progress の分)。先に x を進めて、残りで z を進める
		const float distanceX = std::abs(end.x - start.x);
		const float distanceZ = std::abs(end.z - start.z);
		const float travel = (distanceX + distanceZ) * progress;
		Vector3 result{};
		result.x = MoveTowardsFloat(start.x, end.x, travel);
		result.y = start.y + (end.y - start.y) * progress;
		result.z = MoveTowardsFloat(start.z, end.z, std::max(0.0f, travel - distanceX));
		return result;
	}

	// =====================================================================
	// 床を崩す
	// =====================================================================
	// 1マスを delay 秒後に崩す予定にする (できたら true)
	bool Floor::ScheduleCollapse(const Cell& cell, float delay) {
		if (!IsSolid(cell)) {
			return false;
		}
		Tile& tile = At(cell);
		if (tile.remaining >= 0.0f || tile.isIsolated) {
			return false; // もう崩す予定・切り離された床
		}
		tile.remaining = delay;
		return true;
	}

	// いくつかのマスをまとめて崩す
	int32_t Floor::ScheduleCellsCollapse(const std::vector<Cell>& cells, float delay) {
		int32_t count = 0;
		for (const Cell& cell : cells) {
			if (ScheduleCollapse(cell, delay)) {
				++count;
			}
		}
		return count;
	}

	// 崩れる時間を進める・穴になったタイルを落とす
	bool Floor::Tick(float deltaTime) {
		bool isChanged = false;
		for (int32_t z = 0; z < depth_; ++z) {
			for (int32_t x = 0; x < width_; ++x) {
				const Cell cell = { x, z };
				Tile& tile = At(cell);
				if (tile.isIsolated) {
					continue; // 切り離された床は、揺れ・落下を別に動かす
				}

				// 穴になったタイル: 下へ落ちていって消える
				if (!tile.isSolid) {
					if (!tile.isVisible) {
						continue;
					}
					tile.fallTime += deltaTime;
					Vector3 position = GetRestPosition(cell);
					position.y -= kTileFallSpeed * tile.fallTime * tile.fallTime;
					tile.position = position;
					if (tile.fallTime >= kTileFallTime) {
						tile.isVisible = false;
					}
					continue;
				}

				// 崩れる予定のタイル: 時間が来たら穴にする
				if (tile.remaining < 0.0f) {
					continue;
				}
				tile.remaining -= deltaTime;
				if (tile.remaining > 0.0f) {
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
	void Floor::OpenHole(const Cell& cell) {
		At(cell).isSolid = false;
		--solidTileCount_;
	}

	// 1マスを元に戻す
	void Floor::Restore(const Cell& cell) {
		Tile& tile = At(cell);
		// CSV のとおりの最初の状態に戻す
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
	void Floor::Reset() {
		// 切り離し・予告・床の数を戻す
		dropping_.clear();
		previewCells_.clear();
		solidTileCount_ = startSolidTileCount_;
		// 1マスずつ戻す
		for (int32_t z = 0; z < depth_; ++z) {
			for (int32_t x = 0; x < width_; ++x) {
				Restore({ x, z });
			}
		}
	}

	// =====================================================================
	// 崩す形の予告
	// =====================================================================
	// 崩す形の予告を出す (床があるマスだけ)
	void Floor::SetPreview(const std::vector<Cell>& cells, const Vector4& color, float blend) {
		// 前の予告を消して、色と混ぜる割合を覚える
		ClearPreview();
		previewColor_ = color;
		previewBlend_ = blend;
		for (const Cell& cell : cells) {
			if (!IsSolid(cell)) {
				continue;
			}
			At(cell).isPreview = true;
			previewCells_.push_back(cell);
		}
	}

	// 予告を消す
	void Floor::ClearPreview() {
		for (const Cell& cell : previewCells_) {
			At(cell).isPreview = false;
		}
		previewCells_.clear();
	}

	// =====================================================================
	// 道さがし
	// =====================================================================
	// start から行けるマスを全部調べる (parents: 1つ前のマス / order: 見つけた順)
	void Floor::Explore(const Cell& start, bool canJump, std::vector<int32_t>& parents, std::vector<Cell>& order) const {
		parents.assign(tiles_.size(), -1);
		order.clear();
		if (!IsSolid(start)) {
			return;
		}

		// 幅優先: order を待ち行列として使う (先頭から順に調べる)
		parents[ToIndex(start)] = static_cast<int32_t>(ToIndex(start));
		order.push_back(start);
		for (size_t head = 0; head < order.size(); ++head) {
			const Cell current = order[head];
			for (const Cell& direction : kDirections) {
				// 飛び越えられるなら、穴を1つ飛ばした先も行ける
				Cell next = current + direction;
				if (canJump) {
					next = JumpLanding(current, direction);
				}
				if (!IsSolid(next)) {
					continue;
				}
				const size_t nextIndex = ToIndex(next);
				if (parents[nextIndex] >= 0) {
					continue;
				}
				parents[nextIndex] = static_cast<int32_t>(ToIndex(current));
				order.push_back(next);
			}
		}
	}

	// ボスの次のマス (行ける所でプレイヤーに一番近い所へ)
	Cell Floor::NextStep(const Cell& start, const Vector3& target, bool canJump) const {
		// start から行けるマスを全部調べる
		std::vector<int32_t> parents;
		std::vector<Cell> order;
		Explore(start, canJump, parents, order);

		// 行き先: プレイヤーのマスに行けるならそこ。行けなければ、行ける所で一番近いマス
		Cell best = start;
		const Cell targetCell = GetCellAt(target);
		if (Contains(targetCell) && parents[ToIndex(targetCell)] >= 0) {
			best = targetCell;
		} else {
			float bestDistance = std::numeric_limits<float>::infinity();
			for (const Cell& cell : order) {
				const Vector3 center = GetCellCenter(cell);
				const float distance = LengthSquared(Vector3{ center.x - target.x, center.y - target.y, center.z - target.z });
				if (distance < bestDistance) {
					bestDistance = distance;
					best = cell;
				}
			}
		}

		// 行き先から start までさかのぼって、start の次のマスを返す
		if (order.empty()) {
			return best;
		}
		const size_t startIndex = ToIndex(start);
		while (best != start && static_cast<size_t>(parents[ToIndex(best)]) != startIndex) {
			best = ToCell(static_cast<size_t>(parents[ToIndex(best)]));
		}
		return best;
	}

	// ボスの逃げ先 (上下左右で、すぐには崩れない床)
	bool Floor::TryFindLandingNeighbor(const Vector3& position, float timeToLand, Cell& landing) const {
		const Cell current = GetCellAt(position);
		landing = current;
		float bestDistance = std::numeric_limits<float>::infinity();

		// 上下左右のうち、一番近い床を選ぶ
		for (const Cell& direction : kDirections) {
			const Cell cell = current + direction;
			if (!IsSolid(cell)) {
				continue;
			}
			// 着く前に崩れてしまう床はだめ
			const float remaining = At(cell).remaining;
			if (remaining >= 0.0f && remaining <= timeToLand) {
				continue;
			}
			const Vector3 center = GetCellCenter(cell);
			const float distance = LengthSquared(Vector3{ center.x - position.x, center.y - position.y, center.z - position.z });
			if (distance < bestDistance) {
				bestDistance = distance;
				landing = cell;
			}
		}
		return landing != current;
	}

	// =====================================================================
	// 切り離し
	// =====================================================================
	// ボスの島が、ほかの島より小さければ切り離す (できたら true)
	bool Floor::TryIsolate(const Cell& bossCell) {
		// ボスのいる島を調べる
		std::vector<int32_t> bossParents;
		std::vector<Cell> bossRegion;
		Explore(bossCell, false, bossParents, bossRegion);
		if (bossRegion.empty()) {
			return false;
		}

		// ボスの島と同じか、それより大きい島がほかにあるか
		std::vector<bool> isVisited(tiles_.size(), false);
		for (const Cell& cell : bossRegion) {
			isVisited[ToIndex(cell)] = true;
		}
		bool isFound = false;
		std::vector<int32_t> otherParents;
		std::vector<Cell> otherRegion;
		for (int32_t z = 0; z < depth_ && !isFound; ++z) {
			for (int32_t x = 0; x < width_ && !isFound; ++x) {
				const Cell cell = { x, z };
				if (!IsSolid(cell) || isVisited[ToIndex(cell)]) {
					continue;
				}
				Explore(cell, false, otherParents, otherRegion);
				if (otherRegion.size() >= bossRegion.size()) {
					isFound = true;
				} else {
					for (const Cell& other : otherRegion) {
						isVisited[ToIndex(other)] = true;
					}
				}
			}
		}
		if (!isFound) {
			return false;
		}

		// ボスの島を「切り離された床」にする
		for (const Cell& cell : bossRegion) {
			At(cell).isIsolated = true;
			dropping_.push_back(cell);
		}
		return true;
	}

	// ボスからプレイヤーまでたどり着けるか
	bool Floor::CanReunite(const Cell& playerCell, const Cell& bossCell, bool canCrossGaps) const {
		// ボスのマスから、プレイヤーのマスまでたどり着けるか
		std::vector<int32_t> parents;
		std::vector<Cell> order;
		Explore(bossCell, canCrossGaps, parents, order);
		if (!Contains(playerCell)) {
			return false;
		}
		return parents[ToIndex(playerCell)] >= 0;
	}

	// 切り離された床を揺らす
	void Floor::Shake(float elapsed, float amplitude, float frequency) {
		const float pi = std::numbers::pi_v<float>;
		for (const Cell& cell : dropping_) {
			// マスごとに少しずらして揺らす
			const float phase = elapsed * frequency * pi * 2.0f + static_cast<float>(cell.x + cell.z) * 0.35f;
			Vector3 position = GetRestPosition(cell);
			position.x += amplitude * std::sin(phase);
			At(cell).position = position;
		}
	}

	// 切り離された床を穴にする
	void Floor::Release() {
		for (const Cell& cell : dropping_) {
			if (IsSolid(cell)) {
				OpenHole(cell);
			}
		}
	}

	// 切り離された床を下げる
	void Floor::AnimateDrop(float distance) {
		for (const Cell& cell : dropping_) {
			Vector3 position = GetRestPosition(cell);
			position.y -= distance;
			At(cell).position = position;
		}
	}

	// 穴になった床を全部消す
	void Floor::FinishDrop() {
		// 穴になった床 (切り離されて落ちた床・落ちている途中の床) を全部消す
		for (Tile& tile : tiles_) {
			if (!tile.isSolid) {
				tile.isVisible = false;
			}
		}
		dropping_.clear();
	}

	// =====================================================================
	// 残った足場をボスへ飛ばす
	// =====================================================================
	int32_t Floor::PrepareBarrage() {
		// 残っている床を全部、飛ばす足場にする
		barrage_.clear();
		for (int32_t z = 0; z < depth_; ++z) {
			for (int32_t x = 0; x < width_; ++x) {
				const Cell cell = { x, z };
				if (IsSolid(cell)) {
					barrage_.push_back(BarrageTile{ cell, GetBarrageMultiplier(cell) });
				}
			}
		}
		return static_cast<int32_t>(barrage_.size());
	}

	// 足場を1枚ずつボスへ飛ばす
	int32_t Floor::AnimateBarrage(const Vector3& target, float elapsed, float flightDuration, float launchWindow, int64_t& damageUnits) {
		int32_t arrived = 0;
		damageUnits = 0;
		const int32_t count = static_cast<int32_t>(barrage_.size());

		for (int32_t i = 0; i < count; ++i) {
			// 1枚ずつ、少しずつ遅れて飛び立つ
			const float launchTime = launchWindow * static_cast<float>(i) / static_cast<float>(std::max(1, count - 1));
			const float t = std::clamp((elapsed - launchTime) / flightDuration, 0.0f, 1.0f);
			const BarrageTile& barrageTile = barrage_[static_cast<size_t>(i)];
			Tile& tile = At(barrageTile.cell);

			if (t >= 1.0f) {
				// 届いた
				++arrived;
				damageUnits += barrageTile.multiplier;
				tile.isVisible = false;
				continue;
			}
			if (t <= 0.0f) {
				continue;
			}

			// 山なりに飛んで、小さくなりながら回る
			Vector3 position = Lerp(GetRestPosition(barrageTile.cell), target, t * t);
			position.y += 4.0f * t * (1.0f - t);
			tile.position = position;
			tile.scaleRate = 1.0f + (0.25f - 1.0f) * t;
			float spin = 1.0f;
			if (i % 2 != 0) {
				spin = -1.0f;
			}
			tile.rotate = { 180.0f * t, 90.0f * t * spin, 0.0f };
		}
		return arrived;
	}
}
