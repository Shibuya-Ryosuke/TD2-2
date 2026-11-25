#include "SkyDome.h"
#include "../../Camera/StageCamera.h"
#include "../../Config/DirectionConfig.h"
#include <cmath>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		constexpr float kPi = 3.14159265f; //円周率
		constexpr float kRadToDeg = 180.0f / kPi; //ラジアンから度へ
		constexpr float kDegToRad = kPi / 180.0f; //度からラジアンへ
	}

	//いちばん奥に描く光の当たらない天球を作る
	void SkyDome::Initialize()
	{
		model_ = Model::Create(DirectionConfig::kSkyDomeModel);
		//深度を書かずにいちばん奥に描いて、光も影もつけない
		model_->SetBackground(true);
		model_->SetEnableLighting(false);
		model_->SetCastShadow(false);
		//カメラについてくるので画面外で省かない
		model_->SetFrustumCulling(false);
		//遠近感のあるカメラで見上げた空を描くシェーダー
		model_->SetShader(ShaderManager::Load(DirectionConfig::kSkyDomeShader));
		turnDeg_ = 0.0f;
	}

	//カメラの位置と向きに合わせて画面を覆い、天球をゆっくり回す
	void SkyDome::Update(const StageCamera& camera)
	{
		//カメラと同じ向きと位置で画面の四すみまで覆う楕円を描く板にする
		const Vector3& rotate = camera.GetRotate();
		model_->SetRotate({ rotate.x * kRadToDeg, rotate.y * kRadToDeg, rotate.z * kRadToDeg });
		model_->SetTranslate(camera.GetViewPosition());
		const float halfHeight = camera.GetOrthoSize();
		const float halfWidth = halfHeight * camera.GetAspectRatio();
		const float span = DirectionConfig::kSkyDomeSpan;
		model_->SetScale({ halfWidth * span, halfHeight * span, DirectionConfig::kSkyDomeDepth });
		model_->TransferMatrix();

		//天球をゆっくり回して雲が空を横切るようにする
		turnDeg_ = std::fmod(turnDeg_ + DirectionConfig::kSkyTurnSpeed * GetDeltaTime(), 360.0f);

		//シェーダーに画面の大きさと見上げるカメラの視野と角度と回転を渡す
		const float screenWidth = static_cast<float>(DirectXCommon::GetGameWidth());
		const float screenHeight = static_cast<float>(DirectXCommon::GetGameHeight());
		const float tanHalfFov = std::tan(DirectionConfig::kSkyFovYDeg * 0.5f * kDegToRad);
		const float pitch = DirectionConfig::kSkyLookUpDeg * kDegToRad;
		model_->SetShaderParams(0, { screenWidth, screenHeight, tanHalfFov, pitch });
		model_->SetShaderParams(1, { turnDeg_ / 360.0f, 0.0f, 0.0f, 0.0f });
	}

	//ほかのものより先に描く
	void SkyDome::Draw()
	{
		model_->Draw();
	}
}
