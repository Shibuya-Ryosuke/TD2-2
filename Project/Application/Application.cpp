#include "Application.h"
#include "Scene/SceneSetup.h"

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
		ParamEditor::Load();
		LightManager::Load();
		ParticleManager::Load();

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
