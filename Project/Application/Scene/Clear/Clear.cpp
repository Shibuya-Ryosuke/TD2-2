#include "Clear.h"
#include "../Title/Title.h"
#include "../../Config/ClearConfig.h"
#include "../../Input/GameInput.h"

using namespace RyoEngine;

namespace App
{
	//カメラと空と床とプレイヤーとボスを用意する
	void Clear::Initialize()
	{
		stage_.Initialize(ClearConfig::kWideEye, ClearConfig::kWideTarget);
		show_.Initialize();
		transition_.Initialize(stage_.GetCamera());
	}

	//ACESとNearFadeを元に戻す
	void Clear::Finalize()
	{
		stage_.Finalize();
	}

	//流れを進めて、踊り始めたらEnterかAでタイトルへ
	void Clear::Update()
	{
		const float deltaTime = GetDeltaTime();
		show_.Update(deltaTime);
		Vector3 eye{};
		Vector3 target{};
		show_.GetShot(eye, target);
		stage_.LookAt(eye, target);
		stage_.Update(deltaTime);

		//演出の間と、踊り始めるまでは操作させない
		if (transition_.Update())
		{
			return;
		}
		if (show_.IsDancing() && GameInput::IsEnterTriggered())
		{
			transition_.Start<Title>();
		}
	}

	//床とキャラと文字を描く
	void Clear::Draw()
	{
		stage_.Draw();
		show_.Draw();

		//踊り始めたら1920x1080が基準の画面の左寄りにタイトルへ戻るメニュー
		if (show_.IsDancing())
		{
			const float width = static_cast<float>(DirectXCommon::GetGameWidth());
			const float height = static_cast<float>(DirectXCommon::GetGameHeight());
			PrintText("GAME CLEAR", { width * 0.12f, height * 0.38f });
			PrintText("> BACK TO TITLE   ENTER / A", { width * 0.12f, height * 0.44f });
		}

		//いちばん手前に場面切り替えの積み木
		transition_.Draw();
	}
}
