#include "Stage.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>

using namespace RyoEngine;

namespace App {
	// =====================================================================
	// 初期化
	// =====================================================================
	bool Stage::Initialize(const std::string& csvPath) {
		const bool isLoaded = LoadCsv(csvPath);
		if (!isLoaded) {
			Logger::LogWarning("[Stage] CSV が読めなかったので 10x10 の床にしました: {}", csvPath);
			CreateDefaultTiles();
		}

		CreateModels();

		ParamEditor::BeginGroup("Stage");
		ParamEditor::RegisterValue("消えるまでの時間", &collapseDelay_, 0.01f, 0.0f, 10.0f);
		ParamEditor::RegisterValue("すき間 (1マスに対する割合)", &tileGap_, 0.005f, 0.0f, 0.5f);
		ParamEditor::RegisterValue("タイルの厚さ", &tileThickness_, 0.01f, 0.01f, 5.0f);
		ParamEditor::RegisterColor("ふつうの色", &normalColor_);
		ParamEditor::RegisterColor("消える前の色", &warningColor_);
		ParamEditor::EndGroup();

		return isLoaded;
	}

	bool Stage::LoadCsv(const std::string& csvPath) {
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
		width_ = static_cast<int32_t>(maxWidth);
		depth_ = static_cast<int32_t>(rows.size());

		tiles_.assign(static_cast<size_t>(width_ * depth_), Tile{});
		for (int32_t z = 0; z < depth_; ++z) {
			// z = 0 (手前) は CSV の一番下の行
			const std::vector<int32_t>& row = rows[static_cast<size_t>(depth_ - 1 - z)];
			for (int32_t x = 0; x < width_; ++x) {
				bool isSolid = false;
				if (static_cast<size_t>(x) < row.size() && row[static_cast<size_t>(x)] == 1) {
					isSolid = true;
				}
				Tile& tile = tiles_[ToIndex({ x, z })];
				tile.isSolidAtStart = isSolid;
				tile.isSolid = isSolid;
				tile.remaining = -1.0f;
			}
		}
		return true;
	}

	void Stage::CreateDefaultTiles() {
		width_ = 10;
		depth_ = 10;
		Tile solidTile{};
		solidTile.isSolidAtStart = true;
		solidTile.isSolid = true;
		tiles_.assign(static_cast<size_t>(width_ * depth_), solidTile);
	}

	void Stage::CreateModels() {
		models_.clear();
		models_.resize(tiles_.size());

		for (size_t i = 0; i < tiles_.size(); ++i) {
			if (!tiles_[i].isSolidAtStart) {
				continue; // CSV で穴のマスは、モデルも作らない
			}
			std::unique_ptr<Model> model = Model::Create("cube"); // Kari/Model/cube.obj (仮の木箱。1辺が 2)
			model->SetColor(normalColor_);
			models_[i] = std::move(model);
		}
		ApplyTileShape();
	}

	void Stage::ApplyTileShape() {
		// タイルの見た目の大きさ (すき間の分だけ小さくする)
		const float visibleSize = tileSize_ * (1.0f - tileGap_);

		for (int32_t z = 0; z < depth_; ++z) {
			for (int32_t x = 0; x < width_; ++x) {
				const Cell cell = { x, z };
				Model* model = models_[ToIndex(cell)].get();
				if (model == nullptr) {
					continue;
				}
				// 上の面が height_ になるように、厚さの半分だけ下げる
				const Vector3 center = GetCellCenter(cell);
				// cube.obj は1辺が 2 (-1 〜 1) なので、半分にした値を大きさにする
				model->SetScale({ visibleSize * 0.5f, tileThickness_ * 0.5f, visibleSize * 0.5f });
				model->SetTranslate({ center.x, height_ - tileThickness_ * 0.5f, center.z });
			}
		}
	}

	// =====================================================================
	// 更新・描画
	// =====================================================================
	void Stage::Update() {
		const float deltaTime = GetDeltaTime();

		// すき間・厚さを ImGui で変えたら、すぐ見た目に出す
		ApplyTileShape();

		for (size_t i = 0; i < tiles_.size(); ++i) {
			Tile& tile = tiles_[i];
			if (!tile.isSolid) {
				continue;
			}

			// 踏まれていたら時間を進めて、0 になったら消す
			if (tile.remaining >= 0.0f) {
				tile.remaining -= deltaTime;
				if (tile.remaining <= 0.0f) {
					tile.isSolid = false;
					continue;
				}
			}

			Model* model = models_[i].get();
			if (model == nullptr) {
				continue;
			}
			if (tile.remaining >= 0.0f) {
				model->SetColor(warningColor_);
			} else {
				model->SetColor(normalColor_);
			}
			model->TransferMatrix();
		}
	}

	void Stage::Draw() {
		for (size_t i = 0; i < tiles_.size(); ++i) {
			if (!tiles_[i].isSolid) {
				continue;
			}
			if (models_[i] == nullptr) {
				continue;
			}
			models_[i]->Draw();
		}
	}

	void Stage::Reset() {
		for (Tile& tile : tiles_) {
			tile.isSolid = tile.isSolidAtStart;
			tile.remaining = -1.0f;
		}
	}

	// =====================================================================
	// マス
	// =====================================================================
	Cell Stage::GetCellAt(const Vector3& position) const {
		Cell cell{};
		cell.x = static_cast<int32_t>(std::floor(position.x / tileSize_ + static_cast<float>(width_) * 0.5f));
		cell.z = static_cast<int32_t>(std::floor(position.z / tileSize_ + static_cast<float>(depth_) * 0.5f));
		return cell;
	}

	Vector3 Stage::GetCellCenter(const Cell& cell) const {
		Vector3 center{};
		center.x = (static_cast<float>(cell.x) + 0.5f - static_cast<float>(width_) * 0.5f) * tileSize_;
		center.y = height_;
		center.z = (static_cast<float>(cell.z) + 0.5f - static_cast<float>(depth_) * 0.5f) * tileSize_;
		return center;
	}

	bool Stage::IsSolid(const Cell& cell) const {
		if (!Contains(cell)) {
			return false;
		}
		return tiles_[ToIndex(cell)].isSolid;
	}

	void Stage::StepOn(const Cell& cell) {
		if (!IsSolid(cell)) {
			return;
		}
		Tile& tile = tiles_[ToIndex(cell)];
		if (tile.remaining >= 0.0f) {
			return; // もう数え始めている (踏み直しても延びない)
		}
		tile.remaining = collapseDelay_;
	}

	Vector3 Stage::ClampToBoundary(const Vector3& position, float radius) const {
		const float margin = std::min(std::max(0.001f, radius), tileSize_ * 0.5f);
		const float limitX = static_cast<float>(width_) * tileSize_ * 0.5f - margin;
		const float limitZ = static_cast<float>(depth_) * tileSize_ * 0.5f - margin;

		Vector3 result = position;
		result.x = std::clamp(result.x, -limitX, limitX);
		result.z = std::clamp(result.z, -limitZ, limitZ);
		return result;
	}

	bool Stage::Contains(const Cell& cell) const {
		if (cell.x < 0 || cell.x >= width_) {
			return false;
		}
		if (cell.z < 0 || cell.z >= depth_) {
			return false;
		}
		return true;
	}

	size_t Stage::ToIndex(const Cell& cell) const {
		return static_cast<size_t>(cell.z * width_ + cell.x);
	}
}
