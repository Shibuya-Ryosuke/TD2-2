#pragma once
#include "../../Object/Player/Rig/RigPose.h"

//やられ演出で吹っ飛んで倒れるプレイヤーの姿勢
namespace App::DefeatPoses
{
	RigPose FlinchPose(float flight); //両手を前へ出して足を持ち上げ、吹っ飛ぶ途中だけ手足が少しはねる
	RigPose DownPose(); //腕を開いて足をそろえたまま仰向け
}
