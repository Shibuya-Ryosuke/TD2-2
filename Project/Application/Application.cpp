#include "Application.h"
#include "Scene/SceneSetup.h"
#include "Direction/Player/PlayerParticles.h"
#include "Object/Boss/Phase/BossPhaseTable.h"

using namespace RyoEngine;

namespace App
{
	void Application::Initialize()
	{
		//ImGuiで調整した値の保存先を設定して読み込む
		const std::string jsonFolder = "Resources/ApplicationResources/Json/";
		ParamEditor::SetFolderPath(jsonFolder);
		LightManager::SetFolderPath(jsonFolder);
		ParticleManager::SetFolderPath(jsonFolder);
		BossPhaseTable::SetFolderPath(jsonFolder);
		ParamEditor::Load();
		LightManager::Load();
		ParticleManager::Load();
		PlayerParticles::Initialize();
		BossPhaseTable::Load();

		//シーンを登録して起動時のシーンを決める
		SetupScenes();

		//ブルームとトーンマッピングを最初からオンにする
		PostProcess::SetBloomEnabled(true);
		PostProcess::SetACESEnabled(true);
	}

	void Application::Finalize()
	{
	}
}
