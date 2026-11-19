#include "DescentSequence.h"
#include "../../Config/GameConfig.h"
#include "../../Camera/StageCamera.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Player/Player.h"
#include "../../Stage/Floor/Floor.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		// a から b へ、t (0 〜 1) の割合だけ進んだ値
		float LerpFloat(float a, float b, float t)
		{
			return a + (b - a) * t;
		}
	}

	// 降り始める
	void DescentSequence::Begin(const Vector3& bossPosition, const Vector3& playerStart, const Vector3& playerEnd)
	{
		dropOrigin_ = bossPosition;
		descentStart_ = playerStart;
		descentEnd_ = playerEnd;
		isBossLanded_ = false;
		timer_ = 0.0f;
	}

	// 1フレーム進める
	DescentSequence::Result DescentSequence::Advance(float deltaTime, Floor& current, const Floor& below, Player& player, Boss& boss, StageCamera& camera)
	{
		Result result{};

		// 進み具合 0 〜 1 (smooth はゆっくり始まって、ゆっくり止まる)
		timer_ += deltaTime;
		const float t = std::clamp(timer_ / GameConfig::kDescentDuration, 0.0f, 1.0f);
		const float smooth = t * t * (3.0f - 2.0f * t);

		// プレイヤー: 跳ねながら下の層の最初の位置へ (マスの線にそって)
		Vector3 position = Floor::AlongGrid(descentStart_, descentEnd_, smooth);
		position.y = LerpFloat(descentStart_.y, descentEnd_.y, t * t * t) + 4.0f * GameConfig::kDescentHopHeight * t * (1.0f - t);
		player.SetPosition(position);

		// カメラも下の層へ
		camera.SetFloorHeight(LerpFloat(current.GetHeight(), below.GetHeight(), smooth));

		// ボス: 切り離された床と一緒に下の層まで落ちる
		if (!isBossLanded_)
		{
			const float drop = std::clamp(timer_ / GameConfig::kBossDropDuration, 0.0f, 1.0f);
			const float dropEased = drop * drop;
			current.AnimateDrop(GameConfig::kFloorSpacing * dropEased);
			boss.SetPosition({ dropOrigin_.x, LerpFloat(dropOrigin_.y, below.GetHeight(), dropEased), dropOrigin_.z });
			if (drop >= 1.0f)
			{
				// 着いた: 落ちた床を消す
				isBossLanded_ = true;
				current.FinishDrop();
				result.isBossLandedNow = true;
			}
		}

		// ボスもプレイヤーも着いたら終わり
		if (isBossLanded_ && t >= 1.0f)
		{
			result.isFinished = true;
		}
		return result;
	}
}
