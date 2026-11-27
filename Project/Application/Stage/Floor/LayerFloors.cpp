#include "LayerFloors.h"
#include "../../Config/LayerConfig.h"
#include "../../Config/TutorialConfig.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//layer層目から始める
	void LayerFloors::Start(int32_t layer)
	{
		layer_ = layer;
		current_ = CreateFloor(layer_, true);
		below_.reset();
		upper_.reset();
	}

	//下の層がまだなければ作る
	void LayerFloors::EnsureBelow()
	{
		if (below_ == nullptr)
		{
			below_ = CreateFloor(layer_ + 1, false);
		}
	}

	//下の層へ進む
	void LayerFloors::AdvanceLayer()
	{
		EnsureBelow();

		//今の層を上の層にして、下の層を明るい今の層にする
		current_->ClearPreview();
		upper_ = std::move(current_);
		current_ = std::move(below_);
		current_->ShowAsCurrentFloor();

		//さらに下の層を作る
		++layer_;
		below_ = CreateFloor(layer_ + 1, false);
	}

	//上の層を消す
	void LayerFloors::ReleaseUpper()
	{
		upper_.reset();
	}

	//3つの床の見た目を確定する
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

	//下の層、今の層、上の層の順に描く
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

	//index層目の床を作る
	std::unique_ptr<Floor> LayerFloors::CreateFloor(int32_t index, bool isForeground) const
	{
		//チュートリアルだけ専用の床を使い、本編の各層は同じ床を読み込む
		const char* path = TutorialConfig::kCsvPath;
		if (index >= 0)
		{
			path = LayerConfig::kBattleCsvPath;
		}
		const float height = -static_cast<float>(index) * LayerConfig::kFloorSpacing;

		//床を作って読み込む
		std::unique_ptr<Floor> floor = std::make_unique<Floor>();
		floor->Initialize(path, height, isForeground);
		return floor;
	}

	//1層目のマスで決めた最初の位置をこの層のマスに直す
	Vector3 LayerFloors::GetSpawnPosition(const Floor& floor, int32_t firstLayerX, int32_t firstLayerZ)
	{
		//1層目のマスで指定した場所をこの層のマスの数に合わせる
		int32_t x = static_cast<int32_t>(std::floor((static_cast<float>(firstLayerX) + 0.5f) * static_cast<float>(floor.GetWidth()) / static_cast<float>(LayerConfig::kFirstLayerWidth)));
		int32_t z = static_cast<int32_t>(std::floor((static_cast<float>(firstLayerZ) + 0.5f) * static_cast<float>(floor.GetDepth()) / static_cast<float>(LayerConfig::kFirstLayerDepth)));

		//床の外に出ないようにする
		x = std::clamp(x, 0, floor.GetWidth() - 1);
		z = std::clamp(z, 0, floor.GetDepth() - 1);
		return floor.GetCellCenter({ x, z });
	}
}
