#include "FallLines.h"
#include "../../Config/DirectionConfig.h"
#include <RyoEngine.h>
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	//シーンが変わると消えるのでシーンごとに用意する
	void FallLines::Initialize()
	{
		effect_ = PostEffectManager::Add(ShaderManager::LoadPostEffect(DirectionConfig::kFallLinesShader));
		PostEffectManager::SetEnabled(effect_, false);
		strength_ = 0.0f;
	}

	//降りている間は強く、降り終わったら弱くしていく
	void FallLines::Update(bool isFalling)
	{
		const float deltaTime = GetDeltaTime();
		if (isFalling)
		{
			strength_ = std::min(strength_ + deltaTime / DirectionConfig::kFallLinesRise, 1.0f);
		}
		else
		{
			strength_ = std::max(strength_ - deltaTime / DirectionConfig::kFallLinesFade, 0.0f);
		}

		//出ていない間は重くならないように止める
		PostEffectManager::SetEnabled(effect_, strength_ > 0.0f);
		PostEffectManager::SetParam(effect_, 0, strength_ * DirectionConfig::kFallLinesStrength);
	}
}
