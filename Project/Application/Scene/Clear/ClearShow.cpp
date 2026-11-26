#include "ClearShow.h"
#include "ClearPoses.h"
#include "../../Config/ClearConfig.h"
#include "../../Config/DirectionConfig.h"
#include "../../Config/ShowcaseConfig.h"
#include "../../Object/ModelColor.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		//ゆっくり始まってゆっくり止まる進み具合
		float Smooth(float t)
		{
			const float c = std::clamp(t, 0.0f, 1.0f);
			return c * c * (3.0f - 2.0f * c);
		}

		//流れの区切りの秒数
		constexpr float kWindupStart = ClearConfig::kWalkTime;
		constexpr float kSwingStart = kWindupStart + ClearConfig::kWindupTime;
		constexpr float kImpact = kSwingStart + ClearConfig::kSwingTime;
		constexpr float kTurnStart = kImpact + ClearConfig::kScatterTime;
		constexpr float kDanceStart = kTurnStart + ClearConfig::kTurnTime;

		//ダンスのときにカメラの方を向く度
		float DanceYaw()
		{
			const Vector3 toEye = ClearConfig::kDanceEye - ClearConfig::kPlayerStop;
			return std::atan2(toEye.x, toEye.z) * 180.0f / std::numbers::pi_v<float>;
		}
	}

	//プレイヤーとハンマーとボスを作る
	void ClearShow::Initialize()
	{
		rig_.Initialize();
		hammer_ = Model::Create(DirectionConfig::kHammerModel);
		ModelColor::Apply(*hammer_, DirectionConfig::kHammerModel);
		hammer_->SetParent(rig_.GetRightHand());
		boss_.Initialize();
		timer_ = 0.0f;
		hitStop_ = 0.0f;
		shake_ = 0.0f;
		isHammerHidden_ = false;
	}

	//今の流れの姿勢と向きと足元
	RigPose ClearShow::MakePose(float& yaw, Vector3& feet) const
	{
		yaw = ClearConfig::kPlayerFaceBossYaw;
		feet = ClearConfig::kPlayerStop;

		//ボスへ歩いていく
		if (timer_ < kWindupStart)
		{
			const float t = timer_ / ClearConfig::kWalkTime;
			feet = ClearConfig::kPlayerStart + (ClearConfig::kPlayerStop - ClearConfig::kPlayerStart) * t;
			return LerpPose(ClearPoses::WalkPose(timer_), RigPose{}, Smooth((t - 0.85f) / 0.15f));
		}

		//ためて振り上げ、一気に振り下ろす
		if (timer_ < kSwingStart)
		{
			return LerpPose(RigPose{}, ClearPoses::WindupPose(), Smooth((timer_ - kWindupStart) / ClearConfig::kWindupTime));
		}
		if (timer_ < kTurnStart)
		{
			RigPose pose = LerpPose(ClearPoses::WindupPose(), ClearPoses::SmashPose(), Smooth((timer_ - kSwingStart) / ClearConfig::kSwingTime));
			return LerpPose(pose, RigPose{}, Smooth((timer_ - kImpact - 0.6f) / 0.4f));
		}

		//右回りにくるっと回りながらカメラの方を向いて、ポーズで踊る
		const float danceYaw = DanceYaw() + 360.0f;
		if (timer_ < kDanceStart)
		{
			const float t = Smooth((timer_ - kTurnStart) / ClearConfig::kTurnTime);
			yaw = ClearConfig::kPlayerFaceBossYaw + (danceYaw - ClearConfig::kPlayerFaceBossYaw) * t;
			RigPose pose = LerpPose(RigPose{}, ClearPoses::GalPose(0.0f), t);
			pose.bob = 0.5f * 4.0f * t * (1.0f - t);
			return pose;
		}
		yaw = danceYaw;
		return ClearPoses::GalPose((timer_ - kDanceStart) / ClearConfig::kBeat);
	}

	//流れを1フレーム進める
	void ClearShow::Update(float deltaTime)
	{
		shake_ = std::max(shake_ - deltaTime, 0.0f);

		//当たった瞬間は少し止めて見せる
		if (hitStop_ > 0.0f)
		{
			hitStop_ -= deltaTime;
			return;
		}
		const float before = timer_;
		timer_ += deltaTime;

		//振り下ろしきったらボスがバラバラに飛ぶ
		if (before < kImpact && timer_ >= kImpact)
		{
			boss_.Scatter(ClearConfig::kImpactPoint);
			hitStop_ = ClearConfig::kHitStopTime;
			shake_ = 0.4f;
		}

		//回り始めにハンマーをしまう
		if (!isHammerHidden_ && timer_ >= kTurnStart)
		{
			isHammerHidden_ = true;
		}

		float yaw = 0.0f;
		Vector3 feet{};
		const RigPose pose = MakePose(yaw, feet);
		rig_.Apply(pose, feet, yaw, ShowcaseConfig::kPlayerScale);
		UpdateHammer();
		boss_.Update(deltaTime);
	}

	//右手のハンマーを腕の先にのばした向きで持たせる
	void ClearShow::UpdateHammer()
	{
		const float scale = ClearConfig::kHammerScale;
		hammer_->SetScale({ scale, scale, scale });
		hammer_->SetRotate({ ClearConfig::kHammerAlongArm, 0.0f, 0.0f });
		hammer_->SetTranslate({ 0.0f, 0.0f, 0.0f });
		hammer_->TransferMatrix();
	}

	//今の映し方のカメラの位置と見る所
	void ClearShow::GetShot(Vector3& eye, Vector3& target) const
	{
		//歩くのは引き、振り上げは下から寄り、飛ぶのは上から引き、踊りはまん中寄りの右
		const float blend = ClearConfig::kShotBlendTime;
		eye = ClearConfig::kWideEye;
		target = ClearConfig::kWideTarget;
		const float push = Smooth((timer_ - (kWindupStart - blend)) / blend);
		eye = eye + (ClearConfig::kPushEye - eye) * push;
		target = target + (ClearConfig::kPushTarget - target) * push;
		const float scatter = Smooth((timer_ - kImpact) / blend);
		eye = eye + (ClearConfig::kScatterEye - eye) * scatter;
		target = target + (ClearConfig::kScatterTarget - target) * scatter;
		const float dance = Smooth((timer_ - kTurnStart) / blend);
		eye = eye + (ClearConfig::kDanceEye - eye) * dance;
		target = target + (ClearConfig::kDanceTarget - target) * dance;

		//当たったら揺らす
		const float power = shake_ / 0.4f * ClearConfig::kImpactShake;
		const float time = shake_ * 60.0f;
		const Vector3 offset = { std::sin(time * 1.3f) * power, std::cos(time * 1.7f) * power, 0.0f };
		eye = eye + offset;
		target = target + offset;
	}

	//踊り始めたか
	bool ClearShow::IsDancing() const
	{
		return timer_ >= kDanceStart;
	}

	//プレイヤーとハンマーとボスを描く
	void ClearShow::Draw()
	{
		boss_.Draw();
		rig_.Draw();
		if (!isHammerHidden_)
		{
			hammer_->Draw();
		}
	}
}
