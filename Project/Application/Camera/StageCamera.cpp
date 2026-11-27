#include "StageCamera.h"
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//位置と向きと広さをImGuiで調整できるようにする
	void StageCamera::Initialize()
	{
		//縦横比などの初期値はエンジンに任せる
		Camera::Initialize();
		nearZ_ = 0.3f;
		farZ_ = 100.0f;

		//ImGuiの値はシーンをまたいで残るので、ゲーム中に書きかえない値だけを登録する
		ParamEditor::BeginGroup("StageCamera");
		ParamEditor::RegisterValue("位置 (1層目)", &origin_, 0.1f, -100.0f, 100.0f);
		ParamEditor::RegisterValue("向き (ラジアン)", &baseRotate_, 0.01f, -3.14f, 3.14f);
		ParamEditor::RegisterValue("映す広さ (縦の半分)", &baseOrthoSize_, 0.1f, 1.0f, 100.0f);
		ParamEditor::EndGroup();

		//前のシーンで寄せたままにならないように、いつもの映し方から始める
		ResetShot();
		Update();
	}

	//寄せる・ずらす・傾ける・揺らすを全部やめる
	void StageCamera::ResetShot()
	{
		focusOffset_ = { 0.0f, 0.0f, 0.0f };
		zoomOrthoSize_ = 0.0f;
		pitchOffset_ = 0.0f;
		shakeOffset_ = { 0.0f, 0.0f, 0.0f };
		yawOffset_ = 0.0f;
		orbitPivot_ = { 0.0f, 0.0f, 0.0f };
	}

	//寄せていなければいつもの広さ
	float StageCamera::GetOrthoSize() const
	{
		if (zoomOrthoSize_ > 0.0f)
		{
			return zoomOrthoSize_;
		}
		return baseOrthoSize_;
	}

	//床の高さとずらした分の位置を中心のまわりに回り込ませて、揺れを足したカメラの位置
	Vector3 StageCamera::GetViewPosition() const
	{
		Vector3 position =
		{
			origin_.x + focusOffset_.x,
			origin_.y + floorHeight_ + focusOffset_.y,
			origin_.z + focusOffset_.z
		};
		//向きと同じ回し方で中心から見た位置を回すと、中心は画面の同じ所に残る
		if (std::abs(yawOffset_) > 0.0f)
		{
			const Vector3 fromPivot = TransformVector3(position - orbitPivot_, MakeRotateMatrix({ 0.0f, yawOffset_, 0.0f }));
			position = orbitPivot_ + fromPivot;
		}
		return position + shakeOffset_;
	}

	//いつもの向きに縦の傾きを足す
	Vector3 StageCamera::GetViewRotate() const
	{
		return { baseRotate_.x + pitchOffset_, baseRotate_.y + yawOffset_, baseRotate_.z };
	}

	//傾けた分と回り込んだ分も入れた正面の向き
	Vector3 StageCamera::GetViewForward() const
	{
		return GetOrbitForward(yawOffset_);
	}

	//縦の傾きは今のままで、yawOffsetだけ横へ回り込んだときの正面の向き
	Vector3 StageCamera::GetOrbitForward(float yawOffset) const
	{
		const Vector3 rotate = { baseRotate_.x + pitchOffset_, baseRotate_.y + yawOffset, baseRotate_.z };
		return Normalize(TransformVector3({ 0.0f, 0.0f, 1.0f }, MakeRotateMatrix(rotate)));
	}

	//ずらす前の位置から見て、回り込む前の正面の向きに垂直な分だけずらせばその点が真ん中に来る
	Vector3 StageCamera::GetCenteringOffset(const Vector3& target) const
	{
		const Vector3 cameraPosition = { origin_.x, origin_.y + floorHeight_, origin_.z };
		const Vector3 forward = GetOrbitForward(0.0f);
		const Vector3 toTarget = target - cameraPosition;
		const float depth = toTarget.x * forward.x + toTarget.y * forward.y + toTarget.z * forward.z;
		return toTarget - forward * depth;
	}

	//平行投影でカメラの行列を毎フレーム作る
	void StageCamera::Update()
	{
		//行列を直接指定されている間は自動で作らない
		if (useCustomMatrices_)
		{
			return;
		}

		//今映している床の高さの分だけ上下して、傾けた分も向きに入れる
		translate_ = GetViewPosition();
		rotate_ = GetViewRotate();

		//カメラのワールド行列の逆行列がビュー行列
		const Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, rotate_, translate_);
		viewMatrix_ = Inverse(cameraMatrix);

		//平行投影のプロジェクション行列
		const float halfHeight = GetOrthoSize();
		const float halfWidth = halfHeight * aspectRatio_;
		projectionMatrix_ = MakeOrthographicMatrix(-halfWidth, halfHeight, halfWidth, -halfHeight, nearZ_, farZ_);

		viewProjectionMatrix_ = viewMatrix_ * projectionMatrix_;

		//画面外のモデルを省くための6平面を作り直す
		UpdateFrustumPlanes();
	}

	//ワールドの位置を1920x1080が基準のデバッグ表示の画面の位置にする
	Vector2 StageCamera::WorldToScreen(const Vector3& world) const
	{
		const Matrix4x4& m = GetViewProjectionMatrix();
		const float x = world.x * m.m[0][0] + world.y * m.m[1][0] + world.z * m.m[2][0] + m.m[3][0];
		const float y = world.x * m.m[0][1] + world.y * m.m[1][1] + world.z * m.m[2][1] + m.m[3][1];
		float w = world.x * m.m[0][3] + world.y * m.m[1][3] + world.z * m.m[2][3] + m.m[3][3];
		if (std::abs(w) < 0.0001f)
		{
			w = 1.0f;
		}
		const float width = static_cast<float>(DirectXCommon::GetGameWidth());
		const float height = static_cast<float>(DirectXCommon::GetGameHeight());
		return { (x / w * 0.5f + 0.5f) * width, (0.5f - y / w * 0.5f) * height };
	}
}
