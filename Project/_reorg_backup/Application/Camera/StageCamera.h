#pragma once
#include <RyoEngine.h>

namespace App
{
	// ステージを斜め上から映すカメラ (平行投影。遠くのものも小さくならない)。
	// エンジンの Camera を継承して、プロジェクション行列だけ平行投影に変えている。
	// 位置・向き・映す広さの初期値は Unity のプロトタイプと同じ。
	// 下の層へ降りるときは SetFloorHeight で、床の高さの分だけ一緒に下がる。
	class StageCamera : public RyoEngine::Camera
	{
	public:
		StageCamera() = default;
		~StageCamera() override = default;

		StageCamera(const StageCamera&) = delete;
		StageCamera& operator=(const StageCamera&) = delete;

		void Initialize() override; // カメラの位置・向き
		void Update() override;     // 平行投影で行列を作る (CameraManagerが毎フレーム呼ぶ)

		// 今映している床の高さ (カメラはその分だけ上下する)
		void SetFloorHeight(float floorHeight) { floorHeight_ = floorHeight; }

		// 画面の縦の半分に映す長さ
		void SetOrthoSize(float orthoSize) { orthoSize_ = orthoSize; }
		float GetOrthoSize() const { return orthoSize_; }

		// このフレームのカメラの位置 (Update を待たずに、今の床の高さから求める)
		RyoEngine::Vector3 GetViewPosition() const { return { origin_.x, origin_.y + floorHeight_, origin_.z }; }

	private:
		RyoEngine::Vector3 origin_ = { 15.0f, 24.0f, -23.0f }; // 1層目 (高さ 0) を映すときの位置
		float floorHeight_ = 0.0f;                              // 今映している床の高さ
		float orthoSize_ = 12.6f;                               // 画面の縦の半分に映す長さ
	};
}
