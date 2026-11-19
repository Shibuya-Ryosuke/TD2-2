#include "FloorView.h"
#include "Floor.h"
#include "../../Config/GameConfig.h"
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App
{
	// 床があるマスにモデルを作る
	void FloorView::CreateModels(const Floor& floor)
	{
		models_.clear();
		models_.resize(floor.GetTileCount());
		for (size_t i = 0; i < models_.size(); ++i)
		{
			// CSV で穴のマスは、モデルも作らない
			if (!floor.GetTile(floor.ToCell(i)).isSolidAtStart)
			{
				continue;
			}
			models_[i] = Model::Create(GameConfig::kCubeModel);
		}
	}

	// タイルの 位置・大きさ・色・光り方 を確定する
	void FloorView::Update(const Floor& floor)
	{
		// 予告のマスの光の強さ: 弱い ⇔ 強い をゆっくりくり返す
		previewGlowTime_ += GetDeltaTime();
		const float wave = 0.5f + 0.5f * std::sin(previewGlowTime_ * GameConfig::kPreviewGlowSpeed * std::numbers::pi_v<float> * 2.0f);
		const float glow = GameConfig::kPreviewGlowMin + (GameConfig::kPreviewGlowMax - GameConfig::kPreviewGlowMin) * wave;
		const Vector4& previewColor = floor.GetPreviewColor();
		const Vector3 glowColor = { previewColor.x, previewColor.y, previewColor.z };

		// cube.obj の大きさに合わせて、タイルの大きさにする (すき間の分だけ小さく)
		const float modelScale = 1.0f / GameConfig::kCubeModelSize;
		const float sizeX = floor.GetTileSizeX() * (1.0f - GameConfig::kTileGap) * modelScale;
		const float sizeY = GameConfig::kTileThickness * modelScale;
		const float sizeZ = floor.GetTileSizeZ() * (1.0f - GameConfig::kTileGap) * modelScale;

		// タイルごとに 位置・向き・大きさ・色 を入れる
		for (size_t i = 0; i < models_.size(); ++i)
		{
			Model* model = models_[i].get();
			if (model == nullptr)
			{
				continue;
			}
			const Cell cell = floor.ToCell(i);
			const FloorTile& tile = floor.GetTile(cell);
			model->SetTranslate(tile.position);
			model->SetRotate(tile.rotate);
			model->SetScale({ sizeX * tile.scaleRate, sizeY * tile.scaleRate, sizeZ * tile.scaleRate });
			model->SetColor(GetTileColor(floor, cell));

			// 崩す形の予告のマスは、形の色で光らせる (エンジンの発光。ブルームでにじむ)
			if (IsPreviewGlowing(floor, cell))
			{
				model->SetEmissive(glowColor, glow);
			}
			else
			{
				model->SetEmissive({ 0.0f, 0.0f, 0.0f }, 0.0f);
			}
			model->TransferMatrix();
		}
	}

	// 見えているタイルを描く
	void FloorView::Draw(const Floor& floor)
	{
		for (size_t i = 0; i < models_.size(); ++i)
		{
			if (models_[i] == nullptr || !floor.GetTile(floor.ToCell(i)).isVisible)
			{
				continue;
			}
			models_[i]->Draw();
		}
	}

	// いつもの色 (切り離し・穴のとなり・ふつう)
	Vector4 FloorView::GetBaseColor(const Floor& floor, const Cell& cell) const
	{
		// 切り離された床 → 穴のとなり (飛ばすと 4 倍) → ふつう
		if (floor.GetTile(cell).isIsolated)
		{
			return GameConfig::kIsolatedColor;
		}
		if (floor.GetBarrageMultiplier(cell) == 4)
		{
			return GameConfig::kBonusColor;
		}
		return GameConfig::kFloorColor;
	}

	// 予告で光らせるマスか
	bool FloorView::IsPreviewGlowing(const Floor& floor, const Cell& cell) const
	{
		const FloorTile& tile = floor.GetTile(cell);
		// 今いる層の、予告を出している床だけ (崩す予定・切り離された床は光らせない)
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

	// 今の色 (下の層・崩れる前・予告 も入れる)
	Vector4 FloorView::GetTileColor(const Floor& floor, const Cell& cell) const
	{
		const FloorTile& tile = floor.GetTile(cell);
		const Vector4 baseColor = GetBaseColor(floor, cell);

		// 今いる層でなければ暗くする
		if (!floor.IsForeground())
		{
			const float brightness = GameConfig::kLowerFloorBrightness;
			return Vector4{ baseColor.x * brightness, baseColor.y * brightness, baseColor.z * brightness, baseColor.w };
		}

		// 崩れる予定: ふつうはオレンジ → 最後の少しは点滅 → 最後は赤
		if (!tile.isIsolated && tile.remaining >= 0.0f)
		{
			if (tile.remaining <= GameConfig::kCollapseRedDuration)
			{
				return GameConfig::kCollapseRedColor;
			}
			if (tile.remaining <= GameConfig::kCollapseWarningDuration)
			{
				const float phase = (GameConfig::kCollapseWarningDuration - tile.remaining) * GameConfig::kCollapseBlinkFrequency;
				if (phase - std::floor(phase) < 0.5f)
				{
					return GameConfig::kCollapseBlinkColor;
				}
			}
			return GameConfig::kSteppedTileColor;
		}

		// 崩す形の予告: いつもの色に予告の色を混ぜる
		if (!tile.isIsolated && tile.isPreview)
		{
			const Vector4& previewColor = floor.GetPreviewColor();
			const float t = floor.GetPreviewBlend();
			return Vector4{
				baseColor.x + (previewColor.x - baseColor.x) * t,
				baseColor.y + (previewColor.y - baseColor.y) * t,
				baseColor.z + (previewColor.z - baseColor.z) * t,
				baseColor.w + (previewColor.w - baseColor.w) * t,
			};
		}
		return baseColor;
	}
}
