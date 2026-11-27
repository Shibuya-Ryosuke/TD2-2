#include "PlayerRig.h"
#include "../../ModelColor.h"
#include "../../../Config/PlayerConfig.h"
#include "../../../Config/RigConfig.h"
#include <Graphics/3D/ModelManager.h>
#include <cmath>
#include <initializer_list>
#include <utility>

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


		//読み込み済みの服の面が内向きなら頂点順と法線をそろえて反転する
		void CorrectBodyWinding(Model& body)
		{
			const ModelManager::ModelAsset* asset = ModelManager::GetAsset(body.GetModelIndex());
			if (asset == nullptr || asset->hasSkeleton)
			{
				return;
			}
			for (const ModelManager::MeshAsset& mesh : asset->meshes)
			{
				bool isFlipped = false;
				for (const char* material : RigConfig::kFlippedBodyMaterials)
				{
					if (mesh.materialName == material)
					{
						isFlipped = true;
					}
				}
				if (!isFlipped)
				{
					continue;
				}
				if (!mesh.vertexResource || mesh.vertexCount == 0 || mesh.vertexCount % 3 != 0
					|| mesh.vertexBufferView.StrideInBytes != sizeof(VertexData))
				{
					continue;
				}

				const D3D12_RANGE readRange = { 0, static_cast<SIZE_T>(mesh.vertexCount) * sizeof(VertexData) };
				void* mapped = nullptr;
				if (FAILED(mesh.vertexResource->Map(0, &readRange, &mapped)))
				{
					Logger::LogWarning("[PlayerRig] 服の頂点バッファを開けなかったので面の向きは直しません\n");
					continue;
				}
				VertexData* vertices = static_cast<VertexData*>(mapped);
				const auto position = [](const VertexData& vertex)
				{
					return Vector3{ vertex.position.x, vertex.position.y, vertex.position.z };
				};
				Vector3 center{};
				for (uint32_t i = 0; i < mesh.vertexCount; ++i)
				{
					center = center + position(vertices[i]);
				}
				center = center * (1.0f / static_cast<float>(mesh.vertexCount));

				//面が囲む符号付き体積から服の外側と内側を見分ける
				float volume = 0.0f;
				for (uint32_t i = 0; i < mesh.vertexCount; i += 3)
				{
					const Vector3 a = position(vertices[i]) - center;
					const Vector3 b = position(vertices[i + 1]) - center;
					const Vector3 c = position(vertices[i + 2]) - center;
					volume += Dot(a, Cross(b, c));
				}
				D3D12_RANGE writtenRange{};
				if (volume < -0.00001f)
				{
					for (uint32_t i = 0; i < mesh.vertexCount; i += 3)
					{
						std::swap(vertices[i + 1], vertices[i + 2]);
						vertices[i].normal = vertices[i].normal * -1.0f;
						vertices[i + 1].normal = vertices[i + 1].normal * -1.0f;
						vertices[i + 2].normal = vertices[i + 2].normal * -1.0f;
					}
					writtenRange = readRange;
				}
				mesh.vertexResource->Unmap(0, &writtenRange);
			}
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

		//腕は肩を中心に大きくして、右手に持たせるものは元の大きさのままにする
		const float arm = PlayerConfig::kArmScale;
		for (Joint shoulder : { kShoulderL, kShoulderR })
		{
			Transform local = joints_[static_cast<size_t>(shoulder)].GetLocal();
			local.scale = { arm, arm, arm };
			joints_[static_cast<size_t>(shoulder)].SetLocal(local);
		}
		Transform handLocal = rightHand_.GetLocal();
		handLocal.scale = { 1.0f / arm, 1.0f / arm, 1.0f / arm };
		rightHand_.SetLocal(handLocal);

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
			if (i == kBody)
			{
				CorrectBodyWinding(*part);
			}
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
