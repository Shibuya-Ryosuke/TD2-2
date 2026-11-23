#include "DamageFlash.h"
#include "../../Config/DirectionConfig.h"
#include <RyoEngine.h>
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	//シーンが変わると消えるのでシーンごとに用意する
	void DamageFlash::Initialize()
	{
		effect_ = PostEffectManager::Add(ShaderManager::LoadPostEffect(DirectionConfig::kDamageVignetteShader));
		PostEffectManager::SetEnabled(effect_, false);
		timer_ = -1.0f;
	}

	//パッと光らせる
	void DamageFlash::Play()
	{
		timer_ = 0.0f;
		PostEffectManager::SetEnabled(effect_, true);
	}

	//すっと消していく
	void DamageFlash::Update()
	{
		if (timer_ < 0.0f)
		{
			return;
		}
		timer_ += GetDeltaTime();

		//パッと強くなってからすっと弱くなる
		float strength = 1.0f;
		if (timer_ < DirectionConfig::kDamageFlashRise)
		{
			strength = timer_ / DirectionConfig::kDamageFlashRise;
		}
		else
		{
			const float fade = (timer_ - DirectionConfig::kDamageFlashRise) / (DirectionConfig::kDamageFlashTime - DirectionConfig::kDamageFlashRise);
			strength = 1.0f - std::clamp(fade, 0.0f, 1.0f);
			strength *= strength; //最後はゆっくり消える
		}
		PostEffectManager::SetParam(effect_, 0, strength * DirectionConfig::kDamageFlashStrength);

		//消えきったら重くならないように止める
		if (timer_ >= DirectionConfig::kDamageFlashTime)
		{
			timer_ = -1.0f;
			PostEffectManager::SetEnabled(effect_, false);
		}
	}
}
