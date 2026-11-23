#include "NearFadeSwitch.h"
#include <RyoEngine.h>

using namespace RyoEngine;

namespace App
{
	//今の設定を覚えてから切る
	void NearFadeSwitch::Disable()
	{
		savedStart_ = ModelManager::GetNearFadeStart();
		savedEnd_ = ModelManager::GetNearFadeEnd();
		ModelManager::SetNearFade(0.0f, 0.0f);
	}

	//覚えておいた設定に戻す
	void NearFadeSwitch::Restore()
	{
		ModelManager::SetNearFade(savedStart_, savedEnd_);
	}
}
