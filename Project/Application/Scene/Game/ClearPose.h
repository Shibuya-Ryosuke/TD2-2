#pragma once
#include <RyoEngine.h>

namespace App
{
	class Player;
	class StageCamera;

	//クリアのときにカメラをプレイヤーへ寄せて、カメラの方を向いたポーズを見せる
	class ClearPose
	{
	public:
		//カメラの方を向かせて寄せ始める
		void Start(StageCamera& camera, Player& player);
		//寄せてからポーズをして少し待ち、終わったらtrue
		bool Update(float deltaTime, StageCamera& camera, Player& player);

	private:
		float elapsed_ = 0.0f; //寄せ始めてからの秒数
		float startOrthoSize_ = 0.0f; //寄せ始める前の映す広さ
		bool isPosing_ = false; //寄せ終わってポーズを始めたか
		float holdElapsed_ = 0.0f; //ポーズが終わってからの秒数
	};
}
