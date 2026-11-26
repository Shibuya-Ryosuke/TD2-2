#pragma once
#include <RyoEngine.h>

namespace App
{
	class Player;

	//トランポリンのように足場にボン！と飛ばされて、山なりに決めた所へ戻る
	class PlayerLaunch
	{
	public:
		//今の位置からtargetへ飛ばし始める
		void Begin(const RyoEngine::Vector3& from, const RyoEngine::Vector3& target);
		//1フレーム進めて、着いたらtrue
		bool Update(float deltaTime, Player& player);
		bool IsActive() const { return isActive_; }

	private:
		RyoEngine::Vector3 from_{};
		RyoEngine::Vector3 target_{};
		float timer_ = 0.0f;
		bool isActive_ = false;
	};
}
