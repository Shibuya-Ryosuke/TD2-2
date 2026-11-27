#include "FloorView.h"
#include "Floor.h"
#include "../../Config/FloorConfig.h"
#include "../../Config/LayerConfig.h"
#include "../../Config/LightConfig.h"
#include "../../Config/ModelConfig.h"
#include "../../Config/DirectionConfig.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App
{
	//床があるマスにモデルを作る
	void FloorView::CreateModels(const Floor& floor)
	{
		models_.clear();
		models_.resize(floor.GetTileCount());
		crumble_.Initialize(floor.GetTileCount());
		jiggle_.Initialize(floor.GetTileCount());
		wear_.Initialize(floor.GetTileCount());
		const uint32_t shader = ShaderManager::Load(DirectionConfig::kFloorTileShader); //ハートと砕ける崩れる演出
		for (size_t i = 0; i < models_.size(); ++i)
		{
			//CSVで穴のマスはモデルを作らない
			if (!floor.GetTile(floor.ToCell(i)).isSolidAtStart)
			{
				continue;
			}
			models_[i] = Model::Create(ModelConfig::kCubeModel);
			models_[i]->SetShader(shader);
			models_[i]->SetCastShadow(LightConfig::kFloorCastsShadow);
		}
	}

	//タイルの位置と大きさと色と光り方を確定する
	void FloorView::Update(const Floor& floor)
	{
		//予告のマスの光を弱い強いでゆっくりくり返す
		previewGlowTime_ += GetDeltaTime();
		const float wave = 0.5f + 0.5f * std::sin(previewGlowTime_ * FloorConfig::kPreviewGlowSpeed * std::numbers::pi_v<float> * 2.0f);
		const float glow = FloorConfig::kPreviewGlowMin + (FloorConfig::kPreviewGlowMax - FloorConfig::kPreviewGlowMin) * wave;
		const Vector4& previewColor = floor.GetPreviewColor();
		const Vector3 glowColor = { previewColor.x, previewColor.y, previewColor.z };

		//すき間の分だけ小さいタイルの大きさ
		const float modelScale = 1.0f / ModelConfig::kCubeModelSize;
		const float sizeX = floor.GetTileSizeX() * (1.0f - FloorConfig::kTileGap) * modelScale;
		const float sizeY = FloorConfig::kTileThickness * modelScale;
		const float sizeZ = floor.GetTileSizeZ() * (1.0f - FloorConfig::kTileGap) * modelScale;

		//タイルごとに位置と向きと大きさと色を入れる
		for (size_t i = 0; i < models_.size(); ++i)
		{
			Model* model = models_[i].get();
			if (model == nullptr)
			{
				continue;
			}
			const Cell cell = floor.ToCell(i);
			const FloorTile& tile = floor.GetTile(cell);
			//崩れる演出はFloorCrumbleに任せる
			//着地のぷるるんで沈んだりふくらんだりする
			FloorCrumble::State crumble = crumble_.Compute(floor, cell);
			//穴のとなりの床に細いヒビを広げる
			crumble.wear = wear_.Advance(i, tile.isSolid && floor.IsHoleAdjacent(cell), GetDeltaTime());
			const FloorJiggle::Offset jiggle = jiggle_.Get(i);
			const float scale = tile.scaleRate * crumble.scale;
			model->SetTranslate(tile.position + crumble.shake + Vector3{ 0.0f, jiggle.offsetY, 0.0f });
			model->SetRotate(tile.rotate);
			model->SetScale({ sizeX * scale * jiggle.scaleXZ, sizeY * scale * jiggle.scaleY, sizeZ * scale * jiggle.scaleXZ });
			//床があるタイルだけ透かして、落ちていく途中のタイルはそのまま見せる
			Vector4 color = GetTileColor(floor, cell);
			BlendMode blend = BlendMode::None;
			if (tile.isSolid && floor.GetFade() < 1.0f)
			{
				color.w *= std::max(floor.GetFade(), 0.0f);
				blend = BlendMode::Normal;
			}
			model->SetColor(color);
			model->SetBlendMode(blend);
			crumble_.Apply(i, *model, crumble, floor, cell);

			//残った足場をためているときは全部のタイルを光らせる
			if (floor.GetChargeGlow() > 0.0f)
			{
				model->SetEmissive(floor.GetChargeColor(), floor.GetChargeGlow());
			}
			//予告のマスの上の面のふちの枠を予告の色で光らせてブルームでにじませる
			else if (IsPreviewGlowing(floor, cell))
			{
				model->SetEmissive(glowColor, glow);
			}
			else
			{
				model->SetEmissive({ 0.0f, 0.0f, 0.0f }, 0.0f);
			}
			model->TransferMatrix();
		}
		crumble_.Update();
		jiggle_.Update();
	}

	//着地したマスからぷるるんと揺らす
	void FloorView::Jiggle(const Floor& floor, const Cell& center, float power)
	{
		jiggle_.Trigger(floor, center, power);
	}

	//地面パンチで、歩数だけ遅らせてマスを跳ねさせる
	void FloorView::Bounce(const std::vector<int32_t>& steps, float height, float stepDelay, float speed, float damping) {
		jiggle_.TriggerBounce(steps, height, stepDelay, speed, damping);
	}

	//見えているタイルを描く
	void FloorView::Draw(const Floor& floor)
	{
		for (size_t i = 0; i < models_.size(); ++i)
		{
			const FloorTile& tile = floor.GetTile(floor.ToCell(i));
			if (models_[i] == nullptr || !tile.isVisible)
			{
				continue;
			}
			//透けきったタイルは描かない
			if (tile.isSolid && floor.GetFade() <= 0.0f)
			{
				continue;
			}
			models_[i]->Draw();
		}
		crumble_.Draw();
	}

	//切り離しと穴のとなりとふつうのいつもの色
	Vector4 FloorView::GetBaseColor(const Floor& floor, const Cell& cell) const
	{
		//切り離された床、穴のとなり、ふつうの順に色を決める
		if (floor.GetTile(cell).isIsolated)
		{
			return FloorConfig::kIsolatedColor;
		}
		if (floor.IsHoleAdjacent(cell))
		{
			return FloorConfig::kBonusColor;
		}
		return FloorConfig::kFloorColor;
	}

	//予告で光らせるマスか
	bool FloorView::IsPreviewGlowing(const Floor& floor, const Cell& cell) const
	{
		const FloorTile& tile = floor.GetTile(cell);
		//今いる層の予告を出している床だけ光らせる
		if (!floor.IsForeground() || !tile.isPreview || !tile.isSolid)
		{
			return false;
		}
		if (tile.isIsolated || tile.remaining >= 0.0f)
		{
			return false;
		}
		return true;
	}

	//下の層と予告も入れた今の色
	Vector4 FloorView::GetTileColor(const Floor& floor, const Cell& cell) const
	{
		const Vector4 baseColor = GetBaseColor(floor, cell);

		//今いる層でなければ暗くする
		if (!floor.IsForeground())
		{
			const float brightness = LayerConfig::kLowerFloorBrightness;
			return Vector4{ baseColor.x * brightness, baseColor.y * brightness, baseColor.z * brightness, baseColor.w };
		}

		//予告のマスも中はいつもの色で、枠だけシェーダーで光らせる
		return baseColor;
	}
}
