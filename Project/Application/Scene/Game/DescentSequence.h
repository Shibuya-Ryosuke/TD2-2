#pragma once
#include <RyoEngine.h>

namespace App
{
	class Floor;
	class Boss;
	class StageCamera;

	//ボスが切り離された床と下の層へ落ちて、カメラは上の層と下の層の両方が見える高さまで下がる流れ
	class DescentSequence
	{
	public:
		//このフレームで起きたこと
		struct Result
		{
			bool isBossLandedNow = false; //ボスが下の層に着いて落下ダメージを入れる
			bool isFinished = false; //ボスが着いてカメラも下がりきった
		};

		//ボスの位置から落ち始める
		void Begin(const RyoEngine::Vector3& bossPosition);

		//今の層と下の層を使って1フレーム進める
		Result Advance(float deltaTime, Floor& current, const Floor& below, Boss& boss, StageCamera& camera);

	private:
		RyoEngine::Vector3 dropOrigin_{}; //ボスが落ち始めた位置
		bool isBossLanded_ = false;
		float timer_ = 0.0f;
	};
}
