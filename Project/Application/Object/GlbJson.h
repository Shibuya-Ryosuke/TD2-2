#pragma once
#include <json.hpp>
#include <string>

namespace App
{
	//glbのファイルの中に入っているJSONを読む
	namespace GlbJson
	{
		//読めなかったら中身のないJSONを返す
		nlohmann::json Read(const std::string& path);
	}
}
