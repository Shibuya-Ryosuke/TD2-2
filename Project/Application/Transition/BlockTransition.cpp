#include "BlockTransition.h"
#include "../Camera/StageCamera.h"
#include "../Config/GameConfig.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		// 前のシーンが画面を埋めてから切り替えたか (次のシーンは倒れる演出から始める)
		bool sIsCovered = false;

		constexpr float kRadToDeg = 180.0f / std::numbers::pi_v<float>;
	}

	// 遠近法のカメラ
	void BlockTransition::Initialize(const Camera* camera)
	{
		stageCamera_ = nullptr;
		view_.isOrthographic = false;
		view_.refDepth = GameConfig::kTransitionDepth;
		view_.halfHeight = GameConfig::kTransitionDepth * std::tan(camera->GetFovY() * 0.5f);
		Setup(camera);
	}

	// ゲームシーンの平行投影のカメラ
	void BlockTransition::Initialize(const StageCamera* camera)
	{
		stageCamera_ = camera;
		view_.isOrthographic = true;
		view_.refDepth = GameConfig::kTransitionDepth;
		view_.halfHeight = camera->GetOrthoSize();
		Setup(camera);
	}

	// ブロック・シェーダーを用意する
	void BlockTransition::Setup(const Camera* camera)
	{
		camera_ = camera;
		view_.aspectRatio = camera->GetAspectRatio();

		const uint32_t blockShader = ShaderManager::Load(GameConfig::kTransitionBlockShader);
		debris_.Initialize(&frame_, blockShader);
		wall_.Initialize(&frame_, blockShader);
		phase_ = Phase::Idle;

		// 前のシーンが画面を埋めて来た: 埋まった画面から倒れる演出を始める
		if (sIsCovered)
		{
			sIsCovered = false;
			BeginTopple();
		}
	}

	// がれきから始める
	void BlockTransition::BeginDebris()
	{
		phase_ = Phase::Debris;
		timer_ = 0.0f;
		UpdateFrame();
		debris_.Begin(random_, view_);
	}

	// 倒れる演出を始める (風の筋も出す)
	void BlockTransition::BeginTopple()
	{
		phase_ = Phase::Topple;
		timer_ = 0.0f;
		UpdateFrame();
		wall_.BeginTopple(random_, view_);
		windEffect_ = PostEffectManager::Add(ShaderManager::LoadPostEffect(GameConfig::kWindStreakShader));
		SetWindStreak(0.0f);
	}

	// 倒れ終わった: 全部片付けて、操作できるようにする
	void BlockTransition::FinishTopple()
	{
		phase_ = Phase::Idle;
		wall_.Hide();
		if (windEffect_ != PostEffectManager::kInvalidEffect)
		{
			PostEffectManager::Remove(windEffect_);
			windEffect_ = PostEffectManager::kInvalidEffect;
		}
	}

	// 演出を進める
	bool BlockTransition::Update()
	{
		if (phase_ == Phase::Idle)
		{
			return false;
		}
		// シーンを読み込んだ直後は1フレームが長くなるので、一気に進みすぎないようにする
		timer_ += std::min(GetDeltaTime(), GameConfig::kTransitionMaxDeltaTime);
		UpdateFrame();

		if (phase_ == Phase::Debris)
		{
			// がれき → 積み木
			debris_.Update(timer_, view_);
			if (timer_ >= GameConfig::kTransitionDebrisTime)
			{
				debris_.Hide();
				phase_ = Phase::Stack;
				timer_ = 0.0f;
			}
		}
		if (phase_ == Phase::Stack)
		{
			// 積み上がる → 埋まったら少し待つ
			wall_.UpdateStack(timer_, view_);
			if (timer_ >= GameConfig::kTransitionStackTime)
			{
				phase_ = Phase::Hold;
				timer_ = 0.0f;
			}
		}
		if (phase_ == Phase::Hold)
		{
			// 埋まった → 次のシーンへ (次のシーンは倒れる演出から始まる)。最後の積み木のぽよよんは続ける
			wall_.UpdateStack(GameConfig::kTransitionStackTime + timer_, view_);
			if (timer_ >= GameConfig::kTransitionHoldTime && changeScene_ != nullptr)
			{
				sIsCovered = true;
				changeScene_();
				changeScene_ = nullptr;
			}
		}
		if (phase_ == Phase::Topple)
		{
			// 風で倒れる → 倒れ終わったら操作できる (このフレームはまだ操作させない)
			wall_.UpdateTopple(timer_, view_);
			const float progress = std::clamp(timer_ / GameConfig::kTransitionToppleTime, 0.0f, 1.0f);
			SetWindStreak(std::sin(std::numbers::pi_v<float> * std::sqrt(progress)));
			if (timer_ >= GameConfig::kTransitionToppleTime)
			{
				FinishTopple();
			}
		}
		return true;
	}

	// ブロックの親をカメラの位置・向きに合わせる
	void BlockTransition::UpdateFrame()
	{
		// ゲームシーンのカメラは、今の床の高さからこのフレームの位置を求める (Update を待たない)
		Vector3 position = camera_->GetTranslate();
		if (stageCamera_ != nullptr)
		{
			position = stageCamera_->GetViewPosition();
			view_.halfHeight = stageCamera_->GetOrthoSize(); // ImGui で広さを変えても合うように
		}
		const Vector3 rotate = camera_->GetRotate();
		frame_.SetLocal({ { 1.0f, 1.0f, 1.0f }, { rotate.x * kRadToDeg, rotate.y * kRadToDeg, rotate.z * kRadToDeg }, position });
		frame_.UpdateMatrix();
	}

	// 風の筋の強さ (0 〜 1)
	void BlockTransition::SetWindStreak(float strength)
	{
		if (windEffect_ != PostEffectManager::kInvalidEffect)
		{
			PostEffectManager::SetParam(windEffect_, 0, strength * GameConfig::kWindStreakStrength);
		}
	}

	// ブロックを描く
	void BlockTransition::Draw()
	{
		if (phase_ == Phase::Debris)
		{
			debris_.Draw();
		}
		if (phase_ == Phase::Stack || phase_ == Phase::Hold || phase_ == Phase::Topple)
		{
			wall_.Draw();
		}
	}
}
