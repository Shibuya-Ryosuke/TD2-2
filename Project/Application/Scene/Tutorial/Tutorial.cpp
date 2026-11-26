#include "Tutorial.h"
#include "../Game/Game.h"
#include "../Title/Title.h"
#include "../../Camera/StageCamera.h"
#include "../../Input/GameInput.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Player/Player.h"

using namespace RyoEngine;

namespace App
{
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
		UpdateVisuals();
		//タイトルから来たら積み木が吹き飛ぶ演出から始まる
		transition_.Initialize(camera_);
	}

	//シーンを出るとき
	void Tutorial::Finalize()
	{
		//ほかのシーンのために元に戻す
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

	//プレイヤーとボスの位置とダメージをゲームシーンへ渡して、同じ見た目のまますぐ切り替える
	void Tutorial::HandOver()
	{
		isHandedOver_ = true;
		Game::SetWakeStart(WakeStart{ player_->GetPosition(), boss_->GetPosition(), bossDamage_ });
		Game::SetStartMode(Game::StartMode::AfterTutorial);
		SceneManager::ChangeScene<Game>();
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
		action_.UpdatePreview(floors_.GetCurrent(), *player_, flow_.IsPreviewVisible());
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
		flow_.DrawText();
		pauseMenu_.Draw();
	}

	//チュートリアルの流れが動かすもの
	TutorialFlow::Context Tutorial::MakeContext()
	{
		return TutorialFlow::Context{ floors_, *player_, *boss_, *camera_, action_, direction_ };
	}
}
