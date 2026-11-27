#include "PlayerPose.h"
#include "../../Config/DirectionConfig.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//すぐにいつもの姿勢へ戻す
	void PlayerPose::Reset()
	{
		kind_ = Kind::Normal;
		flail_ = 0.0f;
		sit_ = 0.0f;
		spin_ = 0.0f;
		time_ = 0.0f;
	}

	//なりたい姿勢へ近づける
	void PlayerPose::Update(float deltaTime)
	{
		time_ += deltaTime;
		float flailTarget = 0.0f;
		float sitTarget = 0.0f;
		if (kind_ == Kind::Flail || kind_ == Kind::Dangle)
		{
			flailTarget = 1.0f;
		}
		if (kind_ == Kind::Sit)
		{
			sitTarget = 1.0f;
		}

		//尻もちはどすんと速く、戻るのはゆっくり
		const float blend = 1.0f - std::exp(-DirectionConfig::kPoseBlendSpeed * deltaTime);
		flail_ += (flailTarget - flail_) * blend;
		float sitBlend = blend;
		if (sitTarget > sit_)
		{
			sitBlend = 1.0f - std::exp(-DirectionConfig::kSitDropSpeed * deltaTime);
		}
		sit_ += (sitTarget - sit_) * sitBlend;
	}

	//今のじたばたと尻もちの割合からパーツの姿勢を作る
	RigPose PlayerPose::MakeRigPose() const
	{
		if (kind_ == Kind::Dangle)
		{
			//足を垂らして交互に揺らし、腕を広げたまま体を回す
			const float swing = std::sin(time_ * 11.0f);
			RigPose dangle;
			dangle.armForwardL = 45.0f + 9.0f * swing;
			dangle.armForwardR = 45.0f - 9.0f * swing;
			dangle.armOutL = 37.0f;
			dangle.armOutR = 37.0f;
			dangle.legForwardL = 13.0f + 23.0f * swing;
			dangle.legForwardR = 13.0f - 23.0f * swing;
			dangle.legOutL = 7.0f;
			dangle.legOutR = 7.0f;
			dangle.bodyTilt = 14.0f;
			dangle.headRoll = 8.0f * swing;
			RigPose pose = LerpPose(RigPose{}, dangle, flail_);
			pose.spin = spin_;
			return pose;
		}

		//じたばたは腕を上下にばたばた、足を交互にけって、頭をふるふる
		const float speed = DirectionConfig::kFlailSpeed;
		RigPose flail;
		flail.armForwardL = 60.0f + 70.0f * std::sin(time_ * speed);
		flail.armForwardR = 60.0f - 70.0f * std::sin(time_ * speed);
		flail.armOutL = 50.0f + 30.0f * std::sin(time_ * speed * 0.8f);
		flail.armOutR = 50.0f + 30.0f * std::cos(time_ * speed * 0.8f);
		flail.legForwardL = 40.0f * std::sin(time_ * speed * 0.9f);
		flail.legForwardR = -40.0f * std::sin(time_ * speed * 0.9f);
		flail.headRoll = 10.0f * std::sin(time_ * speed * 0.4f);
		flail.waistLean = -10.0f;

		//尻もちは足を前に投げ出して後ろへ手をつき、少し沈む
		RigPose sit;
		sit.legForwardL = 85.0f;
		sit.legForwardR = 85.0f;
		sit.waistLean = -12.0f;
		sit.armForwardL = -40.0f;
		sit.armForwardR = -40.0f;
		sit.armOutL = 20.0f;
		sit.armOutR = 20.0f;
		sit.bob = -DirectionConfig::kSitSink;

		RigPose pose = LerpPose(RigPose{}, flail, flail_);
		return LerpPose(pose, sit, sit_);
	}
}
