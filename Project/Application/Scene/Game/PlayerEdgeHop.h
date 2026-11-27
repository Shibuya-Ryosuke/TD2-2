#pragma once
#include <RyoEngine.h>
#include <cstdint>

namespace App
{
	class Player;

	//ボスが落ちた穴のふちまで、決まった秒数の中で速くぴょんぴょん跳んでいき、ふちでぐっとためる
	class PlayerEdgeHop
	{
	public:
		//fromからedgeまで、1マスがtileSizeの床の上を跳び始める
		void Begin(const RyoEngine::Vector3& from, const RyoEngine::Vector3& edge, float tileSize, Player& player);
		//1フレーム進めて、ためきったフレームだけtrueを返す
		bool Update(float deltaTime, Player& player);

	private:
		RyoEngine::Vector3 from_{}; //跳び始める足元
		RyoEngine::Vector3 edge_{}; //穴のふちの足元
		int32_t count_ = 0; //跳ぶ回数で、0ならためるだけ
		float timer_ = 0.0f; //跳び始めてからの秒数
		bool isActive_ = false; //跳んでいる途中か
	};
}
