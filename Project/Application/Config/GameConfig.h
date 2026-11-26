#pragma once
#include "BossConfig.h"
#include "FloorConfig.h"
#include "LayerConfig.h"
#include "ModelConfig.h"
#include "PlayerConfig.h"
#include "SkyConfig.h"

//ボスのコードがまだ使っているGameConfigの名前を、分けたConfigへつなぐ
namespace App::GameConfig
{
	using namespace BossConfig;
	using namespace FloorConfig;
	using namespace LayerConfig;
	using namespace ModelConfig;
	using namespace PlayerConfig;
	using namespace SkyConfig;
}
