#include "RigPose.h"

namespace App
{
	namespace
	{
		//aからbへtだけ近づけた値
		float Mix(float a, float b, float t)
		{
			return a + (b - a) * t;
		}
	}

	//aからbへtだけ近づけた姿勢
	RigPose LerpPose(const RigPose& a, const RigPose& b, float t)
	{
		RigPose pose;
		pose.waistLean = Mix(a.waistLean, b.waistLean, t);
		pose.waistRoll = Mix(a.waistRoll, b.waistRoll, t);
		pose.waistTwist = Mix(a.waistTwist, b.waistTwist, t);
		pose.headNod = Mix(a.headNod, b.headNod, t);
		pose.headRoll = Mix(a.headRoll, b.headRoll, t);
		pose.headYaw = Mix(a.headYaw, b.headYaw, t);
		pose.armForwardL = Mix(a.armForwardL, b.armForwardL, t);
		pose.armForwardR = Mix(a.armForwardR, b.armForwardR, t);
		pose.armOutL = Mix(a.armOutL, b.armOutL, t);
		pose.armOutR = Mix(a.armOutR, b.armOutR, t);
		pose.legForwardL = Mix(a.legForwardL, b.legForwardL, t);
		pose.legForwardR = Mix(a.legForwardR, b.legForwardR, t);
		pose.legOutL = Mix(a.legOutL, b.legOutL, t);
		pose.legOutR = Mix(a.legOutR, b.legOutR, t);
		pose.bob = Mix(a.bob, b.bob, t);
		pose.squash = Mix(a.squash, b.squash, t);
		pose.spin = Mix(a.spin, b.spin, t);
		pose.bodyTilt = Mix(a.bodyTilt, b.bodyTilt, t);
		return pose;
	}
}
