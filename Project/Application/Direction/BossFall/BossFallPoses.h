#pragma once
#include "BossPose.h"

//ボスが落ちて叩きつけられ、痛そうに座りこんでから起き上がるまでの姿勢
namespace App::BossFallPoses
{
	BossPose NoticePose(float time); //足元がなくなって「えっ」と下を見て、手を少し広げてぷるぷる
	BossPose FlailPose(float time); //落ちながら両手を上へばたばたさせて、足もじたばた
	BossPose SlamPose(); //叩きつけられた瞬間で、お尻から落ちて手足が投げ出される
	BossPose HurtSitPose(float time); //座りこんで片手で頭を押さえ、もう片手を後ろにつく
	BossPose StandPose(); //いつもの立ち姿
}
