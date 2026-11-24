#include "PlayerHealth.h"
#include "../../Config/GameConfig.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace App
{
	//HPを最大にして無敵を消す
	void PlayerHealth::Reset()
	{
		hearts_ = GameConfig::kMaxHearts;
		invincible_ = 0.0f;
	}

	//無敵の時間を進める
	void PlayerHealth::Update(float deltaTime)
	{
		invincible_ = std::max(0.0f, invincible_ - deltaTime);
	}

	//ダメージ
	bool PlayerHealth::Damage(int32_t amount)
	{
		if (amount <= 0 || IsInvincible())
		{
			return false;
		}
		//ハートを減らして、0でなければ少しの間無敵
		hearts_ = std::max(0, hearts_ - amount);
		if (!IsDead())
		{
			StartInvincible();
		}
		return true;
	}

	//最大まで回復する
	void PlayerHealth::Heal(int32_t amount)
	{
		hearts_ = std::min(GameConfig::kMaxHearts, hearts_ + amount);
	}

	void PlayerHealth::StartInvincible()
	{
		invincible_ = GameConfig::kInvincibilityTime;
	}

	void PlayerHealth::ClearInvincible()
	{
		invincible_ = 0.0f;
	}

	//点滅中に見えているか
	bool PlayerHealth::IsVisible() const
	{
		if (!IsInvincible())
		{
			return true;
		}
		//sinのプラスとマイナスで見えると見えないを切り替える
		const float wave = std::sin(invincible_ * GameConfig::kHitBlinkFrequency * std::numbers::pi_v<float> * 2.0f);
		return wave >= 0.0f;
	}
}
