#include "LayerFloors.h"
#include "GameConfig.h"
#include <algorithm>
#include <cmath>
#include <string>

using namespace RyoEngine;

namespace App
{
	// layer 層目から始める
	void LayerFloors::Start(int32_t layer)
	{
		layer_ = layer;
		current_ = CreateFloor(layer_, true);
		below_.reset();
		upper_.reset();
	}

	// 下の層がまだなければ作る
	void LayerFloors::EnsureBelow()
	{
		if (below_ == nullptr)
		{
			below_ = CreateFloor(layer_ + 1, false);
		}
	}

	// 下の層へ進む
	void LayerFloors::AdvanceLayer()
	{
		EnsureBelow();

		// 今の層は上の層に (足場を飛ばすのに使う)、下の層を今の層にして明るくする
		current_->ClearPreview();
		upper_ = std::move(current_);
		current_ = std::move(below_);
		current_->ShowAsCurrentFloor();

		// さらに下の層を作る
		++layer_;
		below_ = CreateFloor(layer_ + 1, false);
	}

	// 上の層を消す
	void LayerFloors::ReleaseUpper()
	{
		upper_.reset();
	}

	// 3つの床の見た目を確定する
	void LayerFloors::UpdateVisuals()
	{
		if (current_ != nullptr)
		{
			current_->UpdateVisuals();
		}
		if (below_ != nullptr)
		{
			below_->UpdateVisuals();
		}
		if (upper_ != nullptr)
		{
			upper_->UpdateVisuals();
		}
	}

	// 下の層 → 今の層 → 上の層 の順に描く
	void LayerFloors::Draw()
	{
		if (below_ != nullptr)
		{
			below_->Draw();
		}
		if (current_ != nullptr)
		{
			current_->Draw();
		}
		if (upper_ != nullptr)
		{
			upper_->Draw();
		}
	}

	// index 層目の床を作る
	std::unique_ptr<Floor> LayerFloors::CreateFloor(int32_t index, bool isForeground) const
	{
		// layer1.csv 〜 layer5.csv を順に使う (6層目からは1層目に戻る)
		const int32_t fileNumber = index % GameConfig::kLayerFileCount + 1;
		const std::string path = "Resources/ApplicationResources/Stage/layer" + std::to_string(fileNumber) + ".csv";
		const float height = -static_cast<float>(index) * GameConfig::kFloorSpacing;

		// 床を作って読み込む
		std::unique_ptr<Floor> floor = std::make_unique<Floor>();
		floor->Initialize(path, height, isForeground);
		return floor;
	}

	// 1層目のマスで決めた最初の位置を、この層のマスに直す
	Vector3 LayerFloors::GetSpawnPosition(const Floor& floor, int32_t firstLayerX, int32_t firstLayerZ)
	{
		// 1層目のマスで指定した場所を、この層のマスの数に合わせる
		int32_t x = static_cast<int32_t>(std::floor((static_cast<float>(firstLayerX) + 0.5f) * static_cast<float>(floor.GetWidth()) / static_cast<float>(GameConfig::kFirstLayerWidth)));
		int32_t z = static_cast<int32_t>(std::floor((static_cast<float>(firstLayerZ) + 0.5f) * static_cast<float>(floor.GetDepth()) / static_cast<float>(GameConfig::kFirstLayerDepth)));

		// 床の外に出ないようにする
		x = std::clamp(x, 0, floor.GetWidth() - 1);
		z = std::clamp(z, 0, floor.GetDepth() - 1);
		return floor.GetCellCenter({ x, z });
	}
}
