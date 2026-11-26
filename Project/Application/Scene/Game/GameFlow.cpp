#include "Game.h"
#include "../../Config/BossConfig.h"
#include "../../Camera/StageCamera.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Player/Player.h"
#include <algorithm>

using namespace RyoEngine;

//Gameのうちチュートリアルの続きと穴に落ちたときの流れ
namespace App
{
	//チュートリアルと同じ所に寝ているボスとプレイヤーを置いて、足場のダメージを入れる
	void Game::StartAfterTutorial()
	{
		floors_.Start(0);
		floors_.EnsureBelow();
		camera_->SetFloorHeight(floors_.GetCurrent().GetHeight());
		battle_.Reset();
		health_.ClearInvincible();
		player_->Place(wakeStart_.playerPosition);
		player_->SetVisible(true);
		boss_->SetActive(true);
		boss_->Place(floors_.GetCurrent(), wakeStart_.bossPosition);
		boss_->SetSleep(1.0f);
		//チュートリアルで当てたダメージを入れる。ここで倒れないように1は残す
		boss_->Damage(std::min(wakeStart_.bossDamage, boss_->GetHp() - 1));
		bossWake_.Begin(wakeStart_.bossPosition);
		phase_ = GamePhase::BossWake;
	}

	//ボスが起きて着地したらボス戦を始める
	void Game::UpdateBossWake(float deltaTime)
	{
		floors_.GetCurrent().Tick(deltaTime);
		if (!bossWake_.Update(deltaTime, floors_.GetCurrent(), *boss_, direction_))
		{
			return;
		}
		StartBattle();
	}

	//穴に落ちて下の層へ行く流れを進めて、ボスが降りてきたら少し無敵で再開
	void Game::UpdatePlayerFall(float deltaTime)
	{
		if (!playerFall_.Update(deltaTime, floors_, *player_, *boss_, *camera_, direction_))
		{
			return;
		}
		camera_->ResetShot();
		battle_.Reset();
		health_.StartInvincible();
		StartBattle();
	}
}
