#include "TutorialSpot.h"
#include "../../Config/FloorConfig.h"
#include "../../Config/ModelConfig.h"
#include "../../Config/TutorialConfig.h"
#include "../../Stage/Floor/Floor.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		//仮の箱をスポットライトのシェーダーで加算して光らせる
		std::unique_ptr<Model> CreateLightModel(uint32_t shader, float mode)
		{
			std::unique_ptr<Model> model = Model::Create(ModelConfig::kCubeModel);
			model->SetShader(shader);
			model->SetBlendMode(BlendMode::Add);
			model->SetCastShadow(false);
			model->SetEnableLighting(false);
			model->SetShaderParam(1, mode);
			return model;
		}
	}

	//光の柱と枠のモデルを作る
	void TutorialSpot::Initialize()
	{
		const uint32_t shader = ShaderManager::Load(TutorialConfig::kSpotShader);
		for (Spot& spot : spots_)
		{
			spot.beam = CreateLightModel(shader, 0.0f);
			spot.frame = CreateLightModel(shader, 1.0f);
			spot.isUsed = false;
		}
		strength_ = 0.0f;
		isShown_ = false;
	}

	//照らすマスを決めてふわっと光らせる
	void TutorialSpot::Show(const std::vector<Cell>& cells)
	{
		for (size_t i = 0; i < spots_.size(); ++i)
		{
			spots_[i].isUsed = i < cells.size();
			if (spots_[i].isUsed)
			{
				spots_[i].cell = cells[i];
			}
		}
		isShown_ = true;
	}

	//ふわっと消す
	void TutorialSpot::Hide()
	{
		isShown_ = false;
	}

	//まだ床があるマスだけ光をマスに合わせる
	void TutorialSpot::Update(const Floor& floor)
	{
		const float step = GetDeltaTime() / TutorialConfig::kSpotFadeTime;
		if (isShown_)
		{
			strength_ = std::min(strength_ + step, 1.0f);
		}
		else
		{
			strength_ = std::max(strength_ - step, 0.0f);
		}

		//箱は1辺がkCubeModelSizeなので、マスとの比で大きさを決める
		const float modelScale = 1.0f / ModelConfig::kCubeModelSize;
		const float sizeX = floor.GetTileSizeX();
		const float sizeZ = floor.GetTileSizeZ();
		for (Spot& spot : spots_)
		{
			spot.isVisible = spot.isUsed && strength_ > 0.0f && floor.IsSolid(spot.cell);
			if (!spot.isVisible)
			{
				continue;
			}
			const Vector3 center = floor.GetTile(spot.cell).position;
			const float top = center.y + FloorConfig::kTileThickness * 0.5f;
			const float height = TutorialConfig::kSpotHeight;
			spot.beam->SetScale({ sizeX * TutorialConfig::kSpotWidthRate * modelScale, height * modelScale, sizeZ * TutorialConfig::kSpotWidthRate * modelScale });
			spot.beam->SetTranslate({ center.x, top + height * 0.5f, center.z });
			spot.beam->SetShaderParam(0, strength_);
			spot.beam->TransferMatrix();
			spot.frame->SetScale({ sizeX * modelScale, TutorialConfig::kSpotFrameThickness * modelScale, sizeZ * modelScale });
			spot.frame->SetTranslate({ center.x, top + TutorialConfig::kSpotFrameThickness, center.z });
			spot.frame->SetShaderParam(0, strength_);
			spot.frame->TransferMatrix();
		}
	}

	//照らしているマスの光を描く
	void TutorialSpot::Draw()
	{
		for (Spot& spot : spots_)
		{
			if (!spot.isVisible)
			{
				continue;
			}
			spot.frame->Draw();
			spot.beam->Draw();
		}
	}
}
