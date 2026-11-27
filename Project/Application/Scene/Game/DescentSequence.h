#pragma once
#include <RyoEngine.h>

namespace App
{
	class Floor;
	class Boss;

	//ボスが切り離された床と一緒に下の層の着地点へ落ちる動き
	class DescentSequence
	{
	public:
		//ボスの位置から、下の層のlandingへ落ち始める
		void Begin(const RyoEngine::Vector3& bossPosition, const RyoEngine::Vector3& landing);

		//1フレーム進めて、着いたフレームだけtrueを返す
		bool Advance(float deltaTime, Floor& current, Boss& boss);

	private:
		RyoEngine::Vector3 dropOrigin_{}; //ボスが落ち始めた位置
		RyoEngine::Vector3 landing_{}; //下の層の着地点
		bool isBossLanded_ = false; //着いたか
		float timer_ = 0.0f; //落ち始めてからの秒数
	};
}
