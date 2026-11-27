#include "BossFallCamera.h"
#include "../../Camera/StageCamera.h"
#include "../../Config/BossFallConfig.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	//今のカメラの床の高さを基準にして、今の映し方からつなぎ始める
	void BossFallCamera::Begin(const StageCamera& camera)
	{
		baseHeight_ = camera.GetFloorHeight();
		Cut(camera);
	}

	//今の映し方からつなぎ直す
	void BossFallCamera::Cut(const StageCamera& camera)
	{
		fromOffset_ = camera.GetFocusOffset();
		fromOrthoSize_ = camera.GetOrthoSize();
	}

	//targetを画面のまん中に映して寄る映し方へ、tの割合だけ近づける
	void BossFallCamera::Focus(StageCamera& camera, const Vector3& target, float orthoSize, float t)
	{
		Blend(camera, camera.GetCenteringOffset(target), orthoSize, t);
	}

	//上の層と下の層の両方を映す引いた映し方へ、tの割合だけ近づける
	void BossFallCamera::Wide(StageCamera& camera, float upperHeight, float lowerHeight, float t)
	{
		//床の高さを上げ下げするのは、映す所を縦にずらすのと同じ
		const float middle = upperHeight + (lowerHeight - upperHeight) * BossFallConfig::kWideHeight;
		Blend(camera, { 0.0f, middle - baseHeight_, 0.0f }, camera.GetBaseOrthoSize() * BossFallConfig::kWideZoom, t);
	}

	//heightの層のいつもの映し方へ、tの割合だけ近づける
	void BossFallCamera::Normal(StageCamera& camera, float height, float t)
	{
		Blend(camera, { 0.0f, height - baseHeight_, 0.0f }, camera.GetBaseOrthoSize(), t);
	}

	//ずらした分を消して、heightの層のいつもの映し方にする
	void BossFallCamera::Finish(StageCamera& camera, float height)
	{
		camera.ResetShot();
		camera.SetFloorHeight(height);
	}

	//今の床の高さのまま、映す所をずらした分と広さをtの割合だけ近づける
	void BossFallCamera::Blend(StageCamera& camera, const Vector3& offset, float orthoSize, float t)
	{
		const float clamped = std::clamp(t, 0.0f, 1.0f);
		const float smooth = clamped * clamped * (3.0f - 2.0f * clamped);
		camera.SetFloorHeight(baseHeight_);
		camera.SetFocusOffset(fromOffset_ + (offset - fromOffset_) * smooth);
		camera.SetOrthoSize(fromOrthoSize_ + (orthoSize - fromOrthoSize_) * smooth);
	}
}
