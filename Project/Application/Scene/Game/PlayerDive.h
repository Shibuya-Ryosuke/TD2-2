#pragma once
#include <RyoEngine.h>

namespace App
{
	class Player;

	//ためた足場にボヨンと飛ばされたプレイヤーが、穴の上まで山なりに飛んで、じたばたくるくる下の層へ落ちる
	class PlayerDive
	{
	public:
		//fromから穴の上のholeへ飛んで、下の層のlandingへ落ち始める
		void Begin(const RyoEngine::Vector3& from, const RyoEngine::Vector3& hole, const RyoEngine::Vector3& landing, Player& player);
		//1フレーム進めて、着いたフレームだけtrueを返す
		bool Update(float deltaTime, Player& player);
		//飛ばされて落ちている途中か
		bool IsActive() const { return isActive_; }
		//カメラで追うプレイヤーの今の足元
		const RyoEngine::Vector3& GetPosition() const { return position_; }

	private:
		RyoEngine::Vector3 from_{}; //飛ばされる前の足元
		RyoEngine::Vector3 hole_{}; //穴の上の、落ち始める所
		RyoEngine::Vector3 landing_{}; //下の層の着地点
		RyoEngine::Vector3 position_{}; //今の足元
		float timer_ = 0.0f; //飛ばされてからの秒数
		bool isActive_ = false; //飛ばされて落ちている途中か
	};
}
