#include "ClearPoses.h"
#include <cmath>
#include <numbers>

namespace App::ClearPoses
{
	//ハンマーを右肩にかついで、手足を交互に振って歩く
	RigPose WalkPose(float time)
	{
		const float step = std::sin(time * 9.0f);
		RigPose pose;
		pose.legForwardL = 28.0f * step;
		pose.legForwardR = -28.0f * step;
		pose.armForwardL = -22.0f * step;
		pose.armForwardR = 150.0f + 6.0f * step;
		pose.armOutR = -10.0f;
		pose.waistLean = 6.0f;
		pose.waistTwist = 5.0f * step;
		pose.bob = 0.08f * std::abs(std::cos(time * 9.0f));
		return pose;
	}

	//ハンマーを頭の上へ大きく振り上げて、体を後ろへそらす
	RigPose WindupPose()
	{
		RigPose pose;
		pose.armForwardR = 200.0f;
		pose.armOutR = -38.0f;
		pose.armForwardL = 150.0f;
		pose.armOutL = 15.0f;
		pose.waistLean = -14.0f;
		pose.headNod = -10.0f;
		pose.legForwardL = 18.0f;
		pose.legForwardR = -12.0f;
		pose.squash = 1.04f;
		return pose;
	}

	//ハンマーを前の下へ振り下ろして、前へかがむ
	RigPose SmashPose()
	{
		RigPose pose;
		pose.armForwardR = 72.0f;
		pose.armOutR = -38.0f;
		pose.armForwardL = 60.0f;
		pose.armOutL = 10.0f;
		pose.waistLean = 30.0f;
		pose.headNod = 15.0f;
		pose.legForwardL = 22.0f;
		pose.legForwardR = -18.0f;
		pose.bob = -0.15f;
		pose.squash = 0.9f;
		return pose;
	}

	//右手をカメラへ突き出し、左手を横前へのばして、内またで腰と首をかしげて拍に乗る
	RigPose GalPose(float beat)
	{
		const float pi = std::numbers::pi_v<float>;
		const float bounce = std::abs(std::sin(pi * beat));
		const float sway = std::sin(pi * beat * 0.5f);
		RigPose pose;
		pose.armForwardR = 80.0f + 8.0f * std::sin(2.0f * pi * beat);
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
