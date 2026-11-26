#include "Title.h"
#include "../Tutorial/Tutorial.h"
#include "../../Config/TitleConfig.h"
#include "../../Input/GameInput.h"
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		//ゆっくり始まってゆっくり止まる進み具合
		float Smooth(float t)
		{
			const float c = std::clamp(t, 0.0f, 1.0f);
			return c * c * (3.0f - 2.0f * c);
		}

		//aからbへtだけ近づけた位置
		Vector3 Mix(const Vector3& a, const Vector3& b, float t)
		{
			return a + (b - a) * t;
		}
	}

	//カメラと空と床とプレイヤーを用意する
	void Title::Initialize()
	{
		stage_.Initialize(TitleConfig::kEye, TitleConfig::kTarget);
		drop_.Initialize();
		isTransitionStarted_ = false;
		transition_.Initialize(stage_.GetCamera());
	}

	//ACESとNearFadeを元に戻す
	void Title::Finalize()
	{
		stage_.Finalize();
	}

	//EnterかAで積み木を落として、床ごと落ちたらチュートリアルへ
	void Title::Update()
	{
		const float deltaTime = GetDeltaTime();
		const TitleDrop::Result result = drop_.Update(deltaTime);
		if (result.isRumbleNow)
		{
			stage_.RumbleFloor(drop_.GetFeet());
		}
		if (result.isFloorDropNow)
		{
			stage_.DropFloor(drop_.GetFeet());
		}
		UpdateCamera();
		stage_.Update(deltaTime);

		//演出の間は操作させない
		if (transition_.Update())
		{
			return;
		}
		if (!drop_.IsStarted())
		{
			if (GameInput::IsEnterTriggered())
			{
				drop_.Start();
			}
			return;
		}

		//落ちていくところで場面切り替えの積み木がふさぐ
		if (!isTransitionStarted_ && drop_.GetStartTime() >= TitleConfig::kTransitionTime)
		{
			isTransitionStarted_ = true;
			transition_.Start<Tutorial>();
		}
	}

	//ゆらゆらは引き、落としたら顔と積み木に寄り、がたがたで床ごと引いて、落ちたら見下ろしながら追う
	void Title::UpdateCamera()
	{
		Vector3 eye = TitleConfig::kEye;
		Vector3 target = TitleConfig::kTarget;
		if (drop_.IsStarted())
		{
			const float time = drop_.GetStartTime();
			const Vector3& start = TitleConfig::kPlayer;

			//あせる顔と、はねる積み木のあいだに寄る
			const float close = Smooth(time / TitleConfig::kCloseTime);
			const Vector3 face = start + Vector3{ 0.0f, TitleConfig::kFaceHeight, 0.0f };
			const Vector3 faceAndBlock = face + (drop_.GetBlockPosition() - face) * 0.15f;
			eye = Mix(eye, TitleConfig::kCloseEye, close);
			target = Mix(target, faceAndBlock, close);

			//床ががたがたし始めたら、床ごと見える引きにする
			const float wide = Smooth((time - TitleConfig::kRumbleTime) / TitleConfig::kWideTime);
			eye = Mix(eye, TitleConfig::kWideEye, wide);
			target = Mix(target, start + Vector3{ 0.0f, TitleConfig::kWideHeight, 0.0f }, wide);

			//落ち始めたら見る所はプレイヤーにぴったり、カメラは少し遅れて下りて見下ろしていく
			const float fallStart = TitleConfig::kFloorDropTime + TitleConfig::kPlayerFallDelay;
			const float follow = Smooth((time - fallStart) / 0.6f);
			eye = Mix(eye, TitleConfig::kFallEye, follow);
			target = Mix(target, drop_.GetFeet() + Vector3{ 0.0f, TitleConfig::kFollowHeight, 0.0f }, follow);
			eye.y += std::min(drop_.GetFeet().y, 0.0f) * TitleConfig::kFollowRatio;
		}
		stage_.LookAt(eye, target);
	}

	//床とプレイヤーと文字を描く
	void Title::Draw()
	{
		stage_.Draw();
		drop_.Draw();

		//1920x1080が基準の画面の左上あたり
		const float width = static_cast<float>(DirectXCommon::GetGameWidth());
		const float height = static_cast<float>(DirectXCommon::GetGameHeight());
		PrintText("ASHIBA GA OCHIRU GAME", { width * 0.08f, height * 0.12f });
		if (!drop_.IsStarted())
		{
			PrintText("PRESS ENTER / A", { width * 0.08f, height * 0.17f });
		}

		//いちばん手前に場面切り替えの積み木
		transition_.Draw();
	}
}
