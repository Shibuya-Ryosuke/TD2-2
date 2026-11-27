#include "Game.h"
#include "../../Camera/StageCamera.h"
#include "../../Config/PlayerConfig.h"
#include "../../Object/Boss/Boss.h"
#include "../../Object/Player/Player.h"
#include "SpawnPlanner.h"

using namespace RyoEngine;

//Gameのうちボスが切り離されて下の層へ落ちていく流れ
namespace App
{
	//ボスを落とす流れを進める
	void Game::UpdateBossFall(float deltaTime)
	{
		const BossFallFlow::Result result = bossFall_.Update(deltaTime, floors_, *player_, *boss_, *camera_, direction_);
		DamageBoss(result.bossDamage);

		//揺れ終わったとき切り離された床にいたらプレイヤーも落ちる
		if (result.isPlayerCaught)
		{
			FailFall();
			return;
		}
		//[アイテム] アイテムは一旦使わないのでコメントアウト
		// //ボスが落ち始めたら置いてあるアイテムを消して持っているものは持ち越す
		// if (result.isDropStarted)
		// {
			// items_.ClearFieldItem();
		// }
		phase_ = bossFall_.GetPhase();
		if (result.isArrived)
		{
			OnArrivedBelow();
		}
		if (result.isFinished)
		{
			FinishBossFall();
		}
	}

	//ボスの島が切り離されたら揺れ始める
	void Game::BeginEscape()
	{
		boss_->Stop();
		bossFall_.BeginEscape();
		phase_ = bossFall_.GetPhase();
	}

	//ボスが下の層へ落ち始める
	void Game::BeginBossDrop()
	{
		//ボスを止める
		boss_->Stop();
		//[アイテム] 置いてあるアイテムを消して持っているものは持ち越すのでコメントアウト
		// items_.ClearFieldItem();
		bossFall_.BeginDrop(floors_, *player_, *boss_, *camera_);
		phase_ = bossFall_.GetPhase();
	}

	//ボスが下の層に着いたらハートを少し回復して、ボスを置き直してプレイヤーが尻もちをつく所を決める
	void Game::OnArrivedBelow()
	{
		//[アイテム] 新しい層でアイテムがまた出るようにするのでコメントアウト
		// items_.ResetForNewLayer();
		health_.Heal(PlayerConfig::kDescentHeal);
		health_.ClearInvincible();
		const Floor& floor = floors_.GetCurrent();
		const Vector3 bossPosition = SpawnPlanner::FindBossPosition(floor, boss_->GetPosition());
		battle_.Reset();
		boss_->SetActive(true);
		boss_->Place(floor, bossPosition);
		bossFall_.SetPlayerLanding(SpawnPlanner::FindPlayerPosition(floor, bossPosition));
	}

	//足場を飛ばし終わったらクリアか次の戦闘へ
	void Game::FinishBossFall()
	{
		if (boss_->IsDefeated())
		{
			boss_->SetActive(false);
			isCleared_ = true;
			clearPose_.Start(*camera_, *player_);
			return;
		}
		StartBattle();
	}
}
