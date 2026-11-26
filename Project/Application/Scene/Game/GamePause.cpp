#include "Game.h"
#include "../Title/Title.h"
#include "../../Input/GameInput.h"

//Gameのうちポーズメニューで選んだあとの切り替え
namespace App
{
	//ポーズメニューで選んだものに合わせて戻るかシーンを変える
	void Game::UpdatePause()
	{
		PauseMenu::Choice choice = pauseMenu_.Update();
		//TABかメニューボタンでもゲームにもどる
		if (GameInput::IsPauseTriggered())
		{
			choice = PauseMenu::Choice::Resume;
		}
		if (choice == PauseMenu::Choice::None)
		{
			return;
		}
		pauseMenu_.Close();
		//タイトルとリトライは積み木で画面を埋めてから切り替える
		if (choice == PauseMenu::Choice::Title)
		{
			transition_.Start<Title>();
		}
		else if (choice == PauseMenu::Choice::Retry)
		{
			SetStartMode(StartMode::Battle);
			transition_.Start<Game>();
		}
	}
}
