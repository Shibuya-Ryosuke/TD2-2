#include "GameDirection.h"
#include "../Object/Player/PlayerHealth.h"
#include "../Object/Player/Player.h"
#include "../Object/Boss/Boss.h"
#include "../Stage/Floor/Floor.h"
#include "../Config/DirectionConfig.h"
#include <algorithm>

namespace App
{
	//ポストエフェクトと残像を用意する
	void GameDirection::Initialize()
	{
		fallLines_.Initialize();
		ghost_.Initialize();
		damage_.Initialize();
		lastHearts_ = -1;
		wasBossJumping_ = false;
	}

	//HPと場面と着地を見て演出を進める
	void GameDirection::Update(const PlayerHealth& health, GamePhase phase, const Player& player, const Boss& boss, Floor& floor, const BossTuning& tuning)
	{
		//HPが減ったら画面のふちを光らせる
		const int32_t hearts = health.GetHearts();
		if (lastHearts_ >= 0 && hearts < lastHearts_)
		{
			damage_.Play();
		}
		lastHearts_ = hearts;
		damage_.Update();

		//層を降りている間だけ落ちる線を出す
		fallLines_.Update(phase == GamePhase::Descent);

		//プレイヤーが着地した足元のマスをぷるるんと揺らす
		const float playerLanding = player.GetLandingPower();
		if (playerLanding > 0.0f)
		{
			floor.Jiggle(floor.GetCellAt(player.GetPosition()), std::min(playerLanding, DirectionConfig::kJigglePlayerMax));
		}

		//ボスが跳び終わったら足元のマスを大きめに揺らす
		const bool isBossJumping = boss.IsJumping();
		if (wasBossJumping_ && !isBossJumping && boss.IsActive())
		{
			floor.Jiggle(floor.GetCellAt(boss.GetPosition()), DirectionConfig::kJiggleBossPower);
		}
		wasBossJumping_ = isBossJumping;

		//戦闘中と戦闘開始待ちのときだけボスの残像を出す
		const bool isGhostVisible = phase == GamePhase::Waiting || phase == GamePhase::Battle;
		ghost_.Update(boss, floor, player.GetPosition(), tuning, isGhostVisible);
	}

	//ボスの残像を描く
	void GameDirection::Draw()
	{
		ghost_.Draw();
	}
}
