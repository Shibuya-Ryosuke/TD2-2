#include "BossFallPoses.h"
#include "../../Config/BossFallConfig.h"
#include <cmath>
#include <numbers>

namespace App
{
	namespace
	{
		//fromからtoへtの割合だけ近づけた値
		float LerpValue(float from, float to, float t)
		{
			return from + (to - from) * t;
		}
	}

	//aからbへtの割合だけ近づけた姿勢
	BossPose LerpBossPose(const BossPose& a, const BossPose& b, float t)
	{
		BossPose pose;
		pose.sink = LerpValue(a.sink, b.sink, t);
		pose.bodyLean = LerpValue(a.bodyLean, b.bodyLean, t);
		pose.bodyRoll = LerpValue(a.bodyRoll, b.bodyRoll, t);
		pose.bodyTwist = LerpValue(a.bodyTwist, b.bodyTwist, t);
		pose.headNod = LerpValue(a.headNod, b.headNod, t);
		pose.headRoll = LerpValue(a.headRoll, b.headRoll, t);
		pose.armForwardL = LerpValue(a.armForwardL, b.armForwardL, t);
		pose.armForwardR = LerpValue(a.armForwardR, b.armForwardR, t);
		pose.armOutL = LerpValue(a.armOutL, b.armOutL, t);
		pose.armOutR = LerpValue(a.armOutR, b.armOutR, t);
		pose.legForwardL = LerpValue(a.legForwardL, b.legForwardL, t);
		pose.legForwardR = LerpValue(a.legForwardR, b.legForwardR, t);
		pose.legOutL = LerpValue(a.legOutL, b.legOutL, t);
		pose.legOutR = LerpValue(a.legOutR, b.legOutR, t);
		return pose;
	}
}

namespace App::BossFallPoses
{
	//足元がなくなって「えっ」と下を見て、手を少し広げてぷるぷる
	BossPose NoticePose(float time)
	{
		const float shiver = std::sin(time * BossFallConfig::kFlailSpeed * 2.0f);
		BossPose pose;
		pose.headNod = 28.0f;
		pose.armForwardL = 30.0f;
		pose.armForwardR = 30.0f;
		pose.armOutL = 35.0f + 4.0f * shiver;
		pose.armOutR = 35.0f - 4.0f * shiver;
		pose.legOutL = 6.0f;
		pose.legOutR = 6.0f;
		return pose;
	}

	//落ちながら両手を上へばたばたさせて、足もじたばた
	BossPose FlailPose(float time)
	{
		const float phase = time * BossFallConfig::kFlailSpeed;
		const float wave = std::sin(phase);
		const float other = std::sin(phase + std::numbers::pi_v<float>);
		BossPose pose;
		pose.bodyLean = -12.0f;
		pose.headNod = -22.0f;
		pose.headRoll = 10.0f * std::sin(phase * 0.7f);
		pose.armForwardL = 155.0f + 25.0f * wave;
		pose.armForwardR = 155.0f + 25.0f * other;
		pose.armOutL = 25.0f;
		pose.armOutR = 25.0f;
		pose.legForwardL = 20.0f + 30.0f * other;
		pose.legForwardR = 20.0f + 30.0f * wave;
		pose.legOutL = 12.0f;
		pose.legOutR = 12.0f;
		return pose;
	}

	//叩きつけられた瞬間で、お尻から落ちて手足が投げ出される
	BossPose SlamPose()
	{
		BossPose pose;
		pose.sink = BossFallConfig::kSitSink;
		pose.bodyLean = -25.0f;
		pose.headNod = 25.0f;
		pose.armForwardL = 40.0f;
		pose.armForwardR = 40.0f;
		pose.armOutL = 70.0f;
		pose.armOutR = 70.0f;
		pose.legForwardL = 100.0f;
		pose.legForwardR = 100.0f;
		pose.legOutL = 25.0f;
		pose.legOutR = 25.0f;
		return pose;
	}

	//座りこんで片手で頭を押さえ、もう片手を後ろにつく
	BossPose HurtSitPose(float time)
	{
		const float sway = std::sin(time * BossFallConfig::kHurtSwaySpeed);
		BossPose pose;
		pose.sink = BossFallConfig::kSitSink;
		pose.bodyLean = -16.0f;
		pose.bodyRoll = -6.0f;
		pose.bodyTwist = 8.0f;
		pose.headNod = 14.0f;
		pose.headRoll = -12.0f + 4.0f * sway;
		pose.armForwardR = 15.0f;
		pose.armOutR = 160.0f;
		pose.armForwardL = -45.0f;
		pose.armOutL = -10.0f;
		pose.legForwardL = 82.0f;
		pose.legForwardR = 96.0f;
		pose.legOutL = 14.0f;
		pose.legOutR = 20.0f;
		return pose;
	}

	//いつもの立ち姿
	BossPose StandPose()
	{
		return BossPose{};
	}
}
