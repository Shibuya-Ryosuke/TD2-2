#pragma once
#include <RyoEngine.h>
#include <string>

namespace App
{
	//objならmtlのKd、glbなら中のbaseColorFactorの色をメッシュごとに塗る
	namespace ModelColor
	{
		//nameのモデルのファイルに書いてある色をmodelのメッシュに塗る
		void Apply(RyoEngine::Model& model, const std::string& name);
	}
}
