#pragma once
#include "../../Object/Player/Rig/RigPose.h"

//タイトルで積み木を持つプレイヤーの姿勢
namespace App::TitlePoses
{
	RigPose HoldPose(float time); //積み木を胸の前で持ってゆらゆら
	RigPose SurprisePose(); //積み木がすべってびっくりして手が上がる
	RigPose PanicPose(float time); //あせって手足をばたばたさせながら首をふる
	RigPose FallPose(float time); //落ちながらじたばた
	RigPose HangPose(); //床が抜けて宙で固まり、手を上げたまま下を見る
	RigPose SupinePose(float time); //仰向けになって、上へ向けた足をばたばた
}
