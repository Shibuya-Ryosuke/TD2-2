#pragma once
#include <RyoEngine.h>
#include "AttackBase.h"
#include "../BossTuning.h"
#include "../../../Stage/Floor/Floor.h"
#include "../../../Stage/Grid/Cell.h"

#include <vector>

namespace App {
	class Rush : public AttackBase {
	public:

		void Start(const Floor& floor, const RyoEngine::Vector3& bossPos, const RyoEngine::Vector3& target, const BossTuning& tuning) override;
		int32_t Update(const Floor& floor, const RyoEngine::Vector3& bossPos, const RyoEngine::Vector3& target, const BossTuning& tuning, float deltaTime) override;

		// ターゲットまで一直線の道を作成
		static float BuildRoute(const Floor& floor, const RyoEngine::Vector3& bossPos, const RyoEngine::Vector3& targetPos,
			float maxDistance, float bodyRadius, std::vector<Cell>& route);

		//突進中のボスの位置
		const RyoEngine::Vector3& GetPosition() const { return position_; }
		//進む向き
		const RyoEngine::Vector3& GetForward() const { return forward_; }
		//警告を出すマス
		const std::vector<Cell>& GetRoute() const { return route_; }
		//警告を出すマスの真ん中の位置
		const std::vector<RyoEngine::Vector3>& GetRouteCenters() const { return routeCenters_; }
		//1マスの大きさ
		const RyoEngine::Vector2& GetTileSize() const { return tileSize_; }

		void Cancel() override;
	private:
		//route_のマスの位置と大きさを覚える
		void StoreRouteCenters(const Floor& floor);

	private:
		RyoEngine::Vector3 origin_{};   //突進を始めた位置
		RyoEngine::Vector3 forward_{};  //進む向き
		RyoEngine::Vector3 position_{}; //今の位置
		float reach_ = 0.0f;            //進める距離
		float traveled_ = 0.0f;         //進んだ距離
		bool hasHit_ = false;           //この突進でもう当たったか

		std::vector<Cell> route_{}; // 通るルートの保持

		std::vector<RyoEngine::Vector3> routeCenters_; //通るマスの真ん中の位置
		RyoEngine::Vector2 tileSize_{};                //1マスの大きさ
	};
}

