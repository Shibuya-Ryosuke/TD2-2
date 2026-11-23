#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <memory>
#include <vector>
#include "../../Object/Boss/BossPlan.h"
#include "../../Object/Boss/BossTuning.h"

namespace App
{
	class Boss;
	class Floor;

	//ボスの次の何手かを半透明のボスがくり返し見せる残像
	class BossGhost
	{
	public:
		BossGhost() = default;
		~BossGhost() = default;

		BossGhost(const BossGhost&) = delete;
		BossGhost& operator=(const BossGhost&) = delete;

		void Initialize(); //半透明の体と拳を作る

		//先読みを作り直して残像を進める
		void Update(const Boss& boss, const Floor& floor, const RyoEngine::Vector3& player, const BossTuning& tuning, bool isVisible);
		void Draw();

	private:
		//1つの行動を見せる秒数
		static float GetShowTime(const BossPlannedAction& action);
		//見せている行動に合わせて体と拳を置く
		void Pose(const BossPlannedAction& action, float t, const BossTuning& tuning);

		std::unique_ptr<RyoEngine::Model> body_{};
		std::unique_ptr<RyoEngine::Model> fist_{};
		std::vector<BossPlannedAction> plan_; //先読みした行動
		float elapsed_ = 0.0f; //くり返し見せている時間
		float refreshTimer_ = 0.0f; //次に先読みを作り直すまでの時間
		int32_t lastActionCount_ = -1; //前に先読みしたときのボスの行動の数
		const Floor* lastFloor_ = nullptr; //前に先読みしたときの床
		bool isVisible_ = false;
	};
}
