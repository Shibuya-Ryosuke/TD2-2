#include "StageCamera.h"

using namespace RyoEngine;

namespace App
{
	//位置と向きをImGuiで調整できるようにする
	void StageCamera::Initialize()
	{
		//縦横比などの初期値はエンジンに任せる
		Camera::Initialize();

		//原点を見下ろす向き
		rotate_ = { 0.7183f, -0.5779f, 0.0f };
		nearZ_ = 0.3f;
		farZ_ = 100.0f;

		ParamEditor::BeginGroup("StageCamera");
		ParamEditor::RegisterValue("位置 (1層目)", &origin_, 0.1f, -100.0f, 100.0f);
		ParamEditor::RegisterValue("向き (ラジアン)", &rotate_, 0.01f, -3.14f, 3.14f);
		ParamEditor::RegisterValue("映す広さ (縦の半分)", &orthoSize_, 0.1f, 1.0f, 100.0f);
		ParamEditor::EndGroup();

		Update();
	}

	//平行投影でカメラの行列を毎フレーム作る
	void StageCamera::Update()
	{
		//行列を直接指定されている間は自動で作らない
		if (useCustomMatrices_)
		{
			return;
		}

		//今映している床の高さの分だけ上下する
		translate_ = GetViewPosition();

		//カメラのワールド行列の逆行列がビュー行列
		const Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, rotate_, translate_);
		viewMatrix_ = Inverse(cameraMatrix);

		//平行投影のプロジェクション行列
		const float halfHeight = orthoSize_;
		const float halfWidth = orthoSize_ * aspectRatio_;
		projectionMatrix_ = MakeOrthographicMatrix(-halfWidth, halfHeight, halfWidth, -halfHeight, nearZ_, farZ_);

		viewProjectionMatrix_ = viewMatrix_ * projectionMatrix_;

		//画面外のモデルを省くための6平面を作り直す
		UpdateFrustumPlanes();
	}
}
