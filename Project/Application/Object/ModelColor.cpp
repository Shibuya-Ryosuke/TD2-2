#include "ModelColor.h"
#include "GlbJson.h"
#include <fstream>
#include <json.hpp>
#include <sstream>
#include <vector>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		//mtlのマテリアル名とKdの色
		struct MtlColor
		{
			std::string name;
			Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		};

		//objのパスの拡張子をmtlにしたパス
		std::string ToMtlPath(const std::string& objPath)
		{
			const size_t dot = objPath.find_last_of('.');
			if (dot == std::string::npos)
			{
				return objPath + ".mtl";
			}
			return objPath.substr(0, dot) + ".mtl";
		}

		//mtlを読んでマテリアルごとのKdの色を集める
		std::vector<MtlColor> LoadMtlColors(const std::string& path)
		{
			std::vector<MtlColor> colors;
			std::ifstream file(path);
			std::string line;
			while (std::getline(file, line))
			{
				std::istringstream stream(line);
				std::string key;
				stream >> key;
				//新しいマテリアルが始まる
				if (key == "newmtl")
				{
					MtlColor color{};
					stream >> color.name;
					colors.push_back(color);
					continue;
				}
				//今のマテリアルの色
				if (key == "Kd" && !colors.empty())
				{
					Vector4& color = colors.back().color;
					stream >> color.x >> color.y >> color.z;
				}
			}
			return colors;
		}

		//パスがglbで終わるか
		bool IsGlb(const std::string& path)
		{
			const std::string extension = ".glb";
			if (path.size() < extension.size())
			{
				return false;
			}
			return path.compare(path.size() - extension.size(), extension.size(), extension) == 0;
		}

		//glbの中のJSONを読んでマテリアルごとのbaseColorFactorの色を集める
		std::vector<MtlColor> LoadGlbColors(const std::string& path)
		{
			std::vector<MtlColor> colors;
			const nlohmann::json json = GlbJson::Read(path);
			if (!json.contains("materials"))
			{
				return colors;
			}
			for (const nlohmann::json& material : json["materials"])
			{
				MtlColor color{};
				color.name = material.value("name", std::string());
				//色が書いていなければ白のまま
				if (material.contains("pbrMetallicRoughness"))
				{
					const nlohmann::json& pbr = material["pbrMetallicRoughness"];
					if (pbr.contains("baseColorFactor") && pbr["baseColorFactor"].size() >= 3)
					{
						const nlohmann::json& factor = pbr["baseColorFactor"];
						color.color = { factor[0].get<float>(), factor[1].get<float>(), factor[2].get<float>(), 1.0f };
					}
				}
				colors.push_back(color);
			}
			return colors;
		}
	}

	//nameのモデルのファイルに書いてある色をmodelのメッシュに塗る
	void ModelColor::Apply(Model& model, const std::string& name)
	{
		const std::string modelPath = AssetRegistry::Resolve(AssetRegistry::Type::Model, name);
		if (modelPath.empty())
		{
			return;
		}
		//glbは中のJSONから、objは同じ名前のmtlから色を読む
		std::vector<MtlColor> colors;
		if (IsGlb(modelPath))
		{
			colors = LoadGlbColors(modelPath);
		}
		else
		{
			colors = LoadMtlColors(ToMtlPath(modelPath));
		}

		//メッシュのマテリアル名と同じ名前の色を塗る
		const size_t meshCount = model.GetMeshCount();
		for (size_t index = 0; index < meshCount; ++index)
		{
			const std::string& material = model.GetMaterialName(index);
			for (const MtlColor& color : colors)
			{
				if (color.name == material)
				{
					model.SetColor(color.color, static_cast<int32_t>(index));
					break;
				}
			}
		}
	}
}
