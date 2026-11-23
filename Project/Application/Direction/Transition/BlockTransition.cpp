#include "BlockTransition.h"
#include "../../Camera/StageCamera.h"
#include "../../Config/GameConfig.h"
#include "../../Config/DirectionConfig.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		//前のシーンが画面を埋めてから切り替えたか
		bool sIsCovered = false;

		constexpr float kRadToDeg = 180.0f / std::numbers::pi_v<float>;
	}

	//遠近法のカメラで使う
	void BlockTransition::Initialize(const Camera* camera)
	{
		stageCamera_ = nullptr;
		view_.isOrthographic = false;
		view_.refDepth = DirectionConfig::kTransitionDepth;
		view_.halfHeight = DirectionConfig::kTransitionDepth * std::tan(camera->GetFovY() * 0.5f);
		Setup(camera);
	}

	//ゲームシーンの平行投影のカメラで使う
	void BlockTransition::Initialize(const StageCamera* camera)
	{
		stageCamera_ = camera;
		view_.isOrthographic = true;
		view_.refDepth = DirectionConfig::kTransitionDepth;
		view_.halfHeight = camera->GetOrthoSize();
		Setup(camera);
	}

	//ブロックとシェーダーを用意する
	void BlockTransition::Setup(const Camera* camera)
	{
		camera_ = camera;
		view_.aspectRatio = camera->GetAspectRatio();

		//紙吹雪も積み木もTransitionBlockシェーダーを使う
		const uint32_t blockShader = ShaderManager::Load(DirectionConfig::kTransitionBlockShader);
		debris_.Initialize(&frame_, blockShader);
		wall_.Initialize(&frame_, blockShader);
		phase_ = Phase::Idle;

		//前のシーンが埋めて来たら吹き飛ぶ演出を始める
		if (sIsCovered)
		{
			sIsCovered = false;
			BeginBlow();
		}
	}

	//がれきから始める
	void BlockTransition::BeginDebris()
	{
		phase_ = Phase::Debris;
		timer_ = 0.0f;
		UpdateFrame();
		debris_.Begin(random_, view_);
	}

	//吹き飛ぶ演出と横風を始める
	void BlockTransition::BeginBlow()
	{
		phase_ = Phase::Blow;
		timer_ = 0.0f;
		UpdateFrame();
		wall_.BeginBlow(random_, view_);
		windEffect_ = PostEffectManager::Add(ShaderManager::LoadPostEffect(DirectionConfig::kGustLinesShader));
		SetGust(0.0f);
	}

	//吹き飛び終わったら片付けて操作できるようにする
	void BlockTransition::FinishBlow()
	{
		phase_ = Phase::Idle;
		wall_.Hide();
		if (windEffect_ != PostEffectManager::kInvalidEffect)
		{
			PostEffectManager::Remove(windEffect_);
			windEffect_ = PostEffectManager::kInvalidEffect;
		}
	}

	//演出を進める
	bool BlockTransition::Update()
	{
		if (phase_ == Phase::Idle)
		{
			return false;
		}
		//読み込み直後の長い1フレームで一気に進まないようにする
		timer_ += std::min(GetDeltaTime(), DirectionConfig::kTransitionMaxDeltaTime);
		UpdateFrame();

		if (phase_ == Phase::Debris)
		{
			//がれきが落ちきったら積み上げる
			debris_.Update(timer_, view_);
			if (timer_ >= DirectionConfig::kTransitionDebrisTime)
			{
				debris_.Hide();
				phase_ = Phase::Stack;
				timer_ = 0.0f;
			}
		}
		if (phase_ == Phase::Stack)
		{
			//積み上がったら少し待つ
			wall_.UpdateStack(timer_, view_);
			if (timer_ >= DirectionConfig::kTransitionStackTime)
			{
				phase_ = Phase::Hold;
				timer_ = 0.0f;
			}
		}
		if (phase_ == Phase::Hold)
		{
			//埋まったら次のシーンへ行き、最後の積み木のぽよよんは続ける
			wall_.UpdateStack(DirectionConfig::kTransitionStackTime + timer_, view_);
			if (timer_ >= DirectionConfig::kTransitionHoldTime && changeScene_ != nullptr)
			{
				sIsCovered = true;
				changeScene_();
				changeScene_ = nullptr;
			}
		}
		if (phase_ == Phase::Blow)
		{
			//横に吹き飛び終わったら次のフレームから操作できる
			wall_.UpdateBlow(timer_, view_);
			const float progress = std::clamp(timer_ / DirectionConfig::kTransitionBlowTime, 0.0f, 1.0f);
			SetGust(std::sin(std::numbers::pi_v<float> * std::sqrt(progress)));
			if (timer_ >= DirectionConfig::kTransitionBlowTime)
			{
				FinishBlow();
			}
		}
		return true;
	}

	//ブロックの親をカメラの位置と向きに合わせる
	void BlockTransition::UpdateFrame()
	{
		//ゲームシーンは今の床の高さからこのフレームのカメラの位置を出す
		Vector3 position = camera_->GetTranslate();
		if (stageCamera_ != nullptr)
		{
			position = stageCamera_->GetViewPosition();
			view_.halfHeight = stageCamera_->GetOrthoSize(); //ImGuiで広さを変えても合わせる
		}
		const Vector3 rotate = camera_->GetRotate();
		frame_.SetLocal({ { 1.0f, 1.0f, 1.0f }, { rotate.x * kRadToDeg, rotate.y * kRadToDeg, rotate.z * kRadToDeg }, position });
		frame_.UpdateMatrix();
	}

	//横風の強さ
	void BlockTransition::SetGust(float strength)
	{
		if (windEffect_ != PostEffectManager::kInvalidEffect)
		{
			PostEffectManager::SetParam(windEffect_, 0, strength * DirectionConfig::kGustLinesStrength);
		}
	}

	//ブロックを描く
	void BlockTransition::Draw()
	{
		if (phase_ == Phase::Debris)
		{
			debris_.Draw();
		}
		if (phase_ == Phase::Stack || phase_ == Phase::Hold || phase_ == Phase::Blow)
		{
			wall_.Draw();
		}
	}
}
