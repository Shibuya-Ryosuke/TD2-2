#include "SkyDome.h"
#include "../../Camera/StageCamera.h"
#include "../../Config/SkyConfig.h"
#include <cmath>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		constexpr float kPi = 3.14159265f; //円周率
		constexpr float kRadToDeg = 180.0f / kPi; //ラジアンから度へ
		constexpr float kImageAspect = 2.0f; //横が縦の2倍の天球の画像の縦横比
	}

	//いちばん奥に描く光の当たらない天球を作る
	void SkyDome::Initialize()
	{
		model_ = Model::Create(SkyConfig::kSkyDomeModel);
		//深度を書かずにいちばん奥に描いて、光も影もつけない
		model_->SetBackground(true);
		model_->SetEnableLighting(false);
		model_->SetCastShadow(false);
		//カメラについてくるので画面外で省かない
		model_->SetFrustumCulling(false);
		scroll_ = 0.0f;
	}

	//カメラの位置と向きに合わせて置き、雲をゆっくり横に流す
	void SkyDome::Update(const StageCamera& camera, float deltaTime)
	{
		Place(camera.GetViewRotate(), camera.GetViewPosition(), camera.GetOrthoSize(), camera.GetAspectRatio(), deltaTime);
	}

	//遠近法のカメラは天球の奥の深さで見える縦の半分の長さに合わせる
	void SkyDome::UpdatePerspective(const Camera& camera)
	{
		const float halfHeight = SkyConfig::kSkyDomeDepth * std::tan(camera.GetFovY() * 0.5f);
		Place(camera.GetRotate(), camera.GetTranslate(), halfHeight, camera.GetAspectRatio(), GetDeltaTime());
	}

	//カメラの向きと位置と画面の縦の半分の長さと縦横比に合わせて置き、雲を流す
	void SkyDome::Place(const Vector3& rotate, const Vector3& position, float halfHeight, float aspectRatio, float deltaTime)
	{
		//カメラと同じ向きにすると天球の上が画面の上、奥が画面の奥になる
		model_->SetRotate({ rotate.x * kRadToDeg, rotate.y * kRadToDeg, rotate.z * kRadToDeg });
		model_->SetTranslate(position);

		//画面の四すみまで覆う楕円にして、縦にピンクから水色まで映す
		const float halfWidth = halfHeight * aspectRatio;
		const float span = SkyConfig::kSkyDomeSpan;
		model_->SetScale({ halfWidth * span, halfHeight * span, SkyConfig::kSkyDomeDepth });

		//画面に映るのは画像の横のこの割合と縦のこの2倍の割合だけ
		const float visible = std::asin(1.0f / span) / kPi;

		//縦は画像の上から下までが画面に入り、横は画面の縦横比に合わせて引き伸ばされないようにUVを広げる
		const float fill = SkyConfig::kSkyTextureFill;
		const float scaleV = fill / (visible * 2.0f);
		const float scaleU = aspectRatio * fill / (kImageAspect * visible);
		model_->SetUVScale({ scaleU, scaleV });

		//真ん中を画像の真ん中に合わせ、テクスチャを横にずらして雲を流す
		scroll_ = std::fmod(scroll_ + SkyConfig::kSkyScrollSpeed * deltaTime, 1.0f);
		model_->SetUVTranslate({ scroll_, 0.5f - 0.5f * scaleV });
		model_->TransferMatrix();
	}

	//ほかのものより先に描く
	void SkyDome::Draw()
	{
		model_->Draw();
	}
}
