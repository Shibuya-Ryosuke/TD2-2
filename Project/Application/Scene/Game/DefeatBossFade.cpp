#include "DefeatBossFade.h"
#include "../../Camera/StageCamera.h"
#include "../../Config/DirectionConfig.h"
#include "../../Object/Boss/Boss.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//透かす前に戻す
	void DefeatBossFade::Reset(Boss& boss)
	{
		isFading_ = false;
		elapsed_ = 0.0f;
		boss.SetFade(1.0f);
	}

	//重なったら透かし始めて、決めた濃さまでなめらかに薄くする
	void DefeatBossFade::Update(float deltaTime, bool canStart, const StageCamera& camera, const Vector3& player, Boss& boss)
	{
		if (!isFading_)
		{
			if (!canStart || !IsCovering(camera, player, boss.GetPosition()))
			{
				return;
			}
			isFading_ = true;
		}
		elapsed_ += deltaTime;
		const float t = std::clamp(elapsed_ / DirectionConfig::kDefeatBossFadeTime, 0.0f, 1.0f);
		const float smooth = t * t * (3.0f - 2.0f * t);
		boss.SetFade(1.0f + (DirectionConfig::kDefeatBossFadeAlpha - 1.0f) * smooth);
	}

	//ボスとプレイヤーを画面に映した縦長の四角にして、手前でも奥でも四角が重なるか見る
	bool DefeatBossFade::IsCovering(const StageCamera& camera, const Vector3& player, const Vector3& boss)
	{
		const Vector3 forward = camera.GetViewForward();
		const Vector3 right = Normalize(Cross({ 0.0f, 1.0f, 0.0f }, forward));
		const Vector3 up = Cross(forward, right);
		const Vector3 bossHead = boss + Vector3{ 0.0f, DirectionConfig::kDefeatBossViewHeight, 0.0f };
		const Vector3 playerHead = player + Vector3{ 0.0f, DirectionConfig::kDefeatPlayerViewHeight, 0.0f };

		//横の重なり
		const float gapX = std::abs(Dot(boss - player, right));
		if (gapX > DirectionConfig::kDefeatBossViewRadius + DirectionConfig::kDefeatPlayerViewHalfWidth)
		{
			return false;
		}
		//縦の重なり
		const float bossBottom = std::min(Dot(boss, up), Dot(bossHead, up));
		const float bossTop = std::max(Dot(boss, up), Dot(bossHead, up));
		const float playerBottom = std::min(Dot(player, up), Dot(playerHead, up));
		const float playerTop = std::max(Dot(player, up), Dot(playerHead, up));
		return bossBottom < playerTop && playerBottom < bossTop;
	}
}
