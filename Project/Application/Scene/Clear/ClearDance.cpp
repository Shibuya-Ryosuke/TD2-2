#include "ClearDance.h"
#include "../../Config/ShowcaseConfig.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		constexpr float kPi = std::numbers::pi_v<float>;
		constexpr float kGalBlendTime = 0.25f; //回り終わってからポーズになりきるまでの秒数

		//ゆっくり始まってゆっくり止まる進み具合
		float Smooth(float t)
		{
			const float c = std::clamp(t, 0.0f, 1.0f);
			return c * c * (3.0f - 2.0f * c);
		}

		//万歳で跳ねる姿勢で、hopは跳んでいる高さの割合
		RigPose BanzaiPose(float hop, float time)
		{
			RigPose pose;
			pose.armForwardL = 175.0f;
			pose.armForwardR = 175.0f;
			pose.armOutL = 15.0f + 12.0f * std::sin(time * 20.0f);
			pose.armOutR = 15.0f - 12.0f * std::sin(time * 20.0f);
			pose.headNod = -15.0f;
			pose.legForwardL = 25.0f * hop;
			pose.legForwardR = -15.0f * hop;
			pose.legOutL = 10.0f * hop;
			pose.legOutR = 10.0f * hop;
			pose.bob = ShowcaseConfig::kClearHopHeight * hop;
			pose.squash = 1.0f - 0.2f * (1.0f - std::min(hop * 4.0f, 1.0f));
			return pose;
		}

		//右手をカメラへ突き出し、左手を横へのばして、内またで腰と首をかしげるギャルっぽいポーズ
		RigPose GalPose(float beat)
		{
			const float bounce = std::abs(std::sin(kPi * beat));
			const float sway = std::sin(kPi * beat * 0.5f);
			RigPose pose;
			pose.armForwardR = 80.0f + 8.0f * std::sin(2.0f * kPi * beat);
			pose.armOutR = 5.0f;
			pose.armForwardL = 65.0f;
			pose.armOutL = 45.0f + 6.0f * sway;
			pose.legOutL = -6.0f;
			pose.legOutR = -6.0f;
			pose.legForwardL = -8.0f;
			pose.legForwardR = 10.0f;
			pose.waistRoll = 8.0f * sway;
			pose.waistLean = 6.0f;
			pose.headRoll = -14.0f - 6.0f * sway;
			pose.headNod = 6.0f;
			pose.bob = 0.12f * bounce;
			pose.squash = 1.0f - 0.05f * (1.0f - bounce);
			return pose;
		}
	}

	//プレイヤーとハートを作る
	void ClearDance::Initialize()
	{
		rig_.Initialize();
		hearts_.Initialize();
		timer_ = 0.0f;
		lastBeat_ = -1;
		lastHop_ = 0;
	}

	//ポーズを決めてリズムに乗っているか
	bool ClearDance::IsPosing() const
	{
		const float hopEnd = ShowcaseConfig::kClearHopTime * static_cast<float>(ShowcaseConfig::kClearHopCount);
		return timer_ >= hopEnd + ShowcaseConfig::kClearSpinTime;
	}

	//喜ぶ、回る、ポーズでリズムを1フレーム進める
	void ClearDance::Update(float deltaTime)
	{
		timer_ += deltaTime;
		hearts_.Update();
		const Vector3& feet = ShowcaseConfig::kClearPlayer;
		const float scale = ShowcaseConfig::kPlayerScale;
		const float faceCamera = std::atan2(ShowcaseConfig::kClearEye.x - feet.x, ShowcaseConfig::kClearEye.z - feet.z) * 180.0f / kPi;
		const float hopEnd = ShowcaseConfig::kClearHopTime * static_cast<float>(ShowcaseConfig::kClearHopCount);

		//わーい！と万歳しながら跳ねて、着地のたびにハート
		if (timer_ < hopEnd)
		{
			const float phase = std::fmod(timer_, ShowcaseConfig::kClearHopTime) / ShowcaseConfig::kClearHopTime;
			const float hop = 4.0f * phase * (1.0f - phase);
			const int32_t hops = static_cast<int32_t>(timer_ / ShowcaseConfig::kClearHopTime);
			if (hops != lastHop_)
			{
				lastHop_ = hops;
				hearts_.Burst(feet, 1.2f);
			}
			rig_.Apply(BanzaiPose(hop, timer_), feet, faceCamera, scale);
			return;
		}

		//右回りにくるっと1回転しながら、万歳からポーズへ
		const float spinTime = timer_ - hopEnd;
		if (spinTime < ShowcaseConfig::kClearSpinTime)
		{
			const float t = spinTime / ShowcaseConfig::kClearSpinTime;
			RigPose pose = LerpPose(BanzaiPose(0.0f, timer_), GalPose(0.0f), Smooth(t));
			pose.spin = 360.0f * Smooth(t);
			pose.bob = 0.4f * 4.0f * t * (1.0f - t);
			rig_.Apply(pose, feet, faceCamera, scale);
			return;
		}

		//ポーズを決めて、拍に合わせてはずみ、2拍ごとにハート
		const float poseTime = spinTime - ShowcaseConfig::kClearSpinTime;
		const float beat = poseTime / ShowcaseConfig::kClearBeat;
		RigPose pose = GalPose(beat);
		if (poseTime < kGalBlendTime)
		{
			pose = LerpPose(GalPose(0.0f), pose, Smooth(poseTime / kGalBlendTime));
		}
		rig_.Apply(pose, feet, faceCamera + ShowcaseConfig::kClearPoseTurn, scale);
		const int32_t beats = static_cast<int32_t>(beat) / ShowcaseConfig::kClearHeartBeats;
		if (beats != lastBeat_)
		{
			lastBeat_ = beats;
			hearts_.Burst(feet, 0.7f);
		}
	}

	//プレイヤーとハートを描く
	void ClearDance::Draw()
	{
		rig_.Draw();
		hearts_.Draw();
	}
}
