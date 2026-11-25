#include "ToneMapSwitch.h"
#include "../../Config/DirectionConfig.h"
#include <RyoEngine.h>

using namespace RyoEngine;

namespace App
{
	//今の設定を覚えてからDirectionConfigの設定にする
	void ToneMapSwitch::Apply()
	{
		savedAcesEnabled_ = PostProcess::IsACESEnabled();
		PostProcess::SetACESEnabled(DirectionConfig::kGameUsesAces);
	}

	//覚えておいた設定に戻す
	void ToneMapSwitch::Restore()
	{
		PostProcess::SetACESEnabled(savedAcesEnabled_);
	}
}
