#include "TutorialFlow.h"
#include "../../Config/TutorialConfig.h"
#include <algorithm>

using namespace RyoEngine;

//チュートリアルのやることと残りの数と押すボタンのデバッグ表示
namespace App
{
	//やることと残りの数と押すボタンを出す
	void TutorialFlow::DrawText() const
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
			PrintText("BOYOING!!  BACK TO THE START", subPosition);
		}
		if (isRepairing_)
		{
			PrintText("BOYOING!!  BACK TO THE START  THE FLOOR IS FIXED!", subPosition);
		}
		if (rejectTimer_ > 0.0f)
		{
			PrintText("BOING!  YOU CAN'T BREAK THAT ONE. BREAK THE GLOWING TILE!", subPosition);
		}

		//段階ごとのやること
		if (step_ == Step::Move && !hasPendingStep_)
		{
			PrintText("TUTORIAL 1/5   MOVE 5 TILES  {}/{}   WASD / LEFT STICK", linePosition,
				std::min(movedCount_, TutorialConfig::kMoveCount), TutorialConfig::kMoveCount);
		}
		else if (step_ == Step::Break && !hasPendingStep_)
		{
			PrintText("TUTORIAL 2/5   BREAK 3 TILES WITH THE HAMMER  {}/{}   SPACE / A / RT", linePosition,
				breakCount_, TutorialConfig::kBreakCount);
		}
		else if (step_ == Step::BlockOneFall || step_ == Step::BlockTwoFall)
		{
			PrintText("LOOK UP!  A BLOCK IS FALLING!", linePosition);
		}
		else if (step_ == Step::BlockOneDrop && !hasPendingStep_)
		{
			PrintText("TUTORIAL 3/5   DROP THE BLOCK!   BREAK THE GLOWING TILE UNDER IT", linePosition);
		}
		else if (step_ == Step::Reshape)
		{
			PrintText("THE FLOOR IS CHANGING...", linePosition);
		}
		else if (step_ == Step::BlockTwoCut || step_ == Step::IslandFall)
		{
			PrintText("TUTORIAL 4/5   CUT OFF THE BLOCK'S ISLAND!   BREAK THE GLOWING TILE", linePosition);
			if (rejectTimer_ <= 0.0f && !isRescuing_)
			{
				PrintText("A SMALLER ISLAND CUT OFF FROM A BIGGER ONE FALLS DOWN", subPosition);
			}
		}
		else if (step_ == Step::GoDown && !hasPendingStep_)
		{
			PrintText("TUTORIAL 5/5   JUMP INTO A HOLE OR OFF THE EDGE TO GO DOWN!", linePosition);
		}
		else if (step_ == Step::Finale)
		{
			PrintText("THE REMAINING TILES FLY TO THE BOSS!", linePosition);
		}
	}
}
