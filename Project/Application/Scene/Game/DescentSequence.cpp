#include "DescentSequence.h"
#include "../../Config/DirectionConfig.h"
#include "../../Config/LayerConfig.h"
#include "../../Camera/StageCamera.h"
#include "../../Object/Boss/Boss.h"
#include "../../Stage/Floor/Floor.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		//aからbへtの割合だけ進んだ値
		float LerpFloat(float a, float b, float t)
		{
			return a + (b - a) * t;
		}
	}

	//ボスの位置から落ち始める
	void DescentSequence::Begin(const Vector3& bossPosition)
	{
		dropOrigin_ = bossPosition;
		isBossLanded_ = false;
		timer_ = 0.0f;
	}

	//1フレーム進める
	DescentSequence::Result DescentSequence::Advance(float deltaTime, Floor& current, const Floor& below, Boss& boss, StageCamera& camera)
	{
		Result result{};

		//ゆっくり始まってゆっくり止まる進み具合
		timer_ += deltaTime;
		const float t = std::clamp(timer_ / LayerConfig::kDescentDuration, 0.0f, 1.0f);
		const float smooth = t * t * (3.0f - 2.0f * t);

		//プレイヤーは上の層に残ったまま、カメラは上の層と下の層の間まで下がって少し引く
		const float middle = LerpFloat(current.GetHeight(), below.GetHeight(), DirectionConfig::kBossFallViewHeight);
		camera.SetFloorHeight(LerpFloat(current.GetHeight(), middle, smooth));
		camera.SetOrthoSize(camera.GetBaseOrthoSize() * LerpFloat(1.0f, DirectionConfig::kBossFallViewZoom, smooth));

		//ボスは切り離された床と一緒に下の層まで落ちる
		if (!isBossLanded_)
		{
			const float drop = std::clamp(timer_ / LayerConfig::kBossDropDuration, 0.0f, 1.0f);
			const float dropEased = drop * drop;
			current.AnimateDrop(LayerConfig::kFloorSpacing * dropEased);
			boss.SetPosition({ dropOrigin_.x, LerpFloat(dropOrigin_.y, below.GetHeight(), dropEased), dropOrigin_.z });
			if (drop >= 1.0f)
			{
				//着いたら落ちた床を消す
				isBossLanded_ = true;
				current.FinishDrop();
				result.isBossLandedNow = true;
			}
		}

		//ボスが着いてカメラも下がりきったら終わり
		if (isBossLanded_ && t >= 1.0f)
		{
			result.isFinished = true;
		}
		return result;
	}
}
