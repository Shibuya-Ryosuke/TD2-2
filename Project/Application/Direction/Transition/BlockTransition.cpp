#include "BlockTransition.h"
#include "../../Camera/StageCamera.h"
#include "../../Config/DirectionConfig.h"
#include <algorithm>
#include <cmath>
#include <numbers>
#include <optional>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		//切り替え先で積み木に覆われた画面から演出を続ける
		std::optional<uint32_t> sCoveredSeed;
		constexpr float kRadToDeg = 180.0f / std::numbers::pi_v<float>;
	}

	void BlockTransition::Initialize(const Camera* camera)
	{
		stageCamera_ = nullptr;
		view_.isOrthographic = false;
		view_.refDepth = DirectionConfig::kTransitionDepth;
		Setup(camera);
	}

	void BlockTransition::Initialize(const StageCamera* camera)
	{
		stageCamera_ = camera;
		view_.isOrthographic = true;
		view_.refDepth = DirectionConfig::kTransitionOrthoDepth;
		Setup(camera);
	}

	//積み木と紙吹雪を用意し、切り替え先では全面を覆った状態から始める
	void BlockTransition::Setup(const Camera* camera)
	{
		camera_ = camera;
		const uint32_t blockShader = ShaderManager::Load(DirectionConfig::kTransitionBlockShader);
		debris_.Initialize(&frame_, blockShader);
		wall_.Initialize(&frame_, blockShader);
		phase_ = Phase::Idle;
		timer_ = 0.0f;
		changeScene_ = nullptr;

		if (sCoveredSeed.has_value())
		{
			phase_ = Phase::Drop;
			wall_.Begin(*sCoveredSeed);
			sCoveredSeed.reset();
			UpdateFrame();
			wall_.UpdateDrop(0.0f, view_);
		}
	}

	void BlockTransition::Start(void (*changeScene)())
	{
		if (IsBusy())
		{
			return;
		}
		changeScene_ = changeScene;
		BeginCover();
	}

	void BlockTransition::BeginCover()
	{
		phase_ = Phase::Cover;
		timer_ = 0.0f;
		coverSeed_ = static_cast<uint32_t>(random_());
		wall_.Begin(coverSeed_);
		UpdateFrame();
		debris_.Begin(random_, view_);
	}

	bool BlockTransition::Update()
	{
		if (!IsBusy())
		{
			return false;
		}

		//読み込み直後の長いフレームで演出を飛ばさない
		timer_ += std::min(GetDeltaTime(), DirectionConfig::kTransitionMaxDeltaTime);
		UpdateFrame();

		if (phase_ == Phase::Cover)
		{
			wall_.UpdateCover(std::min(timer_, DirectionConfig::kTransitionCoverTime), view_);
			debris_.Update(timer_);
			if (timer_ >= DirectionConfig::kTransitionCoverTime && changeScene_ != nullptr)
			{
				sCoveredSeed = coverSeed_;
				changeScene_();
				changeScene_ = nullptr;
			}
		}
		else
		{
			wall_.UpdateDrop(timer_, view_);
			if (timer_ >= DirectionConfig::kTransitionDropTime)
			{
				wall_.Hide();
				phase_ = Phase::Idle;
			}
		}
		return true;
	}

	//カメラの位置と画角に合わせて手前の積み木を配置する
	void BlockTransition::UpdateFrame()
	{
		view_.aspectRatio = camera_->GetAspectRatio();
		Vector3 position = camera_->GetTranslate();
		Vector3 rotate = camera_->GetRotate();
		float scale = DirectionConfig::kTransitionNearScale;
		if (stageCamera_ != nullptr)
		{
			position = stageCamera_->GetViewPosition();
			rotate = stageCamera_->GetViewRotate();
			view_.halfHeight = stageCamera_->GetOrthoSize();
			scale = 1.0f;
		}
		else
		{
			view_.halfHeight = view_.refDepth * std::tan(camera_->GetFovY() * 0.5f);
		}
		frame_.SetLocal({ { scale, scale, scale }, rotate * kRadToDeg, position });
		frame_.UpdateMatrix();
	}

	void BlockTransition::Draw()
	{
		if (!IsBusy())
		{
			return;
		}
		UpdateFrame();
		if (phase_ == Phase::Cover)
		{
			debris_.Draw(view_);
		}
		wall_.Draw(view_);
	}
}
