#pragma once
#include <RyoEngine.h>

namespace App
{
	class Boss;
	class StageCamera;

	//やられ演出でボスがプレイヤーと画面の上で重なったら、すーっと透かす
	class DefeatBossFade
	{
	public:
		//透かす前に戻す
		void Reset(Boss& boss);
		//canStartの間に重なったら透かし始め、一度透かし始めたら最後まで進める
		void Update(float deltaTime, bool canStart, const StageCamera& camera, const RyoEngine::Vector3& player, Boss& boss);

	private:
		//カメラから見てボスとプレイヤーが画面の上で重なっているか
		static bool IsCovering(const StageCamera& camera, const RyoEngine::Vector3& player, const RyoEngine::Vector3& boss);

		bool isFading_ = false; //透かし始めたか
		float elapsed_ = 0.0f; //透かし始めてからの秒数
	};
}
