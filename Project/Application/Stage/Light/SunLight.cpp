#include "SunLight.h"
#include "../../Config/LightConfig.h"

using namespace RyoEngine;

namespace App::SunLight
{
	//LightConfigの値で光をそろえて、なければ影の側を照らす光を足す
	void Apply()
	{
		//太陽はエンジンの0番の光で、影の向きもこの光で決まる
		if (!LightManager::HasLight(LightConfig::kSunName))
		{
			LightManager::AddLight(LightType::Directional, LightConfig::kSunName);
		}
		LightManager::SetLightEnabled(LightConfig::kSunName, true);
		LightManager::SetLightDirection(LightConfig::kSunName, Normalize(LightConfig::kSunDirection));
		LightManager::SetLightColor(LightConfig::kSunName, LightConfig::kSunColor);
		LightManager::SetLightIntensity(LightConfig::kSunName, LightConfig::kSunIntensity);

		//影の側をうっすら照らす光
		if (!LightManager::HasLight(LightConfig::kFillName))
		{
			LightManager::AddLight(LightType::Directional, LightConfig::kFillName);
		}
		LightManager::SetLightEnabled(LightConfig::kFillName, true);
		LightManager::SetLightDirection(LightConfig::kFillName, Normalize(LightConfig::kFillDirection));
		LightManager::SetLightColor(LightConfig::kFillName, LightConfig::kFillColor);
		LightManager::SetLightIntensity(LightConfig::kFillName, LightConfig::kFillIntensity);

		//どこにでも当たる底上げの光
		LightManager::SetAmbientColor(LightConfig::kAmbientColor);
		LightManager::SetAmbientIntensity(LightConfig::kAmbientIntensity);
	}
}
