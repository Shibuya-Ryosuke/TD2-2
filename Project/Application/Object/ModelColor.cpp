#include "ModelColor.h"
#include <fstream>
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
	}

	//nameのobjのmtlの色をmodelのメッシュに塗る
	void ModelColor::Apply(Model& model, const std::string& name)
	{
		const std::string objPath = AssetRegistry::Resolve(AssetRegistry::Type::Model, name);
		if (objPath.empty())
		{
			return;
		}
		const std::vector<MtlColor> colors = LoadMtlColors(ToMtlPath(objPath));

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
