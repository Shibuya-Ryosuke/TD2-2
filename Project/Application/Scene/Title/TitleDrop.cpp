#include "TitleDrop.h"
#include "TitlePoses.h"
#include "../../Config/ShowcaseConfig.h"
#include "../../Config/TitleConfig.h"
#include "../../Object/Tsumiki/Tsumiki.h"
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
	}

	//プレイヤーと持っている積み木を作る
	void TitleDrop::Initialize()
	{
		rig_.Initialize();
		block_ = Tsumiki::Create();
		block_->SetColor(TitleConfig::kBlockColor);
		feet_ = TitleConfig::kPlayer;
		const Vector3 toEye = TitleConfig::kEye - feet_;
		faceYaw_ = std::atan2(toEye.x, toEye.z) * 180.0f / std::numbers::pi_v<float>;
		timer_ = 0.0f;
		startTimer_ = 0.0f;
		playerVelocityY_ = 0.0f;
		isStarted_ = false;
		isFloorDropped_ = false;
		blockRotate_ = { 0.0f, faceYaw_, 0.0f };
		rig_.Apply(MakePose(), feet_, faceYaw_, ShowcaseConfig::kPlayerScale);
		UpdateBlock(0.0f);
	}

	//積み木を落とす流れを始める
	void TitleDrop::Start()
	{
		if (isStarted_)
		{
			return;
		}
		isStarted_ = true;
		startTimer_ = 0.0f;

		//手からつるっと前へすべり落ちて、くるくる回る
		const float rad = faceYaw_ * std::numbers::pi_v<float> / 180.0f;
		blockVelocity_ = { std::sin(rad) * TitleConfig::kBlockSlide, 2.2f, std::cos(rad) * TitleConfig::kBlockSlide };
		blockSpin_ = { 260.0f, 90.0f, -180.0f };
	}

	//今の流れの姿勢で、びっくり、あせる、宙で固まる、じたばた落ちる、仰向けでばたばた
	RigPose TitleDrop::MakePose() const
	{
		if (!isStarted_)
		{
			return TitlePoses::HoldPose(timer_);
		}
		const float fallStart = TitleConfig::kFloorDropTime + TitleConfig::kPlayerFallDelay;
		const float flipStart = fallStart + TitleConfig::kFlipDelay;
		if (startTimer_ >= flipStart)
		{
			return LerpPose(TitlePoses::FallPose(timer_), TitlePoses::SupinePose(timer_), Smooth((startTimer_ - flipStart) / TitleConfig::kFlipTime));
		}
		if (startTimer_ >= fallStart)
		{
			return LerpPose(TitlePoses::HangPose(), TitlePoses::FallPose(timer_), Smooth((startTimer_ - fallStart) / 0.15f));
		}
		if (startTimer_ >= TitleConfig::kFloorDropTime)
		{
			return LerpPose(TitlePoses::PanicPose(timer_), TitlePoses::HangPose(), Smooth((startTimer_ - TitleConfig::kFloorDropTime) / 0.1f));
		}
		if (startTimer_ >= TitleConfig::kSlipTime)
		{
			return LerpPose(TitlePoses::SurprisePose(), TitlePoses::PanicPose(timer_), Smooth((startTimer_ - TitleConfig::kSlipTime) / 0.2f));
		}
		return LerpPose(TitlePoses::HoldPose(timer_), TitlePoses::SurprisePose(), Smooth(startTimer_ / 0.12f));
	}

	//ゆらゆらか、落としてから落ちるまでを1フレーム進める
	TitleDrop::Result TitleDrop::Update(float deltaTime)
	{
		Result result{};
		timer_ += deltaTime;
		if (isStarted_)
		{
			const float before = startTimer_;
			startTimer_ += deltaTime;

			//あせっている間に床ががたがたし始めて、そのあと床が落ちる
			if (before < TitleConfig::kRumbleTime && startTimer_ >= TitleConfig::kRumbleTime)
			{
				result.isRumbleNow = true;
			}
			if (!isFloorDropped_ && startTimer_ >= TitleConfig::kFloorDropTime)
			{
				isFloorDropped_ = true;
				result.isFloorDropNow = true;
			}

			//少し宙で固まってから、目で追える速さまでで落ちていく
			if (startTimer_ >= TitleConfig::kFloorDropTime + TitleConfig::kPlayerFallDelay)
			{
				playerVelocityY_ = std::max(playerVelocityY_ - TitleConfig::kPlayerGravity * deltaTime, -TitleConfig::kPlayerMaxFallSpeed);
				feet_.y += playerVelocityY_ * deltaTime;
			}
		}
		rig_.Apply(MakePose(), feet_, faceYaw_, ShowcaseConfig::kPlayerScale);
		UpdateBlock(deltaTime);
		return result;
	}

	//手の中か、落ちてはねる積み木
	void TitleDrop::UpdateBlock(float deltaTime)
	{
		const float size = TitleConfig::kBlockSize;
		if (!isStarted_)
		{
			//両手のまん中で少しだけゆらす
			const Vector3 hands = (rig_.GetHandPosition(true) + rig_.GetHandPosition(false)) * 0.5f;
			blockPosition_ = hands + Vector3{ 0.0f, -size * 0.1f, 0.0f };
			blockRotate_ = { 0.0f, faceYaw_ + 8.0f * std::sin(timer_ * TitleConfig::kSwaySpeed), 0.0f };
		}
		else
		{
			//落ちて床ではねて、床が消えたらそのまま下へ
			blockVelocity_.y -= TitleConfig::kBlockGravity * deltaTime;
			blockPosition_ = blockPosition_ + blockVelocity_ * deltaTime;
			blockRotate_ = blockRotate_ + blockSpin_ * deltaTime;
			const float ground = size * 0.5f;
			if (!isFloorDropped_ && blockPosition_.y < ground && blockVelocity_.y < 0.0f)
			{
				blockPosition_.y = ground;
				blockVelocity_.y = -blockVelocity_.y * TitleConfig::kBlockBounce;
				blockVelocity_.x *= 0.6f;
				blockVelocity_.z *= 0.6f;
				blockSpin_ = blockSpin_ * 0.5f;
			}
		}
		Tsumiki::Place(*block_, blockPosition_, { size, size, size }, blockRotate_);
		block_->TransferMatrix();
	}

	//プレイヤーと積み木を描く
	void TitleDrop::Draw()
	{
		rig_.Draw();
		block_->Draw();
	}
}
