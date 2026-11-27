#pragma once
#include <RyoEngine.h>

//シーンの始めに、太陽の光と影の側を照らす光と底上げの光をそろえる
namespace App::SunLight
{
	void Apply(); //LightConfigの値で光をそろえて、なければ影の側を照らす光を足す
}
