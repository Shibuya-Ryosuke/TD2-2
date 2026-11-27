#include "PlayerArms.h"
#include "../GlbJson.h"
#include "../../Config/PlayerConfig.h"
#include <functional>

using namespace RyoEngine;

namespace App
{
	//glbの中から腕のメッシュを探して、左右の腕に肩から大きくするシェーダーをつける
	void PlayerArms::Initialize(Model& body, const std::string& modelName)
	{
		body_ = &body;
		isReady_ = false;
		shoulderLeft_ = body.GetJointTransform(PlayerConfig::kArmJointLeft);
		shoulderRight_ = body.GetJointTransform(PlayerConfig::kArmJointRight);
		if (shoulderLeft_ == nullptr || shoulderRight_ == nullptr)
		{
			return;
		}

		//エンジンのメッシュとマテリアルの名前が順番どおりにそろっているときだけ使う
		const std::vector<MeshEntry> meshes = ListMeshes(modelName);
		if (meshes.size() != body.GetMeshCount())
		{
			Logger::LogWarning("[PlayerArms] glbのメッシュの数がエンジンと合わないので腕は大きくしません\n");
			return;
		}
		for (size_t index = 0; index < meshes.size(); ++index)
		{
			if (meshes[index].material != body.GetMaterialName(index))
			{
				Logger::LogWarning("[PlayerArms] glbのメッシュの順番がエンジンと合わないので腕は大きくしません\n");
				return;
			}
		}

		//左右の腕のメッシュだけ、肩から大きくするシェーダーにする
		const uint32_t shaderLeft = ShaderManager::Load(PlayerConfig::kArmShaderLeft);
		const uint32_t shaderRight = ShaderManager::Load(PlayerConfig::kArmShaderRight);
		for (size_t index = 0; index < meshes.size(); ++index)
		{
			const int32_t side = ToArmSide(meshes[index]);
			if (side == 1)
			{
				body.SetShader(shaderLeft, static_cast<int32_t>(index));
			}
			else if (side == 2)
			{
				body.SetShader(shaderRight, static_cast<int32_t>(index));
			}
		}
		isReady_ = true;
		Update();
	}

	//体の行列を確定したあとに、今の肩の位置と倍率をシェーダーに渡す
	void PlayerArms::Update()
	{
		if (!isReady_)
		{
			return;
		}
		//0番は着地のピカッなので、1番から倍率と左の肩と右の肩を入れる
		const Vector3& left = shoulderLeft_->GetLocal().translate;
		const Vector3& right = shoulderRight_->GetLocal().translate;
		body_->SetShaderParam(1, PlayerConfig::kArmScale);
		body_->SetShaderParam(2, left.x);
		body_->SetShaderParam(3, left.y);
		body_->SetShaderParam(4, left.z);
		body_->SetShaderParam(5, right.x);
		body_->SetShaderParam(6, right.y);
		body_->SetShaderParam(7, right.z);
	}

	//右手のボーンの位置を、大きくした腕の手の位置へ動かし、手の大きさも合わせる
	Transform PlayerArms::ScaleRightHand(const Transform& hand) const
	{
		if (!isReady_)
		{
			return hand;
		}
		const float scale = PlayerConfig::kArmScale;
		const Vector3& shoulder = shoulderRight_->GetLocal().translate;
		Transform scaled = hand;
		scaled.translate = shoulder + (hand.translate - shoulder) * scale;
		scaled.scale = hand.scale * scale;
		return scaled;
	}

	//glbの中のノードを、エンジンと同じ深さ優先で子は前からの順にたどって、メッシュごとに並べる
	std::vector<PlayerArms::MeshEntry> PlayerArms::ListMeshes(const std::string& modelName)
	{
		std::vector<MeshEntry> entries;
		const std::string path = AssetRegistry::Resolve(AssetRegistry::Type::Model, modelName);
		const nlohmann::json json = GlbJson::Read(path);
		if (!json.contains("nodes") || !json.contains("meshes") || !json.contains("scenes"))
		{
			return entries;
		}
		const nlohmann::json& nodes = json["nodes"];
		const nlohmann::json& meshes = json["meshes"];
		const nlohmann::json& materials = json.value("materials", nlohmann::json::array());

		//ノードが持つメッシュは1つの区切りごとに1つのメッシュになる
		const std::function<void(size_t)> visit = [&](size_t nodeIndex)
		{
			const nlohmann::json& node = nodes[nodeIndex];
			if (node.contains("mesh"))
			{
				const nlohmann::json& mesh = meshes[node["mesh"].get<size_t>()];
				for (const nlohmann::json& primitive : mesh["primitives"])
				{
					MeshEntry entry;
					entry.node = node.value("name", std::string());
					if (primitive.contains("material"))
					{
						entry.material = materials[primitive["material"].get<size_t>()].value("name", std::string());
					}
					entries.push_back(entry);
				}
			}
			if (node.contains("children"))
			{
				for (const nlohmann::json& child : node["children"])
				{
					visit(child.get<size_t>());
				}
			}
		};
		const size_t sceneIndex = json.value("scene", static_cast<size_t>(0));
		for (const nlohmann::json& root : json["scenes"][sceneIndex]["nodes"])
		{
			visit(root.get<size_t>());
		}
		return entries;
	}

	//腕のノード名と結合された袖のマテリアル名で左右を見分ける
	int32_t PlayerArms::ToArmSide(const MeshEntry& mesh)
	{
		if (mesh.node == PlayerConfig::kSleeveNode)
		{
			if (mesh.material == PlayerConfig::kSleeveMaterialLeft)
			{
				return 1;
			}
			if (mesh.material == PlayerConfig::kSleeveMaterialRight)
			{
				return 2;
			}
			return 0;
		}
		const std::string& node = mesh.node;
		bool isArm = false;
		for (const char* name : PlayerConfig::kArmMeshNames)
		{
			if (node.rfind(name, 0) == 0)
			{
				isArm = true;
			}
		}
		if (!isArm || node.size() < 2)
		{
			return 0;
		}
		const std::string side = node.substr(node.size() - 2);
		if (side == ".L")
		{
			return 1;
		}
		if (side == ".R")
		{
			return 2;
		}
		return 0;
	}
}
