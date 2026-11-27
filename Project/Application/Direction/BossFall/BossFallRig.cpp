#include "BossFallRig.h"
#include "../../Config/BossFallConfig.h"
#include "../../Config/ClearConfig.h"
#include "../../Object/ModelColor.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		//関節をparentの子にして、parentから見た関節の位置に置く
		void Attach(WorldTransform& joint, WorldTransform& parent, const Vector3& offset)
		{
			joint.SetParent(&parent);
			joint.SetLocal({ { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, offset });
		}
	}

	//6つのパーツを読みこんで、関節の位置を原点にずらして関節につなぐ
	void BossFallRig::Initialize()
	{
		using namespace BossFallConfig;
		Attach(joints_[kPelvis], root_, kPelvisPivot);
		Attach(joints_[kNeck], joints_[kPelvis], kNeckPivot - kPelvisPivot);
		Attach(joints_[kShoulderL], joints_[kPelvis], kLeftShoulderPivot - kPelvisPivot);
		Attach(joints_[kShoulderR], joints_[kPelvis], kRightShoulderPivot - kPelvisPivot);
		Attach(joints_[kHipL], root_, kLeftHipPivot);
		Attach(joints_[kHipR], root_, kRightHipPivot);

		//パーツと親の関節と、関節の位置
		struct PartSetup
		{
			const char* model;
			Joint joint;
			Vector3 pivot;
		};
		const std::array<PartSetup, kPartCount> setups = { {
			{ ClearConfig::kBossBody, kPelvis, kPelvisPivot },
			{ ClearConfig::kBossHead, kNeck, kNeckPivot },
			{ ClearConfig::kBossLeftArm, kShoulderL, kLeftShoulderPivot },
			{ ClearConfig::kBossRightArm, kShoulderR, kRightShoulderPivot },
			{ ClearConfig::kBossLeftFeet, kHipL, kLeftHipPivot },
			{ ClearConfig::kBossRightFeet, kHipR, kRightHipPivot },
		} };
		for (size_t i = 0; i < setups.size(); ++i)
		{
			std::unique_ptr<Model> part = Model::Create(setups[i].model);
			ModelColor::Apply(*part, setups[i].model);
			part->SetParent(&joints_[static_cast<size_t>(setups[i].joint)]);
			part->SetTranslate({ -setups[i].pivot.x, -setups[i].pivot.y, -setups[i].pivot.z });
			parts_[i] = std::move(part);
		}

		//一度も行列を確定していないモデルは原点に元の大きさで出てしまうので、最初から確定しておく
		Apply(BossPose{}, { 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f);
		SetFlash(0.0f);
	}

	//関節の回転を変える
	void BossFallRig::SetJointRotate(Joint joint, const Vector3& rotate)
	{
		Transform local = joints_[static_cast<size_t>(joint)].GetLocal();
		local.rotate = rotate;
		joints_[static_cast<size_t>(joint)].SetLocal(local);
	}

	//足元の位置と向きの度数と姿勢と、縦につぶす量から全部のパーツの行列を確定する
	void BossFallRig::Apply(const BossPose& pose, const Vector3& feet, float yawDeg, float squash)
	{
		//縦につぶした分だけ横にふくらませる
		const float scale = BossFallConfig::kRigScale;
		const float height = std::max(1.0f - squash, 0.2f);
		const float widen = 1.0f / std::sqrt(height);
		root_.SetLocal({
			{ scale * widen, scale * height, scale * widen },
			{ 0.0f, yawDeg, 0.0f },
			{ feet.x, feet.y - pose.sink * scale * height, feet.z } });

		//前へはXのマイナス、外へは左がZのマイナスで右がZのプラス、上半身を前へ倒すのはXのプラス
		SetJointRotate(kPelvis, { pose.bodyLean, pose.bodyTwist, -pose.bodyRoll });
		SetJointRotate(kNeck, { pose.headNod, 0.0f, -pose.headRoll });
		SetJointRotate(kShoulderL, { -pose.armForwardL, 0.0f, -pose.armOutL });
		SetJointRotate(kShoulderR, { -pose.armForwardR, 0.0f, pose.armOutR });
		SetJointRotate(kHipL, { -pose.legForwardL, 0.0f, -pose.legOutL });
		SetJointRotate(kHipR, { -pose.legForwardR, 0.0f, pose.legOutR });

		//親から順に行列を確定する
		root_.UpdateMatrix();
		for (WorldTransform& joint : joints_)
		{
			joint.UpdateMatrix();
		}
		for (std::unique_ptr<Model>& part : parts_)
		{
			part->TransferMatrix();
		}
	}

	//当たった瞬間の白い光の強さを入れる
	void BossFallRig::SetFlash(float flash)
	{
		for (std::unique_ptr<Model>& part : parts_)
		{
			part->SetEmissive(BossFallConfig::kHitFlashColor, std::max(flash, 0.0f));
		}
	}

	//6つのパーツを描く
	void BossFallRig::Draw()
	{
		for (std::unique_ptr<Model>& part : parts_)
		{
			part->Draw();
		}
	}
}
