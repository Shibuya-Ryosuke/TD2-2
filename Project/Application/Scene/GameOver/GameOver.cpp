#include "GameOver.h"
#include "../Title/Title.h"
#include "../Game/Game.h"
#include "../../Config/GameOverConfig.h"
#include "../../Config/ShowcaseConfig.h"
#include "../../Input/GameInput.h"

using namespace RyoEngine;

namespace App
{
	//カメラと空と床とプレイヤーと積み木を用意する
	void GameOver::Initialize()
	{
		stage_.Initialize(GameOverConfig::kOverEye, GameOverConfig::kOverTarget);
		bury_.Initialize();
		menuTimer_ = 0.0f;
		transition_.Initialize(stage_.GetCamera());
	}

	//ACESを元に戻す
	void GameOver::Finalize()
	{
		stage_.Finalize();
	}

	//埋もれる演出を進めて、埋もれたら選んだ方へ積み木で切り替える
	void GameOver::Update()
	{
		stage_.Update(GetDeltaTime());
		bury_.Update(GetDeltaTime());

		//演出の間と、埋もれてメニューが出るまでは操作させない
		if (transition_.Update())
		{
			return;
		}
		if (bury_.IsBuried())
		{
			menuTimer_ += GetDeltaTime();
		}
		if (menuTimer_ < GameOverConfig::kMenuDelay)
		{
			return;
		}
		const GameOverMenu::Choice choice = menu_.Update();
		if (choice == GameOverMenu::Choice::Retry)
		{
			//やり直しはチュートリアルを飛ばしてボス戦の最初から
			Game::SetStartMode(Game::StartMode::Battle);
			transition_.Start<Game>();
		}
		else if (choice == GameOverMenu::Choice::Title)
		{
			transition_.Start<Title>();
		}
	}

	//積み木とプレイヤーと文字を描く
	void GameOver::Draw()
	{
		stage_.Draw();
		bury_.Draw();

		//埋もれてから1920x1080が基準の画面の真ん中あたりにメニュー
		if (!transition_.IsBusy() && menuTimer_ >= GameOverConfig::kMenuDelay)
		{
			menu_.Draw();
		}

		//いちばん手前に場面切り替えの積み木
		transition_.Draw();
	}
}
