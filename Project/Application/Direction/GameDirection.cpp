#include "GameDirection.h"
#include "../Object/Player/PlayerHealth.h"
#include "../Object/Player/Player.h"
#include "../Object/Boss/Boss.h"
#include "../Stage/Floor/Floor.h"
#include "../Config/DirectionConfig.h"
#include <algorithm>

namespace App
{
	//ポストエフェクトを用意する
	void GameDirection::Initialize()
	{
		fallLines_.Initialize();
		damage_.Initialize();
		lastHearts_ = -1;
		wasBossJumping_ = false;
	}

	//HPと落ちているかと着地を見て演出を進める
	void GameDirection::Update(const PlayerHealth& health, bool isFalling, const Player& player, const Boss& boss, Floor& floor)
	{
		//HPが減ったら画面のふちを赤く光らせて揺らす
		const int32_t hearts = health.GetHearts();
		if (lastHearts_ >= 0 && hearts < lastHearts_)
		{
			damage_.Play();
			shake_.Play(DirectionConfig::kDamageShakePower);
		}
		lastHearts_ = hearts;
		damage_.Update();
		shakeOffset_ = shake_.Update(RyoEngine::GetDeltaTime());

		//層を降りている間だけ落ちる線を出す
		fallLines_.Update(isFalling);

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
	}
}
