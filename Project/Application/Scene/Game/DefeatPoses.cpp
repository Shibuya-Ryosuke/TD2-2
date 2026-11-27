#include "DefeatPoses.h"

namespace App::DefeatPoses
{
	//両手を前へ出して足を持ち上げ、吹っ飛ぶ途中だけ手足が少しはねる
	RigPose FlinchPose(float flight)
	{
		const float arc = 4.0f * flight * (1.0f - flight);
		RigPose pose;
		pose.bodyTilt = 11.0f;
		pose.waistLean = -12.0f;
		pose.waistTwist = 8.0f;
		pose.headNod = 12.0f;
		pose.armForwardL = 83.0f + 7.0f * arc;
		pose.armForwardR = 76.0f - 6.0f * arc;
		pose.armOutL = 12.0f;
		pose.armOutR = 16.0f;
		pose.legForwardL = 56.0f + 10.0f * arc;
		pose.legForwardR = 43.0f;
		pose.legOutL = 9.0f;
		pose.legOutR = 6.0f;
		pose.squash = 0.94f;
		return pose;
	}

	//腕を開いて足をそろえたまま仰向け
	RigPose DownPose()
	{
		RigPose pose;
		pose.bodyTilt = 90.0f;
		pose.waistTwist = -3.0f;
		pose.headRoll = 5.0f;
		pose.armForwardL = 9.0f;
		pose.armForwardR = 5.0f;
		pose.armOutL = 42.0f;
		pose.armOutR = 35.0f;
		pose.bob = -0.34f;
		return pose;
	}
}
