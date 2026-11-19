#pragma once
#include <RyoEngine.h>

// 各シーン
#include "Title/Title.h"
#include "Game/Game.h"
#include "Clear/Clear.h"
#include "GameOver/GameOver.h"

namespace App
{
	// シーンの登録と、起動時のシーンの指定 (Application::Initialize から1回だけ呼ぶ)
	// シーンを増やしたら、ここに RegisterScene を1行足す
	// 流れ: タイトル →(Enter / A)→ ゲーム →(クリア / ゲームオーバー)→(Enter / A)→ タイトル
	inline void SetupScenes()
	{
		// 各シーンの登録 (ImGui のシーン一覧には、クラス名で出る)
		RyoEngine::SceneManager::RegisterScene<Title>();
		RyoEngine::SceneManager::RegisterScene<Game>();
		RyoEngine::SceneManager::RegisterScene<Clear>();
		RyoEngine::SceneManager::RegisterScene<GameOver>();

		// 起動時のシーン
		RyoEngine::SceneManager::SetDefaultScene<Title>();
	}
}
