#pragma once
#include <RyoEngine.h>
#include <string>

namespace App
{
	//objと同じ名前のmtlに書いてあるKdの色をメッシュごとに塗る
	namespace ModelColor
	{
		//nameのobjのmtlの色をmodelのメッシュに塗る
		void Apply(RyoEngine::Model& model, const std::string& name);
	}
}
