#include "BossGhost.h"
#include "../../Config/GameConfig.h"
#include "../../Config/DirectionConfig.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Boss/BossDetail.h"
#include "../../Stage/Floor/Floor.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace RyoEngine;
using namespace App::BossDetail;

namespace App
{
	namespace
	{
		//1辺2のcube.objを1辺sizeの箱にする大きさ
		Vector3 CubeScale(const Vector3& size)
		{
			return size * (1.0f / GameConfig::kCubeModelSize);
		}

		//始めと終わりがゆっくりになるイージング
		float SmoothStep(float t)
		{
			return t * t * (3.0f - 2.0f * t);
		}
	}

	//影を落とさない半透明の体と拳を作る
	void BossGhost::Initialize()
	{
		body_ = Model::Create(GameConfig::kCubeModel);
		fist_ = Model::Create(GameConfig::kCubeModel);
		for (Model* model : { body_.get(), fist_.get() })
		{
			model->SetColor(DirectionConfig::kBossGhostColor);
			model->SetBlendMode(BlendMode::Normal);
			model->SetCastShadow(false);
		}
	}

	//本当の時間を縮めて最小と最大におさめた見せる秒数
	float BossGhost::GetShowTime(const BossPlannedAction& action)
	{
		return std::clamp(action.duration * DirectionConfig::kBossGhostTimeScale, DirectionConfig::kBossGhostMinTime, DirectionConfig::kBossGhostMaxTime);
	}

	//先読みを作り直して残像を進める
	void BossGhost::Update(const Boss& boss, const Floor& floor, const Vector3& player, const BossTuning& tuning, bool isVisible)
	{
		isVisible_ = isVisible && boss.IsActive();
		if (!isVisible_)
		{
			return;
		}
		const float deltaTime = GetDeltaTime();

		//ボスが動いたか層が変わったら最初から見せ直す
		refreshTimer_ -= deltaTime;
		const bool isRestart = lastActionCount_ != boss.GetActionCount() || lastFloor_ != &floor;
		if (isRestart || refreshTimer_ <= 0.0f)
		{
			const float speed = tuning.moveSpeed * (1.0f + GameConfig::kMaxBossSpeedBonus * floor.GetCollapsedFraction());
			boss.BuildForecast(floor, player, tuning, speed, GameConfig::kJumpDuration, plan_, GameConfig::kBossForecastTurns);
			refreshTimer_ = DirectionConfig::kBossGhostRefresh;
			lastActionCount_ = boss.GetActionCount();
			lastFloor_ = &floor;
			if (isRestart)
			{
				elapsed_ = 0.0f;
			}
		}
		if (plan_.empty())
		{
			isVisible_ = false;
			return;
		}

		//全部の行動をくり返して今見せる行動を探す
		float total = 0.0f;
		for (const BossPlannedAction& action : plan_)
		{
			total += GetShowTime(action);
		}
		elapsed_ = std::fmod(elapsed_ + deltaTime, total);
		float time = elapsed_;
		size_t index = 0;
		while (index + 1 < plan_.size() && time >= GetShowTime(plan_[index]))
		{
			time -= GetShowTime(plan_[index]);
			++index;
		}
		const BossPlannedAction& action = plan_[index];
		//下の層へ行ってから決める行動は見せない
		if (action.kind == BossActionKind::Deferred)
		{
			isVisible_ = false;
			return;
		}
		Pose(action, std::clamp(time / GetShowTime(action), 0.0f, 1.0f), tuning);
	}

	//見せている行動に合わせて体と拳を置く
	void BossGhost::Pose(const BossPlannedAction& action, float t, const BossTuning& tuning)
	{
		const float pi = std::numbers::pi_v<float>;
		Vector3 position = action.from;
		Vector3 fist = kFistRest;
		float squash = 0.0f;

		//跳ぶ行動はなめらかに山なりで着地先へ
		if (action.kind == BossActionKind::Move && !action.isBlocked)
		{
			position = Lerp(action.from, action.to, SmoothStep(t));
			position.y += 4.0f * tuning.jumpHeight * t * (1.0f - t);
		}
		//パンチは後半で拳が伸びて戻る
		if (action.kind == BossActionKind::Punch)
		{
			const float strike = std::sin(std::clamp((t - 0.4f) / 0.6f, 0.0f, 1.0f) * pi);
			fist = Lerp(kFistRest, Vector3{ 0.0f, kFistRest.y, tuning.punchRange }, strike);
		}
		//近接攻撃は体がむぎゅっとつぶれる
		if (action.kind == BossActionKind::Melee)
		{
			squash = 0.07f * std::sin(t * pi);
		}
		//落ちる行動は下へ落ちていく
		if (action.kind == BossActionKind::Fall)
		{
			position.y -= 6.0f * t * t;
		}

		//動くなら行き先を、動かないなら狙う所を向く
		Vector3 forward = action.to - action.from;
		if (forward.x * forward.x + forward.z * forward.z <= 0.001f)
		{
			forward = action.target - action.from;
		}
		float yaw = 0.0f;
		if (forward.x * forward.x + forward.z * forward.z > 0.001f)
		{
			yaw = std::atan2(forward.x, forward.z) * 180.0f / pi;
		}
		const Vector3 rotate = { 0.0f, yaw, 0.0f };

		//つぶれても足元が動かない体
		const Vector3 bodySize = { kBodySize.x, kBodySize.y * (1.0f - squash), kBodySize.z };
		body_->SetScale(CubeScale(bodySize));
		body_->SetRotate(rotate);
		body_->SetTranslate(position + RotateY({ 0.0f, bodySize.y * 0.5f, 0.0f }, yaw));
		body_->TransferMatrix();

		//拳
		fist_->SetScale(CubeScale(kFistSize));
		fist_->SetRotate(rotate);
		fist_->SetTranslate(position + RotateY(fist, yaw));
		fist_->TransferMatrix();
	}

	//見えているときだけ描く
	void BossGhost::Draw()
	{
		if (!isVisible_)
		{
			return;
		}
		body_->Draw();
		fist_->Draw();
	}
}
