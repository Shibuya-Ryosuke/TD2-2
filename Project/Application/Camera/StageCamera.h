#pragma once
#include <RyoEngine.h>

namespace App
{
	//ステージを斜め上から映す平行投影のカメラ
	class StageCamera : public RyoEngine::Camera
	{
	public:
		StageCamera() = default;
		~StageCamera() override = default;

		StageCamera(const StageCamera&) = delete;
		StageCamera& operator=(const StageCamera&) = delete;

		void Initialize() override; //位置と向きを決める
		void Update() override; //平行投影の行列を毎フレーム作る

		//今映している床の高さに合わせて上下させる
		void SetFloorHeight(float floorHeight) { floorHeight_ = floorHeight; }

		//画面の縦の半分に映す長さ
		void SetOrthoSize(float orthoSize) { orthoSize_ = orthoSize; }
		float GetOrthoSize() const { return orthoSize_; }

		//今の床の高さから求めたこのフレームのカメラの位置
		RyoEngine::Vector3 GetViewPosition() const { return { origin_.x, origin_.y + floorHeight_, origin_.z }; }

	private:
		RyoEngine::Vector3 origin_ = { 15.0f, 24.0f, -23.0f }; //1層目を映すときの位置
		float floorHeight_ = 0.0f; //今映している床の高さ
		float orthoSize_ = 12.6f; //画面の縦の半分に映す長さ
	};
}
