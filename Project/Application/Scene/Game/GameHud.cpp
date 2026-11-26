#include "GameHud.h"
#include "BossFallFlow.h"
#include "../../Config/GameConfig.h"
#include "../../Stage/Floor/LayerFloors.h"
#include "../../Object/Player/PlayerHealth.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Boss/Phase/BossPhaseTable.h"
#include <RyoEngine.h>
#include <algorithm>

using namespace RyoEngine;

namespace App
{
	//ゲームシーンの状態から出すものを集める
	GameHudInfo GameHud::MakeInfo(const PlayerHealth& health, const LayerFloors& floors, const Boss& boss,
		GamePhase phase, bool isPaused, const BossFallFlow& bossFall)
	{
		GameHudInfo info{};
		info.hearts = health.GetHearts();
		info.layer = floors.GetLayer();
		info.bossHealth = boss.GetHp();
		info.bossMaxHealth = boss.GetMaxHp();
		info.bossPhase = boss.GetPhaseController().GetIndex();
		info.bossPhaseCount = static_cast<int32_t>(BossPhaseTable::GetPhases().size());
		info.phase = phase;
		info.isPaused = isPaused;
		info.escapeTimer = bossFall.GetEscapeTimer();
		info.barrageTiles = bossFall.GetBarrage().GetTileCount();
		info.barrageDamage = bossFall.GetBarrage().GetDamageApplied();
		//[アイテム] 持っているアイテムはGameから入れていたのでコメントアウト
		// info.hasBomb = items_.HasBomb();
		// info.hasJumpBoost = player_->HasJumpBoost();
		return info;
	}

	//デバッグ表示
	void GameHud::Draw(const GameHudInfo& info)
	{
		//HPと層とボスのHP
		PrintText("HP {}/{}   LAYER {}   BOSS HP {}/{}   FORM {}/{}", { 20.0f, 20.0f },
			info.hearts, GameConfig::kMaxHearts, info.layer + 1, info.bossHealth, info.bossMaxHealth,
			info.bossPhase + 1, info.bossPhaseCount);

		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// //持っているアイテム
		// const char* bombText = "-";
		// if (info.hasBomb)
		// {
			// bombText = "BOMB (E / B)";
		// }
		// const char* jumpText = "-";
		// if (info.hasJumpBoost)
		// {
			// jumpText = "JUMP+";
		// }
		// PrintText("ITEM  {}   {}", { 20.0f, 50.0f }, bombText, jumpText);

		//ポーズ中はポーズメニューが出るのでお知らせは出さない
		if (!info.isPaused)
		{
			DrawNotice(info);
		}

		//操作は画面のいちばん下
		const float bottomY = static_cast<float>(DirectXCommon::GetGameHeight()) - 40.0f;
		PrintText("MOVE WASD/LSTICK   FACE ARROWS/RSTICK/DPAD   BREAK FRONT SPACE/A   PAUSE TAB/MENU", { 20.0f, bottomY });
	}

	//場面ごとのお知らせ
	void GameHud::DrawNotice(const GameHudInfo& info)
	{
		const Vector2 noticePosition = { 20.0f, 80.0f };
		if (info.phase == GamePhase::Escape)
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
			PrintText("TILES {}   TILE DAMAGE {}", noticePosition, info.barrageTiles, info.barrageDamage);
		}
	}
}
