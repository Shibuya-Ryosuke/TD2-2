#pragma once
#include <RyoEngine.h>
#include <cstdint>

namespace App
{
	class Boss;
	class Floor;
	class GameDirection;

	//チュートリアルからゲームシーンへ渡す、寝ているボスとプレイヤーの位置と足場のダメージ
	struct WakeStart
	{
		RyoEngine::Vector3 playerPosition{}; //尻もちのあと立っている位置
		RyoEngine::Vector3 bossPosition{}; //仰向けに寝ているボスの足元
		int32_t bossDamage = 0; //残った足場で当てたダメージ
	};

	//チュートリアルのあと、仰向けに寝ていたボスがはっと飛び起きてボス戦が始まるまで
	class BossWake
	{
	public:
		//寝ているボスの位置から始める
		void Begin(const RyoEngine::Vector3& bossRest);
		//1フレーム進めて、起きて着地したらtrue
		bool Update(float deltaTime, Floor& floor, Boss& boss, GameDirection& direction);

	private:
		RyoEngine::Vector3 bossRest_{}; //寝ているボスの足元
		float timer_ = 0.0f; //始めてからの秒数
	};
}
