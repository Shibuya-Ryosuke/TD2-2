#pragma once
#include <RyoEngine.h>

//各シーン
#include "Title/Title.h"
#include "Game/Game.h"
#include "Clear/Clear.h"
#include "GameOver/GameOver.h"

namespace App
{
	//シーンを登録して起動時のシーンを決める
	//シーンを増やしたらRegisterSceneを1行足す
	//タイトルからゲーム、クリアかゲームオーバーからタイトルへ
	inline void SetupScenes()
	{
		//ImGuiのシーン一覧にクラス名で出る各シーンの登録
		RyoEngine::SceneManager::RegisterScene<Title>();
		RyoEngine::SceneManager::RegisterScene<Game>();
		RyoEngine::SceneManager::RegisterScene<Clear>();
		RyoEngine::SceneManager::RegisterScene<GameOver>();

		//起動時のシーン
		RyoEngine::SceneManager::SetDefaultScene<Title>();
	}
}
