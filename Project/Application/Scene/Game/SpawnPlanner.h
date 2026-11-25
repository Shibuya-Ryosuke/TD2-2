#pragma once
#include <RyoEngine.h>
#include <cstdint>

namespace App
{
	class Floor;

	//層が始まるときにボスとプレイヤーを反対側の離れた所に置く位置を決める
	class SpawnPlanner
	{
	public:
		//上から落ちてきた位置をもとに、プレイヤーを反対側に置けるボスの位置
		static RyoEngine::Vector3 FindBossPosition(const Floor& floor, const RyoEngine::Vector3& fallingFrom);

		//ボスの反対側で決めたマス数より離れたプレイヤーの位置
		static RyoEngine::Vector3 FindPlayerPosition(const Floor& floor, const RyoEngine::Vector3& bossPosition);

	private:
		//プレイヤーを反対側に置けたらtrueで、置けないときはいちばん遠いマスをbestに入れる
		static bool TryFindPlayerPosition(const Floor& floor, const RyoEngine::Vector3& bossPosition, RyoEngine::Vector3& best);

		//床の真ん中に落ちてきたときに画面の奥へずらしたボスの位置
		static RyoEngine::Vector3 MoveAwayFromCenter(const Floor& floor, const RyoEngine::Vector3& landing);

		//床の真ん中のマスの位置
		static RyoEngine::Vector3 GetCenterPosition(const Floor& floor);

		//ボスとプレイヤーの体が重ならない近さ
		static float GetContactSpacing();

		//カメラの右と画面の上を地面に平らにした向き
		static void GetScreenAxes(RyoEngine::Vector3& right, RyoEngine::Vector3& up);

		//ボスもプレイヤーも置ける床か
		static bool CanStand(const Floor& floor, int32_t x, int32_t z);
	};
}
