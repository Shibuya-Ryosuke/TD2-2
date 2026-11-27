#include "Tutorial.h"
#include "../Game/Game.h"
#include "../Title/Title.h"
#include "../../Camera/StageCamera.h"
#include "../../Config/TutorialConfig.h"
#include "../../Input/GameInput.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Player/Player.h"
#include "../../Stage/Light/SunLight.h"
#include <algorithm>
#include <cmath>
#include <utility>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		bool sEnterFromTitle = false;

		float Smooth(float progress)
		{
			const float t = std::clamp(progress, 0.0f, 1.0f);
			return t * t * (3.0f - 2.0f * t);
		}
	}

	//タイトルから切り替わるときだけ導入の落下を予約する
	void Tutorial::EnterFromTitle()
	{
		sEnterFromTitle = true;
		SceneManager::ChangeScene<Tutorial>();
	}

	//ヘッダーで前方宣言だけしているのでここで作って消す
	Tutorial::Tutorial() = default;
	Tutorial::~Tutorial() = default;

	//カメラとチュートリアルの床と寝ているボスを用意する
	void Tutorial::Initialize()
	{
		//最初に作ったカメラが自動でアクティブになる
		const uint32_t cameraIndex = CameraManager::CreateCamera<StageCamera>("Stage");
		camera_ = CameraManager::GetCamera<StageCamera>(cameraIndex);

		//ゲームシーンと同じ見た目にする
		nearFade_.Disable();
		toneMap_.Apply();
		SunLight::Apply();

		player_ = std::make_unique<Player>();
		player_->Initialize();
		boss_ = std::make_unique<Boss>();
		boss_->Initialize();
		direction_.Initialize();
		sky_.Initialize();
		health_.Reset();
		flow_.Initialize();

		//チュートリアルの床から始めて、ボスは下の1層目で寝ている
		floors_.StartTutorial();
		camera_->SetFloorHeight(floors_.GetCurrent().GetHeight());
		flow_.Begin(MakeContext());
		bossDamage_ = 0;
		isHandedOver_ = false;
		introTime_ = 0.0f;
		introLanding_ = player_->GetPosition();
		isIntroActive_ = std::exchange(sEnterFromTitle, false);
		camera_->ResetShot();
		if (isIntroActive_)
		{
			player_->SetPosition(introLanding_ + Vector3{ 0.0f, TutorialConfig::kIntroHeight, 0.0f });
			player_->SetPose(PlayerPose::Kind::Dangle);
			player_->SetVisible(false);
			ApplyIntroCamera(0.0f);
		}
		UpdateVisuals();
		//タイトルから来たら画面を覆っていた積み木を落とす
		transition_.Initialize(camera_);
	}

	//シーンを出るとき
	void Tutorial::Finalize()
	{
		//ほかのシーンのために元に戻す
		camera_->ResetShot();
		nearFade_.Restore();
		toneMap_.Restore();
	}

	//ポーズとチュートリアルを進めて、終わったらゲームシーンへ
	void Tutorial::Update()
	{
		//場面切り替えの間は操作も止める
		if (transition_.Update() || isHandedOver_)
		{
			UpdateVisuals();
			return;
		}
		if (isIntroActive_)
		{
			UpdateIntro(std::min(GetDeltaTime(), 0.1f));
			UpdateVisuals();
			return;
		}
		if (pauseMenu_.IsOpen())
		{
			UpdatePause();
			UpdateVisuals();
			return;
		}
		if (GameInput::IsPauseTriggered())
		{
			pauseMenu_.Open();
			UpdateVisuals();
			return;
		}

		const PlayerCommand command = GameInput::ReadCommand();
		const TutorialFlow::Result result = flow_.Update(MakeContext(), command, GetDeltaTime());
		bossDamage_ += result.bossDamage;
		UpdateVisuals();
		if (result.isFinished)
		{
			HandOver();
		}
	}

	//プレイヤーとボスの位置とダメージを引き継いで本編へ切り替える
	void Tutorial::HandOver()
	{
		isHandedOver_ = true;
		Game::SetWakeStart(WakeStart{ player_->GetPosition(), boss_->GetPosition(), bossDamage_ });
		Game::SetStartMode(Game::StartMode::AfterTutorial);
		SceneManager::ChangeScene<Game>();
	}

	//積み木が落ちきってからプレイヤーを降ろして着地後にカメラを戻す
	void Tutorial::UpdateIntro(float deltaTime)
	{
		const float before = introTime_;
		introTime_ += deltaTime;
		if (introTime_ < TutorialConfig::kIntroFallTime)
		{
			const float fall = introTime_ / TutorialConfig::kIntroFallTime;
			const float drop = TutorialConfig::kIntroHeight * (1.0f - fall * fall);
			player_->SetVisible(true);
			player_->SetPosition(introLanding_ + Vector3{ 0.0f, drop, 0.0f });
			player_->SetPoseSpin(360.0f * Smooth((fall - 0.24f) / 0.72f));
			ApplyIntroCamera(0.0f);
			return;
		}

		Floor& floor = floors_.GetCurrent();
		if (before < TutorialConfig::kIntroFallTime)
		{
			player_->Place(introLanding_);
			player_->PlayLandingParticles();
			player_->SetVisible(true);
			player_->SetPose(PlayerPose::Kind::Sit);
			floor.Jiggle(floor.GetCellAt(introLanding_), TutorialConfig::kLaunchLandJiggle);
			direction_.Shake(TutorialConfig::kLaunchShake);
		}
		floor.Tick(deltaTime);

		const float landed = introTime_ - TutorialConfig::kIntroFallTime;
		if (landed >= TutorialConfig::kIntroSitTime)
		{
			player_->SetPose(PlayerPose::Kind::Normal);
		}
		const float back = Smooth(landed / TutorialConfig::kIntroPullBackTime);
		ApplyIntroCamera(back);
		if (landed >= TutorialConfig::kIntroPullBackTime)
		{
			camera_->ResetShot();
			isIntroActive_ = false;
		}
	}

	//空から入ってきたプレイヤーを近くで追い、着地後は通常の位置へ戻す
	void Tutorial::ApplyIntroCamera(float pullBack)
	{
		const float close = 1.0f - pullBack;
		const Vector3& feet = player_->GetPosition();
		const float lookHeight = introLanding_.y + TutorialConfig::kIntroHeight - TutorialConfig::kIntroCameraLead;
		const Vector3 target = { feet.x, std::min(feet.y, lookHeight) + TutorialConfig::kIntroFocusHeight, feet.z };
		camera_->SetPitchOffset(TutorialConfig::kIntroPitch * close);
		camera_->SetFocusOffset(camera_->GetCenteringOffset(target) * close);
		camera_->SetOrthoSize(std::lerp(TutorialConfig::kIntroOrthoSize, camera_->GetBaseOrthoSize(), pullBack));
	}

	//ポーズメニューで選んだものに合わせて戻るかシーンを変える
	void Tutorial::UpdatePause()
	{
		PauseMenu::Choice choice = pauseMenu_.Update();
		//TABかメニューボタンでもチュートリアルにもどる
		if (GameInput::IsPauseTriggered())
		{
			choice = PauseMenu::Choice::Resume;
		}
		if (choice == PauseMenu::Choice::None)
		{
			return;
		}
		pauseMenu_.Close();
		if (choice == PauseMenu::Choice::Title)
		{
			transition_.Start<Title>();
		}
		else if (choice == PauseMenu::Choice::Retry)
		{
			transition_.Start<Tutorial>();
		}
	}

	//見た目を確定する
	void Tutorial::UpdateVisuals()
	{
		//下へ落ちる間は落ちる線を出し、揺れはカメラに渡す
		direction_.Update(health_, flow_.IsFalling(), *player_, *boss_, floors_.GetCurrent());
		camera_->SetShakeOffset(direction_.GetShakeOffset());
		action_.UpdatePreview(floors_.GetCurrent(), *player_, !isIntroActive_ && flow_.IsPreviewVisible());
		floors_.UpdateVisuals();
		player_->UpdateVisuals();
		boss_->UpdateVisuals();
		sky_.Update(*camera_);
	}

	//床とキャラと積み木とやることの文字を描く
	void Tutorial::Draw()
	{
		sky_.Draw();
		floors_.Draw();
		player_->Draw();
		boss_->Draw();
		flow_.Draw();
		//いちばん手前に場面切り替えの積み木
		transition_.Draw();
		if (!transition_.IsBusy() && !isIntroActive_)
		{
			flow_.DrawText();
			pauseMenu_.Draw();
		}
	}

	//チュートリアルの流れが動かすもの
	TutorialFlow::Context Tutorial::MakeContext()
	{
		return TutorialFlow::Context{ floors_, *player_, *boss_, *camera_, action_, direction_ };
	}
}
