#include "GameHud.h"
#include "../Game/GameConfig.h"
#include <RyoEngine.h>
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	// デバッグ表示 (HP・アイテム・お知らせ・操作)
	void GameHud::Draw(const GameHudInfo& info)
	{
		// HP・層・ボスの HP
		PrintText("HP {}/{}   LAYER {}   BOSS HP {}/{}", { 20.0f, 20.0f },
			info.hearts, GameConfig::kMaxHearts, info.layer + 1, info.bossHealth, GameConfig::kBossMaxHealth);

		// 持っているアイテム
		const char* bombText = "-";
		if (info.hasBomb)
		{
			bombText = "BOMB (E / B)";
		}
		const char* jumpText = "-";
		if (info.hasJumpBoost)
		{
			jumpText = "JUMP+";
		}
		PrintText("ITEM  {}   {}", { 20.0f, 50.0f }, bombText, jumpText);

		// 場面ごとのお知らせ
		const Vector2 noticePosition = { 20.0f, 80.0f };
		if (info.isPaused)
		{
			PrintText("PAUSE  (ESC / START)", noticePosition);
		}
		else if (info.phase == GamePhase::Waiting)
		{
			PrintText("PRESS A DIRECTION TO START", noticePosition);
		}
		else if (info.phase == GamePhase::Escape)
		{
			PrintText("SEPARATED! GET TO SAFE GROUND  {:.1f}", noticePosition, std::max(0.0f, info.escapeTimer));
		}
		else if (info.phase == GamePhase::Descent)
		{
			PrintText("JUMP TO THE NEXT LAYER", noticePosition);
		}
		else if (info.phase == GamePhase::LandingImpact)
		{
			PrintText("FALL DAMAGE {}", noticePosition, GameConfig::kBossFallDamage);
		}
		else if (info.phase == GamePhase::TileBarrage)
		{
			PrintText("TILES {}   BONUS DAMAGE {}", noticePosition, info.barrageTiles, info.barrageDamage);
		}

		// 操作は画面のいちばん下 (文字の位置はゲーム画面の解像度が基準。今は 1920 x 1080)
		const float bottomY = static_cast<float>(DirectXCommon::GetGameHeight()) - 40.0f;
		PrintText("MOVE WASD/LSTICK   FACE ARROWS/RSTICK/DPAD   BREAK SPACE/A   BOMB E/B   PAUSE ESC/START", { 20.0f, bottomY });
	}
}
