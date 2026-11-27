#pragma once
#include <RyoEngine.h>

namespace App::PlayerParticles
{
	//着地と勝利に使うパーティクルを登録する
	void Initialize();
	//着地した足元からピンクの光を出す
	void Landing(const RyoEngine::Vector3& feet);
	//肩の外側に白い小さな星を出す
	void VictoryStars(const RyoEngine::Vector3& origin);
}
