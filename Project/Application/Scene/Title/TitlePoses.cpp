#include "TitlePoses.h"
#include "../../Config/TitleConfig.h"
#include <cmath>

namespace App::TitlePoses
{
	//積み木を胸の前で両手で持って、体をゆっくり左右にゆらゆら
	RigPose HoldPose(float time)
	{
		const float sway = std::sin(time * TitleConfig::kSwaySpeed);
		RigPose pose;
		pose.armForwardL = 72.0f;
		pose.armForwardR = 72.0f;
		pose.armOutL = -22.0f;
		pose.armOutR = -22.0f;
		pose.waistRoll = TitleConfig::kSwayRoll * sway;
		pose.headRoll = -TitleConfig::kSwayRoll * 0.8f * sway;
		pose.headNod = 4.0f;
		pose.legOutL = 3.0f + 2.0f * sway;
		pose.legOutR = 3.0f - 2.0f * sway;
		pose.bob = 0.04f * std::abs(sway);
		return pose;
	}

	//積み木がすべってびっくりして手が上がり、落ちた積み木を見る
	RigPose SurprisePose()
	{
		RigPose pose;
		pose.armForwardL = 125.0f;
		pose.armForwardR = 125.0f;
		pose.armOutL = 35.0f;
		pose.armOutR = 35.0f;
		pose.headNod = 28.0f;
		pose.waistLean = -8.0f;
		pose.bob = 0.25f;
		pose.squash = 1.06f;
		return pose;
	}

	//あせって手足をばたばたさせながら首をふる
	RigPose PanicPose(float time)
	{
		RigPose pose;
		pose.armForwardL = 120.0f + 50.0f * std::sin(time * 25.0f);
		pose.armForwardR = 120.0f - 50.0f * std::sin(time * 25.0f);
		pose.armOutL = 40.0f;
		pose.armOutR = 40.0f;
		pose.headYaw = 30.0f * std::sin(time * 14.0f);
		pose.headNod = 10.0f;
		const float hop = std::abs(std::sin(time * 9.0f));
		pose.bob = 0.35f * hop;
		pose.legForwardL = 30.0f * std::sin(time * 18.0f);
		pose.legForwardR = -30.0f * std::sin(time * 18.0f);
		return pose;
	}

	//床が抜けて宙で固まり、手を上げたまま下を見る
	RigPose HangPose()
	{
		RigPose pose;
		pose.armForwardL = 150.0f;
		pose.armForwardR = 150.0f;
		pose.armOutL = 30.0f;
		pose.armOutR = 30.0f;
		pose.headNod = 35.0f;
		pose.legForwardL = 12.0f;
		pose.legForwardR = -6.0f;
		pose.squash = 1.08f;
		return pose;
	}

	//仰向けになって、上へ向けた足をばたばた、手もぱたぱた
	RigPose SupinePose(float time)
	{
		RigPose pose;
		pose.bodyTilt = 90.0f;
		pose.spin = TitleConfig::kSupineTurn;
		pose.legForwardL = 70.0f + 35.0f * std::sin(time * 24.0f);
		pose.legForwardR = 70.0f - 35.0f * std::sin(time * 24.0f);
		pose.legOutL = 10.0f;
		pose.legOutR = 10.0f;
		pose.armForwardL = 100.0f + 30.0f * std::sin(time * 19.0f);
		pose.armForwardR = 100.0f - 30.0f * std::sin(time * 19.0f);
		pose.armOutL = 30.0f;
		pose.armOutR = 30.0f;
		pose.headNod = 25.0f + 8.0f * std::sin(time * 12.0f);
		pose.headRoll = 10.0f * std::sin(time * 9.0f);
		return pose;
	}

	//落ちながら手を上にばたばた、足をけって、上を見上げる
	RigPose FallPose(float time)
	{
		RigPose pose;
		pose.armForwardL = 160.0f + 25.0f * std::sin(time * 22.0f);
		pose.armForwardR = 160.0f - 25.0f * std::sin(time * 22.0f);
		pose.armOutL = 45.0f + 20.0f * std::sin(time * 17.0f);
		pose.armOutR = 45.0f + 20.0f * std::cos(time * 17.0f);
		pose.legForwardL = 35.0f * std::sin(time * 20.0f);
		pose.legForwardR = -35.0f * std::sin(time * 20.0f);
		pose.headNod = -25.0f;
		pose.waistLean = -10.0f;
		return pose;
	}
}
