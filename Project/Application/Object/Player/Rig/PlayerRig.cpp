#include "PlayerRig.h"
#include "../../ModelColor.h"
#include "../../../Config/PlayerConfig.h"
#include "../../../Config/RigConfig.h"
#include <cmath>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		//行ベクトルの位置を行列で動かす
		Vector3 TransformPosition(const Vector3& p, const Matrix4x4& m)
		{
			return {
				p.x * m.m[0][0] + p.y * m.m[1][0] + p.z * m.m[2][0] + m.m[3][0],
				p.x * m.m[0][1] + p.y * m.m[1][1] + p.z * m.m[2][1] + m.m[3][1],
				p.x * m.m[0][2] + p.y * m.m[1][2] + p.z * m.m[2][2] + m.m[3][2],
			};
		}

		//関節をparentの子にして、parentから見た関節の位置に置く
		void Attach(WorldTransform& joint, WorldTransform& parent, const Vector3& offset)
		{
			joint.SetParent(&parent);
			joint.SetLocal({ { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, offset });
		}
	}

	//6つのパーツを読みこんで、関節の位置を原点にずらして関節につなぐ
	void PlayerRig::Initialize()
	{
		Attach(tilt_, root_, RigConfig::kWaistPivot);
		Attach(joints_[kWaist], tilt_, { 0.0f, 0.0f, 0.0f });
		Attach(joints_[kNeck], joints_[kWaist], RigConfig::kNeckPivot - RigConfig::kWaistPivot);
		Attach(joints_[kShoulderL], joints_[kWaist], RigConfig::kLeftShoulderPivot - RigConfig::kWaistPivot);
		Attach(joints_[kShoulderR], joints_[kWaist], RigConfig::kRightShoulderPivot - RigConfig::kWaistPivot);
		Attach(joints_[kHipL], tilt_, RigConfig::kLeftHipPivot - RigConfig::kWaistPivot);
		Attach(joints_[kHipR], tilt_, RigConfig::kRightHipPivot - RigConfig::kWaistPivot);
		Attach(rightHand_, joints_[kShoulderR], RigConfig::kRightHand - RigConfig::kRightShoulderPivot);

		//パーツと親の関節と、関節の位置
		struct PartSetup
		{
			const char* model;
			Joint joint;
			Vector3 pivot;
		};
		const std::array<PartSetup, kPartCount> setups = { {
			{ RigConfig::kBodyModel, kWaist, RigConfig::kWaistPivot },
			{ RigConfig::kHeadModel, kNeck, RigConfig::kNeckPivot },
			{ RigConfig::kLeftArmModel, kShoulderL, RigConfig::kLeftShoulderPivot },
			{ RigConfig::kRightArmModel, kShoulderR, RigConfig::kRightShoulderPivot },
			{ RigConfig::kLeftLegModel, kHipL, RigConfig::kLeftHipPivot },
			{ RigConfig::kRightLegModel, kHipR, RigConfig::kRightHipPivot },
		} };
		for (size_t i = 0; i < setups.size(); ++i)
		{
			std::unique_ptr<Model> part = Model::Create(setups[i].model);
			ModelColor::Apply(*part, setups[i].model);
			part->SetParent(&joints_[static_cast<size_t>(setups[i].joint)]);
			part->SetTranslate({ -setups[i].pivot.x, -setups[i].pivot.y, -setups[i].pivot.z });
			parts_[i] = std::move(part);
		}
	}

	//足元の位置と向きと倍率と姿勢から全部のパーツの行列を確定する
	void PlayerRig::Apply(const RigPose& pose, const Vector3& feet, float yawDeg, float scale)
	{
		//縦につぶした分だけ横にふくらませて、横向きのモデルを正面に向ける
		const float squash = std::max(pose.squash, 0.1f);
		const float widen = 1.0f / std::sqrt(squash);
		const float lift = (PlayerConfig::kPlayerModelLift + pose.bob) * scale;
		root_.SetLocal({
			{ scale * widen, scale * squash, scale * widen },
			{ 0.0f, yawDeg + pose.spin + PlayerConfig::kPlayerModelYaw, 0.0f },
			{ feet.x, feet.y + lift, feet.z } });

		//体全体を腰で後ろへ倒すのは前と逆のZ
		Transform tiltLocal = tilt_.GetLocal();
		tiltLocal.rotate = { 0.0f, 0.0f, -pose.bodyTilt };
		tilt_.SetLocal(tiltLocal);

		//関節の回転は、前へはZ、横へはX、ひねりはYで、下を向いた手足は前がマイナスになる
		const auto setRotate = [this](Joint joint, const Vector3& rotate)
		{
			Transform local = joints_[static_cast<size_t>(joint)].GetLocal();
			local.rotate = rotate;
			joints_[static_cast<size_t>(joint)].SetLocal(local);
		};
		setRotate(kWaist, { pose.waistRoll, pose.waistTwist, pose.waistLean });
		setRotate(kNeck, { pose.headRoll, pose.headYaw, pose.headNod });
		setRotate(kShoulderL, { pose.armOutL, 0.0f, -pose.armForwardL });
		setRotate(kShoulderR, { -pose.armOutR, 0.0f, -pose.armForwardR });
		setRotate(kHipL, { pose.legOutL, 0.0f, -pose.legForwardL });
		setRotate(kHipR, { -pose.legOutR, 0.0f, -pose.legForwardR });

		//親から順に行列を確定する
		root_.UpdateMatrix();
		tilt_.UpdateMatrix();
		for (WorldTransform& joint : joints_)
		{
			joint.UpdateMatrix();
		}
		rightHand_.UpdateMatrix();
		for (std::unique_ptr<Model>& part : parts_)
		{
			part->TransferMatrix();
		}
	}

	//6つのパーツを描く
	void PlayerRig::Draw()
	{
		for (std::unique_ptr<Model>& part : parts_)
		{
			part->Draw();
		}
	}

	//今の姿勢での手の先のワールドの位置
	Vector3 PlayerRig::GetHandPosition(bool isRight) const
	{
		if (isRight)
		{
			return TransformPosition(RigConfig::kRightHand - RigConfig::kRightShoulderPivot, joints_[kShoulderR].GetWorldMatrix());
		}
		return TransformPosition(RigConfig::kLeftHand - RigConfig::kLeftShoulderPivot, joints_[kShoulderL].GetWorldMatrix());
	}

	//今の姿勢での頭のてっぺんあたりのワールドの位置
	Vector3 PlayerRig::GetHeadPosition() const
	{
		return TransformPosition({ 0.0f, 1.6f, 0.0f }, joints_[kNeck].GetWorldMatrix());
	}
}
