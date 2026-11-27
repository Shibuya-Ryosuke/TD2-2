#pragma once
#include <RyoEngine.h>

namespace App
{
	class StageCamera;

	//ボスが落ちる流れで、ボスへ寄る、両方の層を映す、プレイヤーについていく、いつもに戻すを、前の映し方からなめらかにつなぐ
	class BossFallCamera
	{
	public:
		//今のカメラの床の高さを基準にして、今の映し方からつなぎ始める
		void Begin(const StageCamera& camera);
		//今の映し方からつなぎ直す
		void Cut(const StageCamera& camera);
		//targetを画面のまん中に映して寄る映し方へ、tの割合だけ近づける
		void Focus(StageCamera& camera, const RyoEngine::Vector3& target, float orthoSize, float t);
		//上の層と下の層の両方を映す引いた映し方へ、tの割合だけ近づける
		void Wide(StageCamera& camera, float upperHeight, float lowerHeight, float t);
		//heightの層のいつもの映し方へ、tの割合だけ近づける
		void Normal(StageCamera& camera, float height, float t);
		//ずらした分を消して、heightの層のいつもの映し方にする
		void Finish(StageCamera& camera, float height);

	private:
		//今の床の高さのまま、映す所をずらした分と広さをtの割合だけ近づける
		void Blend(StageCamera& camera, const RyoEngine::Vector3& offset, float orthoSize, float t);

		float baseHeight_ = 0.0f; //流れの間ずっと使うカメラの床の高さ
		RyoEngine::Vector3 fromOffset_{}; //つなぎ始めの映す所をずらした分
		float fromOrthoSize_ = 0.0f; //つなぎ始めの映す広さ
	};
}
