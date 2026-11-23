#pragma once
#include <RyoEngine.h>

namespace App
{
	class Floor;
	class Player;
	class Boss;
	class StageCamera;

	//ボスが落ちて、プレイヤーとカメラも下の層へ降りる流れ
	class DescentSequence
	{
	public:
		//このフレームで起きたこと
		struct Result
		{
			bool isBossLandedNow = false; //ボスが下の層に着いて落下ダメージを入れる
			bool isFinished = false; //ボスもプレイヤーも着いた
		};

		//ボスとプレイヤーの位置と降りる先を決めて降り始める
		void Begin(const RyoEngine::Vector3& bossPosition, const RyoEngine::Vector3& playerStart, const RyoEngine::Vector3& playerEnd);

		//今の層と下の層を使って1フレーム進める
		Result Advance(float deltaTime, Floor& current, const Floor& below, Player& player, Boss& boss, StageCamera& camera);

	private:
		RyoEngine::Vector3 dropOrigin_{}; //ボスが落ち始めた位置
		RyoEngine::Vector3 descentStart_{}; //プレイヤーが降り始めた位置
		RyoEngine::Vector3 descentEnd_{}; //プレイヤーが降りる先
		bool isBossLanded_ = false;
		float timer_ = 0.0f;
	};
}
