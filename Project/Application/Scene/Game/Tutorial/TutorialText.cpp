#include "TutorialFlow.h"
#include "../../../Camera/StageCamera.h"
#include "../../../Config/TutorialConfig.h"
#include "../../../Object/Boss/Boss.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

//チュートリアルのやることと残りの数とボスの寝言のデバッグ表示
namespace App
{
	namespace
	{
		//ワールドの位置を1920x1080が基準の画面の位置へ
		Vector2 ToScreen(const StageCamera& camera, const Vector3& world)
		{
			const Matrix4x4& m = camera.GetViewProjectionMatrix();
			const float x = world.x * m.m[0][0] + world.y * m.m[1][0] + world.z * m.m[2][0] + m.m[3][0];
			const float y = world.x * m.m[0][1] + world.y * m.m[1][1] + world.z * m.m[2][1] + m.m[3][1];
			float w = world.x * m.m[0][3] + world.y * m.m[1][3] + world.z * m.m[2][3] + m.m[3][3];
			if (std::abs(w) < 0.0001f)
			{
				w = 1.0f;
			}
			const float width = static_cast<float>(DirectXCommon::GetGameWidth());
			const float height = static_cast<float>(DirectXCommon::GetGameHeight());
			return { (x / w * 0.5f + 0.5f) * width, (0.5f - y / w * 0.5f) * height };
		}
	}

	//やることと残りの数とボスの寝言を出す
	void TutorialFlow::DrawText(const StageCamera& camera, const Boss& boss) const
	{
		const float width = static_cast<float>(DirectXCommon::GetGameWidth());
		const float height = static_cast<float>(DirectXCommon::GetGameHeight());
		const Vector2 linePosition = { width * 0.3f, height * 0.1f };
		const Vector2 subPosition = { width * 0.3f, height * 0.1f + 30.0f };

		//やり終わったらCLEAR!
		if (clearTimer_ > 0.0f)
		{
			PrintText("CLEAR!!  NICE!!", { width * 0.45f, height * 0.4f });
		}
		if (isRescuing_)
		{
			PrintText("OOPS!  BOING! BACK TO THE START", subPosition);
		}

		//段階ごとのやること
		if (step_ == Step::Move && !hasPendingStep_)
		{
			PrintText("TUTORIAL 1/5   MOVE 5 TILES  {}/{}   WASD / LEFT STICK", linePosition,
				std::min(movedCount_, TutorialConfig::kMoveCount), TutorialConfig::kMoveCount);
		}
		else if (step_ == Step::Break && !hasPendingStep_)
		{
			PrintText("TUTORIAL 2/5   BREAK 3 TILES WITH THE HAMMER  {}/{}   SPACE / A", linePosition,
				breakCount_, TutorialConfig::kBreakCount);
		}
		else if (step_ == Step::BlockOneFall || step_ == Step::BlockTwoFall)
		{
			PrintText("LOOK UP!  A BLOCK IS FALLING!", linePosition);
		}
		else if (step_ == Step::BlockOneDrop && !hasPendingStep_)
		{
			PrintText("TUTORIAL 3/5   DROP THE BLOCK!   BREAK THE TILE UNDER IT", linePosition);
		}
		else if (step_ == Step::Reshape)
		{
			PrintText("BOING!  THE FLOOR IS CHANGING...", linePosition);
		}
		else if (step_ == Step::BlockTwoCut || step_ == Step::IslandFall)
		{
			PrintText("TUTORIAL 4/5   CUT OFF THE BLOCK'S ISLAND!   BREAK 1 TILE TO SEPARATE IT", linePosition);
			PrintText("A SMALLER ISLAND CUT OFF FROM A BIGGER ONE FALLS DOWN", subPosition);
		}
		else if (step_ == Step::GoDown && !hasPendingStep_)
		{
			PrintText("TUTORIAL 5/5   JUMP INTO A HOLE OR OFF THE EDGE TO GO DOWN!", linePosition);
		}
		else if (step_ == Step::Finale)
		{
			PrintText("THE REMAINING TILES FLY TO THE BOSS!", linePosition);
		}

		//寝ているボスの頭の上に寝言
		const char* bossText = finale_.GetBossText();
		if (step_ != Step::Finale)
		{
			bossText = "Zzz...";
		}
		if (bossText != nullptr)
		{
			const Vector3 head = boss.GetPosition() + Vector3{ 0.0f, 2.5f, 0.0f };
			PrintText("{}", ToScreen(camera, head), bossText);
		}
	}
}
