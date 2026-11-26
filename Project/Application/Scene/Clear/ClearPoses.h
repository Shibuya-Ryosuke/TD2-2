#pragma once
#include "../../Object/Player/Rig/RigPose.h"

//クリアでボスをたたいて踊るプレイヤーの姿勢
namespace App::ClearPoses
{
	RigPose WalkPose(float time); //ハンマーを右肩にかついで、手足を交互に振って歩く
	RigPose WindupPose(); //ハンマーを頭の上へ大きく振り上げて、体を後ろへそらす
	RigPose SmashPose(); //ハンマーを前の下へ振り下ろして、前へかがむ
	RigPose GalPose(float beat); //右手をカメラへ突き出し、左手を横前へのばして、内またで腰と首をかしげる
}
